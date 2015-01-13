// Contents of this file are copyright Andrew Pullin, 2013

//or_telem.c , OctoRoACH specific telemetry packet format


#include <xc.h>
//FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"

//Module include
#include "or_telem.h"
#include "settings.h"

//Library includes
#include "utils.h"
#include "init_default.h"
#include "radio_freertos.h"
#include "mac_packet.h"
#include "sclock.h"
#include "timer.h"
#include "ppool.h"

#include <xc.h>
#include "pid.h"
#include "gyro.h"
#include "xl.h"
#include "ams-enc.h"
#include "imu_freertos.h"
//#include "leg_ctrl.h" //TODO: Temporarily removed while porting
#include "adc_pid.h"
#include "tih.h"
//#include "steering.h" //TODO: Temporarily removed while porting

// TODO (apullin) : Remove externs by adding getters to other modules
//extern pidObj motor_pidObjs[NUM_MOTOR_PIDS];
//extern int bemf[NUM_MOTOR_PIDS];
//extern pidObj steeringPID;
//extern pidObj tailPID;

void orTelemGetData(TELEM_TYPE* data) {
 
//    tptr->inputL = legCtrlGetInput(1); //TODO: Temporarily removed while porting
//    tptr->inputR = legCtrlGetInput(2); //TODO: Temporarily removed while porting
//    tptr->dcA = tiHGetSignedDC(1);
//    tptr->dcB = tiHGetSignedDC(2);
//    tptr->dcC = tiHGetSignedDC(3);
//    tptr->dcD = tiHGetSignedDC(4);
//    tptr->gyroX = imuGetGyroXValue();
//    tptr->gyroY = imuGetGyroYValue();
//    tptr->gyroZ = imuGetGyroZValue();
//    tptr->gyroAvg =imuGetGyroZValueAvgDeg();
//    tptr->accelX = imuGetXLXValue();
//    tptr->accelY = imuGetXLYValue();
//    tptr->accelZ = imuGetXLZValue();
//    tptr->bemfA = adcGetMotorA();
//    tptr->bemfB = adcGetMotorB();
//    tptr->bemfC = adcGetMotorC();
//    tptr->bemfD = adcGetMotorD();
//    tptr->steerIn = steeringGetInput(); //TODO: Temporarily removed while porting
//    tptr->steerOut = steeringGetInput(); //TODO: Temporarily removed while porting
//    tptr->Vbatt = adcGetVbatt();
//    tptr->yawAngle = imuGetBodyZPositionDeg();

    data->inputL = 1; //TODO: Temporarily removed while porting
    data->inputR = 2; //TODO: Temporarily removed while porting
    data->dcA = 3;
    data->dcB = 4;
    data->dcC = 5;
    data->dcD = 6;
    data->gyroX = 7;
    data->gyroY = 8;
    data->gyroZ = 9;
    data->gyroAvg = 10;
    data->accelX = 11;
    data->accelY = 12;
    data->accelZ = 13;
    data->bemfA = 14;
    data->bemfB = 15;
    data->bemfC = 16;
    data->bemfD = 17;
    data->steerIn = 18; //TODO: Temporarily removed while porting
    data->steerOut = 19; //TODO: Temporarily removed while porting
    data->Vbatt = 20;
    data->yawAngle = 21.0;
}

//This may be unneccesary, since the telemtry type isn't totally anonymous

unsigned int orTelemGetSize() {
    return sizeof (orTelemStruct_t);
}