#!/usr/bin/env python
"""
authors: apullin

Contents of this file are copyright Andrew Pullin, 2013

"""
from lib import command
import time,sys,traceback
import serial
import shared

from or_helpers import *


###### Operation Flags ####
SAVE_DATA1 = False 
RESET_R1 = False

EXIT_WAIT   = True

def main():    
    xb = setupSerial(shared.BS_COMPORT, shared.BS_BAUDRATE)
    
    R1 = Robot('\x20\x52', xb)
    
    shared.ROBOTS = [R1] #This is necessary so callbackfunc can reference robots
    shared.xb = xb           #This is necessary so callbackfunc can halt before exit
    
    R1.runtime = 200;
    
    #calculate the number of telemetry packets we expect
    R1.numSamples = int(ceil(R1.telemSampleFreq * (R1.runtime) / 1000.0))
    #allocate an array to write the downloaded telemetry data into
    R1.imudata = [ [] ] * R1.numSamples
    R1.moveq = []
    R1.clAnnounce()
    print "Telemtry samples to save: ",R1.numSamples
    
    R1.eraseFlashMem(timeout = 15)

    # Pause and wait to start run, including leadin time
    print ""
    print "  ***************************"
    print "  *******    READY    *******"
    print "  ***************************"
    raw_input("  Press ENTER to start run ...")
    print ""
    
    # Trigger telemetry save, which starts as soon as it is received
    
    #### Make when saving anything, this if is set ####
    #### to the proper "SAVE_DATA"                 ####
    
    R1.startTelemetrySave()
    
    raw_input("Press Enter to start telemtry readback ...")
    R1.downloadTelemetry(timeout = 2, retry = False)
    
    #pktNum = 1;
    # while True:
        # try:
            # echoString = "Echo test #%d" % pktNum
            # print "Sending: ",echoString
            # pktNum = pktNum + 1
            # R1.sendEcho(echoString)
            # time.sleep(0.5)
        # except KeyboardInterrupt:
                # print "Stopping echo, going to listener mode"
                # break

    if EXIT_WAIT:  #Pause for a Ctrl+C if specified
        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                print "Got Ctrl + C"
                break

    xb_safe_exit()
    

#Provide a try-except over the whole main function
# for clean exit. The Xbee module should have better
# provisions for handling a clean exit, but it doesn't.
#TODO: provide a more informative exit here; stack trace, exception type, etc
if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print "\nRecieved Ctrl+C, exiting."
        shared.xb.halt()
        shared.ser.close()
    except Exception as args:
        print "\nGeneral exception:",args
        print "\n    ******    TRACEBACK    ******    "
        traceback.print_exc()
        print "    *****************************    \n"
        print "Attempting to exit cleanly..."
        shared.xb.halt()
        shared.ser.close()
        sys.exit()
    except serial.serialutil.SerialException:
        shared.xb.halt()
        shared.ser.close()
