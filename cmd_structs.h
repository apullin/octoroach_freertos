/* 
 * File:   cmd_structs.h
 * Author: pullin
 *
 * Created on October 28, 2014, 4:25 PM
 */

#ifndef CMD_STRUCTS_H
#define	CMD_STRUCTS_H

//// Includes here should be to provide TYPES and ENUMS only
//#include "move_queue.h"
//#include "tail_queue.h"
//#include "hall.h"

/////// Argument structures

//cmdSetThrustOpenLoop
typedef struct{
	int channel, dc;
} _args_cmdSetThrustOpenLoop;

//cmdSetThrustClosedLoop
typedef struct{
	int chan1;
        unsigned int runtime1;
        int chan2;
        unsigned int runtime2;
        unsigned int telem_samples;
} _args_cmdSetThrustClosedLoop;

//cmdSetPIDGains
typedef struct{
	int Kp1, Ki1, Kd1, Kaw1, Kff1;
	int Kp2, Ki2, Kd2, Kaw2, Kff2;
} _args_cmdSetPIDGains;

//cmdGetPIDTelemetry
//obsolete

//cmdSetCtrldTurnRate
typedef struct{
	int steerInput;
} _args_cmdSetCtrldTurnRate;

//cmdStreamTelemetry
typedef struct{
    unsigned long count;
} _args_cmdStreamTelemetry;

//cmdSetMoveQueue
//NOTE: This is not for the entire packet, just for one moveQ items,
// the cmd handler will stride across the packet, unpacking these
//typedef struct{
//	int inputL, inputR;
//	unsigned long duration;
//	enum moveSegT type;
//	int params[3];
//} _args_cmdSetMoveQueue;

//cmdSetSteeringGains
typedef struct{
	int Kp, Ki, Kd, Kaw, Kff;
        int steerMode;
} _args_cmdSetSteeringGains;

//cmdSoftwareReset
//no arguments

//cmdSpecialTelemetry
typedef struct{
	unsigned long count;
} _args_cmdSpecialTelemetry;

//cmdEraseSector
typedef struct{
	unsigned long samples;
} _args_cmdEraseSector;

//cmdFlashReadback
typedef struct{
	unsigned long samples;
} _args_cmdFlashReadback;

//cmdSleep

//cmdSetVelProfile
//typedef struct{
//    int intervalsL[NUM_VELS];
//    int deltaL[NUM_VELS];
//    int velL[NUM_VELS];
//    int intervalsR[NUM_VELS];
//    int deltaR[NUM_VELS];
//    int velR[NUM_VELS];
//} _args_cmdSetVelProfile;

//cmdHallTelemetry
typedef struct {
    unsigned long startDelay; // recording start time
    int count; // count of samples to record
    int skip; // samples to skip
} _args_cmdHallTelemetry;

//cmdSetTailQueue
//NOTE: This is not for the entire packet, just for one tailQ item,
// the cmd handler will stride across the packet, unpacking these
//typedef struct {
//    float angle;
//    unsigned long duration;
//    enum tailSegT type;
//    int params[3];
//} _args_cmdSetTailQueue;

//cmdSetTailGains
typedef struct{
	int Kp, Ki, Kd, Kaw, Kff;
} _args_cmdSetTailGains;

//cmdSetThrustHall
typedef struct{
	int chan1;
        unsigned int runtime1;
        int chan2;
        unsigned int runtime2;
} _args_cmdSetThrustHall;

//cmdSetOLVibe
typedef struct{
	int channel;
        int incr;
        int amplitude;
        int phase;
} _args_cmdSetOLVibe;

#endif	/* CMD_STRUCTS_H */

