/*
 * Copyright (c) 2010, Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the University of California, Berkeley nor the names
 *   of its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * FreeRTOS based firmware for the Biomimetic Millisystems Lab 802.15.4 USB basestation.
 *
 * by Andrew Pullin, based on work by Aaron M. Hoover & Kevin Peterson
 *
 *
 * Revisions:
 *  A. Pullin       6-27-2014       Initial verison, porting main.c to FreeRTOS primitives
 *
 * Notes:
 *
 */

#include <xc.h>                 //Microchip chip specific
#include <Generic.h>            //Microchip generic typedefs
#include "generic_typedefs.h"   //iplib generic tyepdefs

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//Library includes
#include "init_default.h"
#include "utils.h"
#include "uart.h"               //Microchip
#include <stdio.h>
#include "settings.h"
#include "sclock.h"
#include "dfmem.h"
#include "settings.h"

//Module includes
#include "imu_freertos.h"
#include "telem-freertos.h"
#include "radio_freertos.h"
#include "cmd_freertos.h"

//Ensure that imageproc-lib is a FreeRTOS branch
#ifndef _IMAGEPROC_LIB_FREERTOS
#error "Switch to FreeRTOS branch of imageproc-lib"
#endif

/* Task priorities. */
#define mainIMU_TASK_PRIORITY                           ( tskIDLE_PRIORITY + 10 )
#define mainTELEM_TASK_SAVE_PRIORITY                    ( tskIDLE_PRIORITY + 9 )
#define mainTELEM_TASK_FLASH_PRIORITY                   ( tskIDLE_PRIORITY + 8 )
#define mainRADIOTEST_TASK_PRIORITY                     ( tskIDLE_PRIORITY + 7 )
#define mainRADIORX_TASK_PRIORITY                       ( tskIDLE_PRIORITY + 5 )
#define mainRADIOTX_TASK_PRIORITY                       ( tskIDLE_PRIORITY + 4 )
#define mainALIVETEST_TASK_PRIORITY                     ( tskIDLE_PRIORITY + 3 )
#define mainCMDHANDLER_TASK_PRIORITY                    ( tskIDLE_PRIORITY + 1 )
//Private function prototypes
static void prvSetupHardware(void);

// Task prototypes
//static void vToggleLED1(void *pvParameters);

//Private function prototypes
static void prvSetupHardware(void);
void prvStartupLights(void);

// Debugging and test tasks
void radioTestSetup( unsigned portBASE_TYPE uxPriority);
void aliveTestSetup( unsigned portBASE_TYPE uxPriority);

int main(void) {
    /* Perform any hardware setup necessary. */
    prvSetupHardware();  //Basic board init

    //Radio
    radioSetup(RADIO_RX_QUEUE_MAX_SIZE, RADIO_TX_QUEUE_MAX_SIZE, mainRADIORX_TASK_PRIORITY, mainRADIOTX_TASK_PRIORITY);
    radioSetChannel(RADIO_CHANNEL);
    radioSetSrcPanID(RADIO_PAN_ID);
    radioSetSrcAddr(RADIO_SRC_ADDR);

    //IMU task, runs at 1Khz
    //imuSetup(mainIMU_TASK_PRIORITY);

    //Telemetry recording task, runs at 1Khz
    //dfmemSetup();
    //telemSetup(mainTELEM_TASK_SAVE_PRIORITY, mainTELEM_TASK_FLASH_PRIORITY);

    //Lights test to show cpu is alive
    //aliveTestSetup(mainALIVETEST_TASK_PRIORITY);
    
    //Cmd Handler task
    cmdSetup(CMD_QUEUE_MAX_SIZE, mainCMDHANDLER_TASK_PRIORITY);
    
    //Startup indicator cycle with LEDs
    prvStartupLights();

    //Test that sends WHOAMI's and ECHO's at 2Hz
    radioTestSetup(mainRADIOTEST_TASK_PRIORITY);

    /* Start the created tasks running. */
    vTaskStartScheduler();

    /* Execution will only reach here if there was insufficient heap to
    start the scheduler. */
__TRACE(0x78);
    for (;;);
    return 0;
}


unsigned long ulIdleCycleCount = 0UL;
/* Idle hook functions MUST be called vApplicationIdleHook(), take no parameters,
and return void. */
void vApplicationIdleHook(void) {
__TRACE(0x79);
    /* This hook function does nothing but increment a counter. */
    ulIdleCycleCount++;
    Idle();  //dsPIC idle function; CPU core off, wakes on any interrupt
    //portSWITCH_CONTEXT();
//    taskYIELD();  //TODO: Unclear if this is needed?
}

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    signed char *pcTaskName ){
    while(1){
__TRACE(0x7a);
        Nop();
    }
}


void prvSetupHardware(void){
    SetupClock();   //from imageproc-lib , config PLL
    SetupPorts();   //from imageproc-lib, set up LATn, TRISn, etc , requires __IMAGEPROC2
    SwitchClocks(); //from imageproc-lib , switch to PLL clock
    sclockSetup();
}



void prvStartupLights(void) {
    int i;

    for (i = 0; i < 4; i++) {
        LED_RED = ~LED_RED;
        delay_ms(30);
        LED_GREEN = ~LED_GREEN;
        delay_ms(30);
        LED_YELLOW = ~LED_YELLOW;
        delay_ms(30);
    }
}

///////////// Just for testing below here! //////////////////

void radioTestSetup( unsigned portBASE_TYPE uxPriority);
static portTASK_FUNCTION_PROTO(vRadioTestTask, pvParameters); //FreeRTOS task
#include "cmd_freertos.h"
#include "version.h"
#include <string.h>

void radioTestSetup( unsigned portBASE_TYPE uxPriority){
    portBASE_TYPE xStatus;

    xStatus = xTaskCreate(vRadioTestTask, /* Pointer to the function that implements the task. */
            (const char *) "Radio Test Task", /* Text name for the task. This is to facilitate debugging. */
            512, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            uxPriority, /* This task will run at priority 1. */
            NULL); /* We are not going to use the task handle. */

//    return xStatus;
}

static portTASK_FUNCTION(vRadioTestTask, pvParameters){
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    portBASE_TYPE xStatus;

    static char echoMsg[90];
    int heapspace = 0;

    unsigned long pktNum = 1;
    
    static int sendPeriod = 100; //made a var so we can change this on the fly

    for (;;) {
__TRACE(0x7b);
        heapspace = xPortGetFreeHeapSize();
        //heapspace = 666;
        sprintf(echoMsg, "FreeRTOS test packet #%lu, heap space: %d", pktNum, heapspace);
        pktNum++;
        
        radioSendData(RADIO_DST_ADDR, 0, CMD_ECHO, strlen(echoMsg), (unsigned char*)echoMsg, 0);
        LED_YELLOW = ~LED_YELLOW;
        //Delay 500ms
        vTaskDelayUntil(&xLastWakeTime, (sendPeriod / portTICK_RATE_MS));

        taskYIELD();
    }
}

//// Alive indicator, "heartbeat" double flash
void aliveTestSetup( unsigned portBASE_TYPE uxPriority);
static portTASK_FUNCTION_PROTO(vAliveTestTask, pvParameters); //FreeRTOS task

void aliveTestSetup( unsigned portBASE_TYPE uxPriority){
    portBASE_TYPE xStatus;

    xStatus = xTaskCreate(vAliveTestTask, /* Pointer to the function that implements the task. */
            (const char *) "Heartbeat task", /* Text name for the task. This is to facilitate debugging. */
            512, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            uxPriority, /* This task will run at priority 1. */
            NULL); /* We are not going to use the task handle. */
//    return xStatus;
}

static portTASK_FUNCTION(vAliveTestTask, pvParameters){
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    portBASE_TYPE xStatus;

    for (;;) {
        
        //LED heartbeat
        LED_RED = 1;
        vTaskDelayUntil(&xLastWakeTime, (100 / portTICK_RATE_MS));
        LED_RED = 0;
        vTaskDelayUntil(&xLastWakeTime, (70 / portTICK_RATE_MS));
        LED_RED = 1;
        vTaskDelayUntil(&xLastWakeTime, (100 / portTICK_RATE_MS));
        LED_RED = 0;
        vTaskDelayUntil(&xLastWakeTime, (800 / portTICK_RATE_MS)); //1 Hz
        taskYIELD();
    }
}