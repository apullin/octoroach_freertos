/******************************************************************************
* Name: cmd_freertos.h
* Desc: application specific command definitions are defined here.
* Date: 2010-07-10
* Author: stanbaek, apullin
Modifications and additions to this file made by Andrew Pullin are copyright, 2013
Copyrights are acknowledged for portions of this code extant before modifications by Andrew Pullin 
Any application of BSD or other license to copyright content without the authors express approval
is invalid and void.
******************************************************************************/
#ifndef __CMD_H
#define __CMD_H

#include "FreeRTOS.h"
#include "queue.h"

#include "cmd_const.h"

#define CMD_VECTOR_SIZE				0xFF //full length vector
#define MAX_CMD_FUNC				0x9F

#define CMD_SET_THRUST_OPENLOOP     0x80
#define CMD_SET_THRUST_CLOSEDLOOP   0x81
#define CMD_SET_PID_GAINS           0x82

#define CMD_SET_CTRLD_TURN_RATE     0x84
#define CMD_STREAM_TELEMETRY        0x85
#define CMD_SET_MOVE_QUEUE          0x86
#define CMD_SET_STEERING_GAINS      0x87
#define CMD_SOFTWARE_RESET          0x88
#define CMD_SPECIAL_TELEMETRY       0x89
#define CMD_ERASE_SECTORS           0x8A
#define CMD_FLASH_READBACK          0x8B
#define CMD_SLEEP                   0x8C
#define CMD_SET_VEL_PROFILE         0x8D
#define CMD_WHO_AM_I                0x8E
#define CMD_HALL_TELEMETRY          0x8F
#define CMD_ZERO_POS                0x90
#define CMD_SET_HALL_GAINS          0x91
#define CMD_SET_TAIL_QUEUE          0x92
#define CMD_SET_TAIL_GAINS          0x93
#define CMD_SET_OL_VIBE             0x95

typedef struct{
    unsigned char status;
    unsigned char cmdType;
    unsigned char length;
    unsigned char* frame;
} cmdStruct_t;

unsigned int cmdSetup(unsigned int cmd_queue_length, unsigned portBASE_TYPE uxPriority);

QueueHandle_t cmdGetQueueHandle(void);

#include "cmd_structs.h"



#endif // __CMD_H

