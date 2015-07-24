/***************************************************************************
 * Name: cmd_freertos.c
 * Desc: Receiving and transmitting queue handler
 * Date: 2010-07-10
 * Author: stanbaek, apullin

Copyrights are acknowledged for portions of this code extant before modifications by Andrew Pullin 
Any application of BSD or other license to copyright content without the authors express approval
is invalid and void.
 **************************************************************************/

#include<xc.h>
//FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//Library includes
#include "utils.h"
#include "cmd_freertos.h"
#include "cmd_const.h"
#include "dfmem.h"
#include "radio_freertos.h"
#include "tih.h"
#include "telem-freertos.h"
#include "version.h"
#include "settings.h" //major config defines, sys-service, hall, etc

#include <stdlib.h>
#include <string.h>

#define PKT_UNPACK(type, var, pktframe) type* var = (type*)(pktframe);

//////////////////////////////////////////////////
//////////      FreeRTOS config        ///////////
//////////////////////////////////////////////////
#define CMD_QUEUE_SIZE          1

//////////////////////////////////////////////////
////////// Private function prototypes ///////////
//////////////////////////////////////////////////
static portTASK_FUNCTION_PROTO(vCmdHandlerTask, pvParameters);       //FreeRTOS task
portBASE_TYPE vStartCmdHandlerTask( unsigned portBASE_TYPE uxPriority);
static QueueHandle_t cmdQueue;

// use an array of function pointer to avoid a number of case statements
// CMD_VECTOR_SIZE is defined in cmd_const.h
void (*cmd_func[CMD_VECTOR_SIZE])(unsigned char, unsigned char, unsigned char*);

/*-----------------------------------------------------------------------------
 *          Declaration of static functions
-----------------------------------------------------------------------------*/
static void cmdSetThrust(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSteer(unsigned char status, unsigned char length, unsigned char *frame);

static void cmdEraseMemSector(unsigned char status, unsigned char length, unsigned char *frame);

static void cmdEcho(unsigned char status, unsigned char length, unsigned char *frame);

static void cmdNop(unsigned char status, unsigned char length, unsigned char *frame);

//User commands
static void cmdSetThrustOpenLoop(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetThrustClosedLoop(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetPIDGains(unsigned char status, unsigned char length, unsigned char *frame);

static void cmdSetCtrldTurnRate(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdGetImuLoopZGyro(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetMoveQueue(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetSteeringGains(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSoftwareReset(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSpecialTelemetry(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdEraseSector(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdFlashReadback(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSleep(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetVelProfile(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdWhoAmI(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdHallTelemetry(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdZeroPos(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetHallGains(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetTailQueue(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetTailGains(unsigned char status, unsigned char length, unsigned char *frame);
static void cmdSetOLVibe(unsigned char status, unsigned char length, unsigned char *frame);

//////////////////////////////////////////////////
////////// Public function definitions ///////////
//////////////////////////////////////////////////

portBASE_TYPE vStartCmdHandlerTask( unsigned portBASE_TYPE uxPriority){
    portBASE_TYPE xStatus;

    xStatus = xTaskCreate(vCmdHandlerTask, /* Pointer to the function that implements the task. */
            (const char *) "Cmd Handler Task", /* Text name for the task. This is to facilitate debugging. */
            512, /* Stack depth in words. */
            NULL, /* We are not using the task parameter. */
            uxPriority, /* This task will run at priority 1. */
            NULL); /* We are not going to use the task handle. */

    return xStatus;
}


unsigned int cmdSetup(unsigned int cmd_queue_length, unsigned portBASE_TYPE uxPriority) {

    unsigned int i;

    // initialize the array of func pointers with Nop()
    for (i = 0; i < MAX_CMD_FUNC; ++i) {
        cmd_func[i] = &cmdNop;
        //cmd_len[i] = 0; //0 indicated an unpoplulated command
    }

    cmd_func[CMD_ECHO] = &cmdEcho;
    cmd_func[CMD_SET_THRUST] = &cmdSetThrust;
    cmd_func[CMD_SET_STEER] = &cmdSteer;
    cmd_func[CMD_ERASE_MEM_SECTOR] = &cmdEraseMemSector;
    //User commands
    cmd_func[CMD_SET_THRUST_OPENLOOP] = &cmdSetThrustOpenLoop;
    cmd_func[CMD_SET_THRUST_CLOSEDLOOP] = &cmdSetThrustClosedLoop;
    cmd_func[CMD_SET_PID_GAINS] = &cmdSetPIDGains;
    cmd_func[CMD_SET_CTRLD_TURN_RATE] = &cmdSetCtrldTurnRate;
    cmd_func[CMD_STREAM_TELEMETRY] = &cmdGetImuLoopZGyro;
    cmd_func[CMD_SET_MOVE_QUEUE] = &cmdSetMoveQueue;
    cmd_func[CMD_SET_STEERING_GAINS] = &cmdSetSteeringGains;
    cmd_func[CMD_SOFTWARE_RESET] = &cmdSoftwareReset;
    cmd_func[CMD_SPECIAL_TELEMETRY] = &cmdSpecialTelemetry;
    cmd_func[CMD_ERASE_SECTORS] = &cmdEraseSector;
    cmd_func[CMD_FLASH_READBACK] = &cmdFlashReadback;
    cmd_func[CMD_SLEEP] = &cmdSleep;
    cmd_func[CMD_SET_VEL_PROFILE] = &cmdSetVelProfile;
    cmd_func[CMD_WHO_AM_I] = &cmdWhoAmI;
    cmd_func[CMD_HALL_TELEMETRY] = &cmdHallTelemetry;
    cmd_func[CMD_ZERO_POS] = &cmdZeroPos;
    cmd_func[CMD_SET_HALL_GAINS] = &cmdSetHallGains;
    cmd_func[CMD_SET_TAIL_QUEUE] = &cmdSetTailQueue;
    cmd_func[CMD_SET_TAIL_GAINS] = &cmdSetTailGains;
    cmd_func[CMD_SET_OL_VIBE]  = &cmdSetOLVibe;

    cmdQueue = xQueueCreate(cmd_queue_length, (unsigned portBASE_TYPE) sizeof ( MacPacketStruct));
    
    //Start FreeRTOS task
    vStartCmdHandlerTask(uxPriority);

    return 1;
}


/*-----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 * The functions below are intended for internal use, i.e., private methods.
 * Users are recommended to use functions defined above.
 * ----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/

static void cmdSetThrust(unsigned char status, unsigned char length, unsigned char *frame) {
/*
    unsigned char chr_test[4];
    float *duty_cycle = (float*) chr_test;

    chr_test[0] = frame[0];
    chr_test[1] = frame[1];
    chr_test[2] = frame[2];
    chr_test[3] = frame[3];

    mcSetDutyCycle(MC_CHANNEL_PWM1, duty_cycle[0]);
    //mcSetDutyCycle(1, duty_cycle[0]);
 */
}

static void cmdSteer(unsigned char status, unsigned char length, unsigned char *frame) {
/*
    unsigned char chr_test[4];
    float *steer_value = (float*) chr_test;

    chr_test[0] = frame[0];
    chr_test[1] = frame[1];
    chr_test[2] = frame[2];
    chr_test[3] = frame[3];

    mcSteer(steer_value[0]);
    */
}

/*-----------------------------------------------------------------------------
 *          IMU functions
-----------------------------------------------------------------------------*/

static void cmdEraseMemSector(unsigned char status, unsigned char length, unsigned char *frame) {
    unsigned int page;
    page = frame[0] + (frame[1] << 8);
    LED_RED = 1;
    dfmemEraseSector(0x0100); // erase Sector 1 (page 256 - 511)
    LED_RED = 0;
}

/*-----------------------------------------------------------------------------
 *          AUX functions
-----------------------------------------------------------------------------*/
void cmdEcho(unsigned char status, unsigned char length, unsigned char *frame) {
    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
__TRACE(0x6a);
    
    radioSendData(RADIO_DST_ADDR, 0, CMD_ECHO, length, frame, 0);
}

static void cmdNop(unsigned char status, unsigned char length, unsigned char *frame) {
    Nop();
}

/*-----------------------------------------------------------------------------
 *         User function
-----------------------------------------------------------------------------*/
static void cmdSetThrustOpenLoop(unsigned char status, unsigned char length, unsigned char *frame) {
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetThrustOpenLoop, argsPtr, frame);

    //set motor duty cycle
    tiHSetDC(argsPtr->channel, argsPtr->dc);
}

static void cmdSetThrustClosedLoop(unsigned char status, unsigned char length, unsigned char *frame) {
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetThrustClosedLoop, argsPtr, frame);

//    legCtrlSetInput(LEG_CTRL_LEFT, argsPtr->chan1);
//    legCtrlOnOff(LEG_CTRL_LEFT, PID_ON); //Motor PID #1 -> ON
//
//    legCtrlSetInput(LEG_CTRL_RIGHT, argsPtr->chan2);
//    legCtrlOnOff(LEG_CTRL_RIGHT, PID_ON); //Motor PID #2 -> ON
}

static void cmdSetPIDGains(unsigned char status, unsigned char length, unsigned char *frame) {
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetPIDGains, argsPtr, frame);

//    legCtrlSetGains(0, argsPtr->Kp1, argsPtr->Ki1, argsPtr->Kd1, argsPtr->Kaw1, argsPtr->Kff1);
//    legCtrlSetGains(1, argsPtr->Kp2, argsPtr->Ki2, argsPtr->Kd2, argsPtr->Kaw2, argsPtr->Kff2);

    //Send confirmation packet, which is the exact same data payload as what was sent
    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_SET_PID_GAINS, length, frame, 0);

}

static void cmdSetCtrldTurnRate(unsigned char status, unsigned char length, unsigned char *frame) {
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetCtrldTurnRate, argsPtr, frame);
//    steeringSetInput(argsPtr->steerInput);

    //Send confirmation packet, which is the exact same data payload as what was sent
    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_SET_CTRLD_TURN_RATE, length, frame, 0);
}

static void cmdGetImuLoopZGyro(unsigned char status, unsigned char length, unsigned char *frame) {
    //Obsolete, do not use
}

static void cmdSetMoveQueue(unsigned char status, unsigned char length, unsigned char *frame) {
    //The count is read via standard pointer math
    unsigned int count;
    count = (unsigned int) (*(frame));

    //Unpack unsigned char* frame into structured values
    //Here, unpacking happens in the loop below.
    //Due to variable length, PKT_UNPACK is not used here
    int idx = sizeof (count); //should be an unsigned int, sizeof(uint) = 2

//    moveCmdT move;
//    int i;
//    for (i = 0; i < count; i++) {
//        move = (moveCmdT) malloc(sizeof (moveCmdStruct));
//        //argsPtr = (_args_cmdSetMoveQueue*)(frame+idx);
//        *move = *((moveCmdT) (frame + idx));
//        mqPush(moveq, move);
//        //idx =+ sizeof(_args_cmdSetMoveQueue);
//        idx += sizeof (moveCmdStruct);
//    }
}

//Format for steering gains:
// [Kp, Ki, Kd, Kaw, Kff, steeringMode]
//

static void cmdSetSteeringGains(unsigned char status, unsigned char length, unsigned char *frame) {
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetSteeringGains, argsPtr, frame);

//    steeringSetGains(argsPtr->Kp, argsPtr->Ki, argsPtr->Kd, argsPtr->Kaw, argsPtr->Kff);
//    steeringSetMode(argsPtr->steerMode);

    //Send confirmation packet, which is the exact same data payload as what was sent
    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_SET_STEERING_GAINS, length, frame, 1);
}

static void cmdSoftwareReset(unsigned char status, unsigned char length, unsigned char *frame) {
    char resetmsg[] = "SOFTWARE RESET";
    int len = strlen(resetmsg);
    
    //Send notification message before reset
    radioSendData(RADIO_DST_ADDR, 0, CMD_SOFTWARE_RESET, len, (unsigned char*)resetmsg, 0);

#ifndef __DEBUG //Prevent reset in debug mode
    __asm__ volatile ("reset");
#endif
}

static void cmdSpecialTelemetry(unsigned char status, unsigned char length, unsigned char *frame) {
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSpecialTelemetry, argsPtr, frame);

    if (argsPtr->count != 0) {
        telemSetStartTime(); // Start telemetry samples from approx 0 time
        telemSetSamplesToSave(argsPtr->count);
    }
}

static void cmdEraseSector(unsigned char status, unsigned char length, unsigned char *frame) {
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdEraseSector, argsPtr, frame);

    telemErase(argsPtr->samples);

    //Send confirmation packet; this only happens when flash erase is completed.
    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_ERASE_SECTORS, length, frame, 0);
}

static void cmdFlashReadback(unsigned char status, unsigned char length, unsigned char *frame) {
    //LED_YELLOW = 1;
    PKT_UNPACK(_args_cmdFlashReadback, argsPtr, frame);

    telemReadbackSamples(argsPtr->samples);

}

static void cmdSleep(unsigned char status, unsigned char length, unsigned char *frame) {
    //Currently unused
    //todo : review power reduction organization
    /*char sleep = frame[0];
    if (sleep) {
        //g_radio_duty_cycle = 1;
    } else {
        //g_radio_duty_cycle = 0;
        Payload pld;
        pld = payCreateEmpty(1);
        paySetData(pld, 1, (unsigned char*) (&sleep)); //echo back a CMD_SLEEP with '0', incdicating a wakeup
        paySetStatus(pld, status);
        paySetType(pld, CMD_SLEEP);
        radioSendPayload(RADIO_DST_ADDR, pld);
    }*/
}

// set up velocity profile structure  - assume 4 set points for now, generalize later
static void cmdSetVelProfile(unsigned char status, unsigned char length, unsigned char *frame) {
/*
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetVelProfile, argsPtr, frame);

    hallSetVelProfile(0, argsPtr->intervalsL, argsPtr->deltaL, argsPtr->velL);
    hallSetVelProfile(1, argsPtr->intervalsR, argsPtr->deltaR, argsPtr->velR);

    //Send confirmation packet; this only happens when flash erase is completed.
    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_SET_VEL_PROFILE, length, frame, 0);
 */
}

// report motor position and  reset motor position (from Hall effect sensors)
// note motor_count is long (4 bytes)
void cmdZeroPos(unsigned char status, unsigned char length, unsigned char *frame) {
/*
    hallZeroPos(0);
    hallZeroPos(1);

    // This function in unsafe. Passing pointers between modules is disallowed.
    long* hallCounts = hallGetMotorCounts();

    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_ZERO_POS, 2*sizeof(unsigned long), (unsigned char*)hallCounts, 0);
 */
}

// alternative telemetry which runs at 1 kHz rate inside PID loop
static void cmdHallTelemetry(unsigned char status, unsigned char length, unsigned char *frame) {
    //TODO: Integration of hall telemetry is unfinished. Fuction will currently
    // do nothing.

    //This is only commented to supress the warning
    //PKT_UNPACK(_args_cmdHallTelemetry, argsPtr, frame);
     
}

// send robot info when queried
void cmdWhoAmI(unsigned char status, unsigned char length, unsigned char *frame) {
    // maximum string length to avoid packet size limit
    char* verstr = versionGetString();
    int verlen = strlen(verstr);

    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_WHO_AM_I, verlen, (unsigned char*)verstr, 0);
}

static void cmdSetHallGains(unsigned char status, unsigned char length, unsigned char *frame) {
    /*
    
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetPIDGains, argsPtr, frame);

    hallSetGains(0, argsPtr->Kp1, argsPtr->Ki1, argsPtr->Kd1, argsPtr->Kaw1, argsPtr->Kff1);
    hallSetGains(1, argsPtr->Kp2, argsPtr->Ki2, argsPtr->Kd2, argsPtr->Kaw2, argsPtr->Kff2);

    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_SET_HALL_GAINS, length, frame, 0);
     */
}

static void cmdSetTailQueue(unsigned char status, unsigned char length, unsigned char *frame) {
    //Tail control being deprecated from mainline project
    //TODO () : figure out an easy way of switching in tail control
    /*
    //The count is read via standard pointer math
    unsigned int count;
    count = (unsigned int) (*(frame));

    //Unpack unsigned char* frame into structured values
    //Here, unpacking happens in the loop below.
    //Due to variable size, PKT_UNPACK is not used in this CMD
    int idx = sizeof (count); //should be an unsigned int, sizeof(uint) = 2

    tailCmdT tailSeg;
    int i;
    for (i = 0; i < count; i++) {
        tailSeg = (tailCmdT) malloc(sizeof (tailCmdStruct));
        //argsPtr = (_args_cmdSetMoveQueue*)(frame+idx);
        *tailSeg = *((tailCmdT) (frame + idx));
        tailqPush(tailq, tailSeg);
        //idx =+ sizeof(_args_cmdSetMoveQueue);
        idx += sizeof (tailCmdStruct);
    }
  */
}

static void cmdSetTailGains(unsigned char status, unsigned char length, unsigned char *frame) {
    //Tail control being deprecated from mainline project
    //TODO () : figure out an easy way of switching in tail control
    /*
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetTailGains, argsPtr, frame);

    tailCtrlSetGains(argsPtr->Kp, argsPtr->Ki, argsPtr->Kd, argsPtr->Kaw, argsPtr->Kff);

    //Note that the destination is the hard-coded RADIO_DST_ADDR
    //todo : extract the destination address properly.
    radioSendData(RADIO_DST_ADDR, 0, CMD_SET_TAIL_GAINS, length, frame, 0);
     */
}


static void cmdSetOLVibe(unsigned char status, unsigned char length, unsigned char *frame){
    //Unpack unsigned char* frame into structured values
    PKT_UNPACK(_args_cmdSetOLVibe, argsPtr, frame);

//    olVibeSetFrequency(argsPtr->channel, argsPtr->incr);
//    olVibeSetPhase(argsPtr->channel, argsPtr->phase);
//    olVibeSetAmplitude(argsPtr->channel, argsPtr->amplitude);
}



////////////// FreeRTOS functions //////////////


static portTASK_FUNCTION(vCmdHandlerTask, pvParameters) { //FreeRTOS task
    //This task will execute at low priority, and handle "commands" that come in
    // from the radio task. Radio task will push commands onto this task.

    cmdStruct_t command;
    portBASE_TYPE xStatus;

    static MacPacketStruct packetStruct; //local stack storage
    static MacPacket packet = &packetStruct;
    static Payload pld;

    for (;;) { //Task loop
        
        //Blocking wait on incoming command
        xStatus = xQueueReceive(cmdQueue, packet, portMAX_DELAY);
__TRACE(0x6b);
        
        //Note: packet has payload and payload->pld_data on heap

        //Structure command
        pld = macGetPayload(packet);
        command.status = payGetStatus(pld);
        command.cmdType = payGetType(pld);
        command.length = payGetDataLength(pld);
        command.frame = payGetData(pld);

        //Invoke handler
        if (command.cmdType < MAX_CMD_FUNC) {
            cmd_func[command.cmdType](command.status, command.length, command.frame);
        }

        //Delete dynamic length parts of packet, which are on heap
        vPortFree(pld->pld_data);
        vPortFree(pld);
__TRACE(0x6c);
        
    }
}

QueueHandle_t cmdGetQueueHandle(void) {
    return cmdQueue;
}