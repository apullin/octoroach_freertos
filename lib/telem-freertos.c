// Contents of this file are copyright Andrew Pullin, 2013

#include<xc.h>
//FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//Library includes
#include "utils.h"
#include "settings.h"
#include "dfmem.h"
#include "telem-freertos.h"
#include "radio.h"
#include "at86rf231_driver.h"
#include "sclock.h"
#include "cmd_freertos.h"

#include <string.h> //for memcpy
//#include "debugpins.h"


//////////////////////////////////////////////////
//////////      FreeRTOS config        ///////////
//////////////////////////////////////////////////
#define TELEM_TASK_PERIOD_MS      1
#define TELEM_QUEUE_SIZE          8

//////////////////////////////////////////////////
//////////      Private variables      ///////////
//////////////////////////////////////////////////
static xTaskHandle xTelemTaskHandle = NULL;
static xTaskHandle xTelemWriteTaskHandle = NULL;
static unsigned int telemDataSize;
static unsigned int telemPacketSize;
static unsigned long samplesToSave = 0;

#define DEFAULT_SKIP_NUM       1           //Default to 1000 Hz save rate
//Skip counter for dividing the 300hz timer into lower telemetry rates
static unsigned int telemSkipNum = DEFAULT_SKIP_NUM;
static unsigned int skipcounter = DEFAULT_SKIP_NUM;
static unsigned long sampIdx = 0;

//static unsigned long samplesToStream = 0;
//static char telemStreamingFlag = TELEM_STREAM_OFF;
//static unsigned int streamSkipCounter = 0;
//static unsigned int streamSkipNum = 15;

//Offset for time value when recording samples
static unsigned long telemStartTime = 0;

static DfmemGeometryStruct mem_geo;

#define READBACK_DELAY_TIME_MS 4


//////////////////////////////////////////////////
////////// Private function prototypes ///////////
//////////////////////////////////////////////////
static portTASK_FUNCTION_PROTO(vTelemTask, pvParameters);       //FreeRTOS task, record telemetry into queue
static portTASK_FUNCTION_PROTO(vTelemWriteTask, pvParameters); //FreeRTOS task, writes queue into dfmem queue at low priority

portBASE_TYPE vStartTelemTasks( unsigned portBASE_TYPE uxPriority);

static QueueHandle_t telemQueue;

//////////////////////////////////////////////////
////////// Public function definitions ///////////
//////////////////////////////////////////////////

portBASE_TYPE vStartTelemTasks( unsigned portBASE_TYPE uxPriority){
    portBASE_TYPE xStatus;

    telemQueue = xQueueCreate(TELEM_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof (telemStruct_t));

    xStatus = xTaskCreate(vTelemTask, /* Pointer to the function that implements the task. */
            (const char *) "Telemetry Recording Task", /* Text name for the task. This is to facilitate debugging. */
            240, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            uxPriority, /* This task will run at priority 1. */
            &xTelemTaskHandle); /* We are not going to use the task handle. */

    xStatus = xTaskCreate(vTelemWriteTask, /* Pointer to the function that implements the task. */
            (const char *) "Telemetry Write Task", /* Text name for the task. This is to facilitate debugging. */
            240, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            uxPriority - 1, /* This task will run at priority 1. */
            &xTelemWriteTaskHandle); /* We are not going to use the task handle. */

    return xStatus;
}


void telemSetup(unsigned portBASE_TYPE uxPriority) {

    dfmemGetGeometryParams(&mem_geo); // Read memory chip sizing

    //Telemetry packet size is set at startupt time.
    telemDataSize = sizeof (TELEM_TYPE); //OctoRoACH specific
    telemPacketSize = sizeof (telemStruct_t);
    
    //Start FreeRTOS task
    vStartTelemTasks(uxPriority);
}

void telemSetSamplesToSave(unsigned long n) {
    samplesToSave = n;
    sampIdx = 0;
}

void telemReadbackSamples(unsigned long numSamples) {
    int delaytime_ms = READBACK_DELAY_TIME_MS;
    unsigned long i = 0; //will actually be the same as the sampleIndex

    telemStruct_t sampleData;

    for (i = 0; i < numSamples; i++) {
        //Retireve data from flash
        telemGetSample(i, sizeof (sampleData), (unsigned char*) (&sampleData));
        //Reliable send, with linear backoff

        radioSendData(RADIO_DST_ADDR, 0, CMD_SPECIAL_TELEMETRY, telemPacketSize,
                (unsigned char *)(&sampleData), 0);
//        do {
//            //debugpins1_set();
//            telemSendDataDelay(&sampleData, delaytime_ms);
//            //Linear backoff
//            delaytime_ms += 0;
//            //debugpins1_clr();
//        } while (trxGetLastACKd() == 0);
        
        delaytime_ms = READBACK_DELAY_TIME_MS;
    }
}

void telemSendDataDelay(telemStruct_t* sample, int delaytime_ms) {
    //radioSendData(RADIO_DST_ADDR, 0, CMD_SPECIAL_TELEMETRY, telemPacketSize, (unsigned char *)sample, 0);
    radioSendData(RADIO_DST_ADDR, 0, CMD_SPECIAL_TELEMETRY, telemPacketSize, (unsigned char *)sample, 0);
    delay_ms(delaytime_ms); // allow radio transmission time
}


//Saves telemetry data structure into flash memory, in order
//Position in flash memory is maintained by dfmem module
void telemEnqueueData(telemStruct_t * telemPkt) {
    
    portBASE_TYPE xStatus;

    //Enqueue data; TOD: Should this fail immediately, or after 1 period?
    xStatus = xQueueSendToBack(telemQueue, telemPkt, portMAX_DELAY);
    //xStatus = xQueueSendToBack(telemQueue, telemPkt, 0);
}

void telemErase(unsigned long numSamples) {
    //dfmemEraseSectorsForSamples(numSamples, sizeof (telemU));
    // TODO (apullin) : Add an explicit check to see if the number of saved
    //                  samples will fit into memory.

    //Green LED will be used as progress indicator


    LED_GREEN = 1;
    unsigned int firstPageOfSector, i;

    //avoid trivial case
    if (numSamples == 0) {
        return;
    }

    //Saves to dfmem will NOT overlap page boundaries, so we need to do this level by level:
    unsigned int samplesPerPage = mem_geo.bytes_per_page / telemPacketSize; //round DOWN int division
    unsigned int numPages = (numSamples + samplesPerPage - 1) / samplesPerPage; //round UP int division
    unsigned int numSectors = (numPages + mem_geo.pages_per_sector - 1) / mem_geo.pages_per_sector;

    //At this point, it is impossible for numSectors == 0
    //Sector 0a and 0b will be erased together always, for simplicity
    //Note that numSectors will be the actual number of sectors to erase,
    //   even though the sectors themselves are numbered starting at '0'
    dfmemEraseSector(0); //Erase Sector 0a
    LED_GREEN = ~LED_GREEN;
    dfmemEraseSector(8); //Erase Sector 0b
    LED_GREEN = ~LED_GREEN;

    //Start erasing the rest from Sector 1,
    // The (numsectors-1) here is because sectors are numbered from 0, whereas
    // numSectors is the actual count of sectors to erase; fencepost error.
    for (i = 1; i <= (numSectors-1); i++) {
        firstPageOfSector = mem_geo.pages_per_sector * i;
        //hold off until dfmem is ready for sector erase command
        //LED should blink indicating progress
        //Send actual erase command
        dfmemEraseSector(firstPageOfSector);
        LED_GREEN = ~LED_GREEN;
    }

    //Leadout flash, should blink faster than above, indicating the last sector
    while (!dfmemIsReady()) {
        LED_GREEN = ~LED_GREEN;
        delay_ms(50);
    }
    LED_GREEN = 0; //Green LED off

    //Since we've erased, reset our place keeper vars
    dfmemZeroIndex();

}


void telemGetSample(unsigned long sampNum, unsigned int sampLen, unsigned char *data)
{
    unsigned int samplesPerPage = mem_geo.bytes_per_page / sampLen; //round DOWN int division
    unsigned int pagenum = sampNum / samplesPerPage;
    unsigned int byteOffset = (sampNum - pagenum*samplesPerPage)*sampLen;

    dfmemRead(pagenum, byteOffset, sampLen, data);
}


void telemSetSkip(unsigned int skipnum) {
    telemSkipNum = skipnum;
}

//This function is a setter for the telemStartTime variable,
//which is used to offset the recorded times for telemetry, such that
//they start at approx. 0, instead of reflecting the total number of
//sclock ticks.

void telemSetStartTime(void) {
    telemStartTime = sclockGetTime();
}


//////////////////////////////////////////////////
////////// Private function definitions //////////
//////////////////////////////////////////////////

//This task will only recieve telemetry packets from a queue and write them to the dfmem.
//This decouples the telemetry recording from the writing to dfmem.
//void vTelemWriteTask(void *pvParameters) { //FreeRTOS task
static portTASK_FUNCTION(vTelemWriteTask, pvParameters) { //FreeRTOS task

    telemStruct_t data;
    portBASE_TYPE xStatus;

    for (;;) { //Task loop
        //Blocking read from
        xStatus = xQueueReceive(telemQueue, (unsigned char*)(&data), portMAX_DELAY);
        //Write to dfmem
        dfmemSave((unsigned char*)(&data), sizeof(data));
    }
}


//void vTelemTask(void *pvParameters) { //FreeRTOS task
static portTASK_FUNCTION(vTelemTask, pvParameters) { //FreeRTOS task

    telemStruct_t sample;
    //portBASE_TYPE xStatus;

    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;) { //Task loop

        //skipcounter decrements to 0, triggering a telemetry save, and resets
        // value of skicounter
        if (skipcounter == 0) {
            if (samplesToSave > 0) {
                sample.timestamp = sclockGetTime() - telemStartTime;
                sample.sampleIndex = sampIdx;
                //Write telemetry data into packet
                TELEMPACKFUNC(&(sample.telemData));

                telemEnqueueData(&sample);
                samplesToSave--;
                sampIdx++;
            }
            //Reset value of skip counter
            skipcounter = telemSkipNum;
        }
        //Always decrement skip counter at every interrupt, at 300Hz
        //This way, if telemSkipNum = 1, a sample is saved at every interrupt.
        skipcounter--;

        // Delay task in a periodic manner
        vTaskDelayUntil(&xLastWakeTime, (TELEM_TASK_PERIOD_MS / portTICK_RATE_MS));

    } // END of task loop
}