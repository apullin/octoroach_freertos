/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

int main(void) {
    /* Perform any hardware setup necessary. */
    //prvSetupHardware();
    /* --- APPLICATION TASKS CAN BE CREATED HERE --- */
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
}