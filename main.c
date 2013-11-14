/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//Library includes
#include "init_default.h"
#include "utils.h"

// Task prototypes
static void vToggleLED1(void *pvParameters);
static void vToggleLED2(void *pvParameters);

//Private function prototypes
static void prvSetupHardware(void);

int main(void) {
    /* Perform any hardware setup necessary. */
    prvSetupHardware();

    // Create tasks
    xTaskCreate(vToggleLED1, /* Pointer to the function that implements the task. */
            "Task 1", /* Text name for the task. This is to facilitate debugging. */
            240, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            1, /* This task will run at priority 1. */
            NULL); /* We are not going to use the task handle. */
    
    xTaskCreate(vToggleLED2, "Task 2", 240, NULL, 1, NULL);

    /* Start the created tasks running. */
    vTaskStartScheduler();
    /* Execution will only reach here if there was insufficient heap to
    start the scheduler. */
    for (;;);
    return 0;
}


unsigned long ulIdleCycleCount = 0UL;
/* Idle hook functions MUST be called vApplicationIdleHook(), take no parameters,
and return void. */
void vApplicationIdleHook(void) {
    /* This hook function does nothing but increment a counter. */
    ulIdleCycleCount++;
    Idle();  //dsPIC idle function; CPU core off, wakes on any interrupt
}

void prvSetupHardware(void){
    SetupClock();   //from imageproc-lib , config PLL
    SetupPorts();   //from imageproc-lib, set up LATn, TRISn, etc , requires __IMAGEPROC2
    SwitchClocks(); //from imageproc-lib , switch to PLL clock
    LED_1 = 1;
    LED_2 = 1;
    LED_3 = 1;
    Nop();
    Nop();
}

void vToggleLED1(void *pvParameters) {
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        LED_1 = ~LED_1; //Toggle LED #1 (color?)
        vTaskDelayUntil(&xLastWakeTime, (1000 / portTICK_RATE_MS));
    }
}

void vToggleLED2(void *pvParameters) {
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        LED_3 = ~LED_3; //Toggle LED #1 (color?)
        vTaskDelayUntil(&xLastWakeTime, (2000 / portTICK_RATE_MS));
    }
}