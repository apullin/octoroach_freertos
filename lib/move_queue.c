// Contents of this file are copyright Andrew Pullin, 2013

/* Queue (FIFO) for moveCmdT

 */

#include<xc.h>
//FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "move_queue.h"
#include "pid.h"
#include "p33Fxxxx.h"
//#include <stdio.h>      // for NULL
#include <stdlib.h>     // for malloc

static int moveQueueLooping = 0;

static QueueHandle_t xMoveQueue;

/*-----------------------------------------------------------------------------
 *          Public functions
-----------------------------------------------------------------------------*/

QueueHandle_t moveQueueInit(unsigned int queueLength) {
    xMoveQueue = xQueueCreate(queueLength, sizeof(moveCmdStruct));
    return xMoveQueue;
}

portBASE_TYPE moveQueuePush(QueueHandle_t queue, moveSegment_t move) {
    portBASE_TYPE xStatus;
    xStatus = xQueueSendToBack(queue, move, 0); //Add to end of the queue with no wait
    return xStatus;
}

portBASE_TYPE moveQueuePop(QueueHandle_t queue, moveSegment_t dest) {

    portBASE_TYPE xStatus;
    xStatus = xQueueReceive(queue, dest, 0); //Pop with no wait

    if(moveQueueLooping){
        moveQueuePush(queue, dest); //Re-add move that was just popped
    }

    return xStatus;
}

int moveQueueIsFull(QueueHandle_t queue) {

    if( uxQueueSpacesAvailable(queue) == 0 ){
        return pdTRUE;
    }
    return pdFALSE;

}

int moveQueueIsEmpty(QueueHandle_t queue) {
    if( uxQueueMessagesWaiting(queue) == 0 ){
        return pdTRUE;
    }
    return pdFALSE;
}


int moveQueueGetSize(QueueHandle_t queue) {
    return uxQueueMessagesWaiting(queue);
}

void moveQueueLoopingOnOff(int onoff){
    moveQueueLooping = onoff;
}

void moveQueueFlush(QueueHandle_t queue){
    int temp = moveQueueLooping;
    moveQueueLooping = 0;
    moveCmdStruct dump;
    while ( xQueueReceive(queue, &dump, 0) );
    moveQueueLooping = temp;
}