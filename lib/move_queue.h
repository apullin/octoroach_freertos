// Contents of this file are copyright Andrew Pullin, 2013

#ifndef __MOVE_QUEUE_H
#define __MOVE_QUEUE_H

//FreeRTOS include
#include "queue.h"

enum moveSegT{
	MOVE_SEG_CONSTANT,
	MOVE_SEG_RAMP,
	MOVE_SEG_SIN,
	MOVE_SEG_TRI,
	MOVE_SEG_SAW,
	MOVE_SEG_IDLE,
        MOVE_SEG_LOOP_DECL,
        MOVE_SEG_LOOP_CLEAR,
        MOVE_SEG_QFLUSH
};

typedef struct
{
	int inputL, inputR;
	unsigned long duration;
	enum moveSegT type;
	int params[3];
        unsigned int steeringType;
        int steeringRate;

} moveCmdStruct;

typedef moveCmdStruct* moveCmdT;

xQueueHandle mqInit();
void mqPush(moveCmdT);
void mqPop(moveCmdT);
int mqIsFull(xQueueHandle);
int mqIsEmpty(xQueueHandle);
int mqGetSize(xQueueHandle);
void mqSetLooping(int);

#endif // __MOVE_QUEUE_H
