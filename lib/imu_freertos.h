/* 
 * File:   imu.h
 * Author: kohut
 *
 * Created on August 15, 2012, 3:37 PM
 */

#ifndef IMU_H
#define	IMU_H

portBASE_TYPE imuSetup(portBASE_TYPE uxPriority);

xTaskHandle imuGetTaskHandle(void);

///// Gyro /////
int imuGetGyroXValue();
int imuGetGyroYValue();
int imuGetGyroZValue();

float imuGetGyroXValueDeg();
float imuGetGyroYValueDeg();
float imuGetGyroZValueDeg();

///// XL /////
int imuGetXLXValue();
int imuGetXLYValue();
int imuGetXLZValue();

///// Moving average gyro /////
int imuGetGyroZValueAvg();
float imuGetGyroZValueAvgDeg();

///// Integrated gyro /////
float imuGetBodyZPositionDeg();

///// IMU Task control /////
void imuSuspend(void);
void imuResume(void);

//Constants
#define GYRO_DRIFT_THRESH 3
#define LSB2DEG    0.0695652174

#endif

