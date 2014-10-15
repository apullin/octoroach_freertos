/*
 * File:   imu_freertos.c
 * Author: Andrew Pullin
 *
 * Created on June 27, 2014
 */

#include<xc.h>
//FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//Library includes
#include "utils.h"
#include "led.h"
#include "gyro.h"
#include "xl.h"
#include "pid.h"
#include "dfilter_avg.h"
#include "mpu6000.h"

#include "imu.h"
#include <stdlib.h>

//////////////////////////////////////////////////
//////////      FreeRTOS config        ///////////
//////////////////////////////////////////////////
#define IMU_TASK_FREQUENCY_HZ   1000.0
#define IMU_TASK_PERIOD_MS      1000.0/IMU_TASK_FREQUENCY_HZ

#define ABS(my_val) ((my_val) < 0) ? -(my_val) : (my_val)


//////////////////////////////////////////////////
//////////      Private variables      ///////////
//////////////////////////////////////////////////
static xTaskHandle xImuTaskHandle = NULL;

static dfilterAvgInt_t gyroZavg;        //Filter stuctures for gyro variables
#define GYRO_AVG_SAMPLES 	4       //Averaging window for Gyro Z filter

//Variables to hold last latest values
static int lastGyro[3] = {0,0,0};
static int lastXL[3] = {0,0,0};
static float lastGyroDeg[3] = {0,0,0};
static int lastGyroZValueAvg = 0;
static float lastGyroZValueAvgDeg = 0.0;
static float lastBodyZPositionDeg = 0.0;


//////////////////////////////////////////////////
////////// Private function prototypes ///////////
//////////////////////////////////////////////////
portBASE_TYPE vStartIMUTask(unsigned portBASE_TYPE uxPriority);
static portTASK_FUNCTION_PROTO(vIMUTask, pvParameters); //FreeRTOS task


//////////////////////////////////////////////////
////////// Public function definitions ///////////
//////////////////////////////////////////////////

void imuSetup(unsigned portBASE_TYPE uxPriority) {
    portBASE_TYPE xStatus;

    //Chip config and SPI bus setup
    mpuSetup();

    //Start FreeRTOS task
    xStatus = vStartIMUTask(uxPriority);

    return xStatus; 
}

xTaskHandle imuGetTaskHandle(void){
    return xImuTaskHandle;
}

int imuGetGyroXValue() {
    return lastGyro[0];
}

int imuGetGyroYValue() {
    return lastGyro[1];
}

int imuGetGyroZValue() {
    return lastGyro[2];
}


float imuGetGyroXValueDeg() {
    return lastGyroDeg[0];
}

float imuGetGyroYValueDeg() {
    return lastGyroDeg[1];
}

float imuGetGyroZValueDeg() {
    return lastGyroDeg[2];
}


int imuGetGyroZValueAvg() {
    return lastGyroZValueAvg;
}

float imuGetGyroZValueAvgDeg() {
    return lastGyroZValueAvgDeg;
}


float imuGetBodyZPositionDeg() {
    return lastBodyZPositionDeg;
}

void imuResetGyroZAvg(){
    dfilterZero(&gyroZavg);
}

int imuGetXLXValue() {
    return lastXL[0];
}

int imuGetXLYValue() {
    return lastXL[1];
}

int imuGetXLZValue() {
    return lastXL[2];
}

// FreeRTOS task control functions
void imuSuspendTask(void) {
    vTaskSuspend(xImuTaskHandle);
}

void imuResumeTask(void) {
    vTaskResume(xImuTaskHandle);
}



//////////////////////////////////////////////////
////////// Private function definitions //////////
//////////////////////////////////////////////////

portBASE_TYPE vStartIMUTask( unsigned portBASE_TYPE uxPriority){
    portBASE_TYPE xStatus;

    xStatus = xTaskCreate(vIMUTask, /* Pointer to the function that implements the task. */
            (const char *) "IMU Task", /* Text name for the task. This is to facilitate debugging. */
            240, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            uxPriority, /* This task will run at priority 1. */
            &xImuTaskHandle); /* We are not going to use the task handle. */

    return xStatus;
}

void vIMUTask( void *pvParameters ) {               //FreeRTOS task

    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    int gyroData[3];
    int xlData[3];

    dfilterAvgCreate(&gyroZavg, GYRO_AVG_SAMPLES);

    for (;;) { //Task loop

        #if defined (__IMAGEPROC24)
        /////// Get Gyro data and calc average via filter
        gyroReadXYZ(); //bad design of gyro module; todo: humhu
        gyroGetIntXYZ(gyroData);
        //XL
        xlGetXYZ((unsigned char*) xlData); //bad design of gyro module; todo: humhu
        #elif defined (__IMAGEPROC25)
        mpuBeginUpdate();
        mpuGetGyro(gyroData);
        mpuGetXl(xlData);
        #endif

        //Copy values to local static variables
        lastGyro[0] = gyroData[0];
        lastGyro[1] = gyroData[1];
        lastGyro[2] = gyroData[2];

        lastXL[0] = xlData[0];
        lastXL[1] = xlData[1];
        lastXL[2] = xlData[2];

        //Gyro threshold:
        int i;
        for (i = 0; i < 3; i++) {
            //Threshold:
            if (ABS(lastGyro[i]) < GYRO_DRIFT_THRESH) {
                lastGyro[0] = lastGyro[i] >> 1; //fast divide by 2
            }
        }

        lastGyroDeg[0] = (float) (lastGyro[0] * LSB2DEG);
        lastGyroDeg[1] = (float) (lastGyro[1] * LSB2DEG);
        lastGyroDeg[2] = (float) (lastGyro[2] * LSB2DEG);


        //Update the filter with a new value
        dfilterAvgUpdate(&gyroZavg, gyroData[2]);
        //Calcualte new average from filter
        lastGyroZValueAvg = dfilterAvgCalc(&gyroZavg);

        //lastGyroZValueAvgDeg = (float)lastGyroZValueAvg*LSB2DEG;

        //lastBodyZPositionDeg = lastBodyZPositionDeg + lastGyroZValueDeg*TIMER_PERIOD;

        // Delay task in a periodic manner
        vTaskDelayUntil(&xLastWakeTime, (IMU_TASK_PERIOD_MS / portTICK_RATE_MS));

    }
}
