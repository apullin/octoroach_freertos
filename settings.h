// Contents of this file are copyright Andrew Pullin, 2013

/******************************************************************************
* Name: settings.h
* Desc: Constants used by Andrew P. are included here.
* Author: pullin
******************************************************************************/
#ifndef __SETTINGS_H
#define __SETTINGS_H


//#warning "REQUIRED: Review and set radio channel & network parameters in firmware/source/settings.h  , then comment out this line."
/////// Radio settings ///////
#define RADIO_CHANNEL		0x19
//#warning "You have changed the radio channel from 0x0E to something else"
#define RADIO_SRC_ADDR 		0x2052
#define RADIO_PAN_ID  	0x2050
//Hard-coded destination address, must match basestation or XBee addr
#define RADIO_DST_ADDR		0x2051

#define RADIO_RX_QUEUE_MAX_SIZE 	8
#define RADIO_TX_QUEUE_MAX_SIZE         8

/////// Configuration options ///////
//Configure project-wide for Hall Sensor operation
//#define HALL_SENSORS

#define TELEM_TYPE orTelemStruct_t
#define TELEM_INCLUDE "or_telem.h"
#define TELEMPACKFUNC(x) orTelemGetData(x)

//Motor controller output routing
// The leg controllers can be directed to different motor outputs from here
#define OCTOROACH_LEG1_MOTOR_CHANNEL 1
#define OCTOROACH_LEG2_MOTOR_CHANNEL 2

#endif //__SETTINGS_H
