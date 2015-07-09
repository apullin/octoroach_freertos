#!/usr/bin/env python
"""
authors: apullin

Contents of this file are copyright Andrew Pullin, 2013

"""
from lib import command
import time,sys,os,traceback
import random
import serial

# Path to imageproc-settings repo must be added
sys.path.append(os.path.dirname("../../imageproc-settings/"))
sys.path.append(os.path.dirname("../imageproc-settings/"))  
import shared_multi as shared

from or_helpers import *


###### Operation Flags ####
SAVE_DATA1 = False 
RESET_R1 = False

EXIT_WAIT   = True
SEND_PKTS   = True

def main():    
    xb = setupSerial(shared.BS_COMPORT, shared.BS_BAUDRATE)
    
    R1 = Robot('\x20\x52', xb)
    
    shared.ROBOTS = [R1] #This is necessary so callbackfunc can reference robots
    shared.xb = xb       #This is necessary so callbackfunc can halt before exit
    
    pktNum = 1
    
    if SEND_PKTS:
        while True:
            try:
                echoString = "Echo test #%d" % pktNum
                print "Sending: ",echoString
                pktNum = pktNum + 1
                R1.sendEcho(echoString)
                #time.sleep(random.random()*3.0)
                time.sleep(0.25)
            except KeyboardInterrupt:
                    print "Stopping echo, going to listener mode"
                    break

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
