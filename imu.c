// Authors: apullin, nkohut

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "utils.h"
#include "led.h"
#include "gyro.h"
#include "xl.h"
#include "pid.h"
#include "dfilter_avg.h"
#include "adc_pid.h"
#include "leg_ctrl.h"
//#include "ams-enc.h"
#include "imu.h"
#include <stdlib.h>

#define IMU_TASK_PRIORITY       1
#define IMU_TASK_FREQUENCY_HZ   1000.0
#define IMU_TASK_PERIOD_MS      1000.0/IMU_TASK_FREQUENCY_HZ

static xTaskHandle xImuTaskHandle = NULL;

//Setup for Gyro Z averaging filter
#define GYRO_AVG_SAMPLES 	4


//Filter stuctures for gyro variables
static dfilterAvgInt_t gyroZavg;


//TODO: change these to arrays
static int lastGyroXValue = 0;
static int lastGyroYValue = 0;
static int lastGyroZValue = 0;

static float lastGyroXValueDeg = 0.0;
static float lastGyroYValueDeg = 0.0;
static float lastGyroZValueDeg = 0.0;

static int lastGyroZValueAvg = 0;

static float lastGyroZValueAvgDeg = 0.0;

static float lastBodyZPositionDeg = 0.0;

//XL
static int lastXLXValue = 0;
static int lastXLYValue = 0;
static int lastXLZValue = 0;


// Private function prototypes
void xImuTask();


portBASE_TYPE vStarLegCtrlTask(void){
    portBASE_TYPE xStatus;
    const signed char* taskString = "IMU Task";
    
    xStatus = xTaskCreate(xImuTask, /* Pointer to the function that implements the task. */
            taskString, /* Text name for the task. This is to facilitate debugging. */
            240, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            IMU_TASK_PRIORITY, /* This task will run at priority 1. */
            &xImuTaskHandle); /* We are not going to use the task handle. */
    
    return xStatus;
}

xTaskHandle imuGetTaskHandle(void){
    return xImuTaskHandle;
}

////   Private functions
void xImuTask( void *pvParameters ) {

    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    int gyroData[3];
    int xlData[3];

    dfilterAvgCreate(&gyroZavg, GYRO_AVG_SAMPLES);

    for (;;) { //Task loop

        // Get Gyro data and calc average via filter
        gyroReadXYZ(); //bad design of gyro module; todo: humhu
        gyroGetIntXYZ(gyroData);

        //XL
        xlGetXYZ((unsigned char*) xlData); //bad design of gyro module; todo: humhu

        //Copy values to local static variables
        lastGyroXValue = gyroData[0];
        lastGyroYValue = gyroData[1];
        lastGyroZValue = gyroData[2];

        lastXLXValue = xlData[0];
        lastXLYValue = xlData[1];
        lastXLZValue = xlData[2];

        //Gyro threshold:
        if ((lastGyroXValue < GYRO_DRIFT_THRESH) && (lastGyroXValue > -GYRO_DRIFT_THRESH)) {
            lastGyroXValue = lastGyroXValue >> 1; //fast divide by 2
        }
        if ((lastGyroYValue < GYRO_DRIFT_THRESH) && (lastGyroYValue > -GYRO_DRIFT_THRESH)) {
            lastGyroYValue = lastGyroYValue >> 1; //fast divide by 2
        }
        if ((lastGyroZValue < GYRO_DRIFT_THRESH) && (lastGyroZValue > -GYRO_DRIFT_THRESH)) {
            lastGyroZValue = lastGyroZValue >> 1; //fast divide by 2
        }


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

////   Public functions
////////////////////////
int imuGetGyroXValue() {
    return lastGyroXValue;
}

int imuGetGyroYValue() {
    return lastGyroYValue;
}

int imuGetGyroZValue() {
    return lastGyroZValue;
}


float imuGetGyroXValueDeg() {
    return lastGyroXValueDeg;
}

float imuGetGyroYValueDeg() {
    return lastGyroYValueDeg;
}

float imuGetGyroZValueDeg() {
    return lastGyroZValueDeg;
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

int imuGetXLXValue(){
    return lastXLXValue;
}

int imuGetXLYValue(){
    return lastXLYValue;
}

int imuGetXLZValue(){
    return lastXLZValue;
}

void imuSuspendTask(void) {
    vTaskSuspend(xImuTaskHandle);
}

void imuResumeTask(void) {
    vTaskResume(xImuTaskHandle);
}