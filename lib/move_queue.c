// Contents of this file are copyright Andrew Pullin, 2013

/* Queue (FIFO) for moveCmdT

 */

//FreeRTOS includes
#include <FreeRTOS.h>
#include "queue.h"

#include "move_queue.h"

#define move_queue_MAX_QUEUE_LENGTH    16

static int moveQueueLooping = 0;

/*-----------------------------------------------------------------------------
 *          Public functions
-----------------------------------------------------------------------------*/

xQueueHandle xMoveQueue;

xQueueHandle mqInit() {
    //Initialize xMoveQueue
    xMoveQueue = xQueueCreate( move_queue_MAX_QUEUE_LENGTH, sizeof( moveCmdStruct ) );

    if( xMoveQueue != NULL )
    {
        return xMoveQueue;
    }
    else
    {
        //The queue could not be created
        return NULL;
    }
}

void mqPush(moveCmdT newmove) {
    portBASE_TYPE xStatus;
    const portTickType xTicksToWait = 10 / portTICK_RATE_MS;

    xStatus = xQueueSendToBack( xMoveQueue, newmove, xTicksToWait );

    if( xStatus != pdPASS )
    {
        //Add to queue did not succeed in 100ms. Throw away commanded movement.
    }
}

void mqPop(moveCmdT dest) {
    portBASE_TYPE xStatus;
    const portTickType xTicksToWait = 10 / portTICK_RATE_MS;

    xStatus = xQueueReceive( xMoveQueue, dest, xTicksToWait );

    if( xStatus == pdPASS )
        //Grabbed queue value succesfully
        {
        if(moveQueueLooping){
            mqPush(dest); //immediately re-push the just-popped item
        }
    }
    else{
        //Move queue pop failed

    }
}

int mqIsFull(xQueueHandle xQueue) {
    return uxQueueMessagesWaiting(xQueue) != 0;
}

int mqIsEmpty(xQueueHandle xQueue) {
    return uxQueueMessagesWaiting(xMoveQueue) == 0;
}


int mqGetSize(xQueueHandle xQueue) {
    return uxQueueMessagesWaiting(xQueue);
}

void mqSetLooping(int setval){
    moveQueueLooping = setval;
}

