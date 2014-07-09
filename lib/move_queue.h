// Contents of this file are copyright Andrew Pullin, 2013

#ifndef __MOVE_QUEUE_H
#define __MOVE_QUEUE_H

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

typedef moveCmdStruct* moveSegment_t;

QueueHandle_t moveQueueInit(unsigned int max_size);

portBASE_TYPE moveQueuePush(QueueHandle_t mq, moveSegment_t mv);

portBASE_TYPE moveQueuePop(QueueHandle_t queue, moveSegment_t buffer);

int moveQueueIsFull(QueueHandle_t queue);

int moveQueueIsEmpty(QueueHandle_t queue);

int moveQueueGetSize(QueueHandle_t queue);

void moveQueueLoopingOnOff(int onoff);

#endif // __MOVE_QUEUE_H
