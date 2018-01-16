import serial
import datetime
import os
import time

import argparse

# Folder where log files are stored
PATH = 'logs/'

# Name of the serial port
SERIAL_NAME = '/dev/tty.usbmodemL3000401'
SERIAL_TIMEOUT = 1

parser = argparse.ArgumentParser()
parser.add_argument("--serial", help="Name of Serial Port")
args = parser.parse_args()
if args.serial:
    SERIAL_NAME = '/dev/tty.usbmodem'+ args.serial

ser = serial.Serial(SERIAL_NAME, baudrate=9600, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, bytesize = serial.EIGHTBITS, timeout = SERIAL_TIMEOUT)
ser.read()
time.sleep(1)
print(ser.in_waiting)
ser.reset_input_buffer()
print(ser.in_waiting)

if not os.path.exists(PATH):
    os.makedirs(PATH)

#f = open(PATH + 'a', 'w+')
try:
    while True:
        message = b''
        while True:
            rawByte = ser.read()
            #print(rawByte)
            if(rawByte is b''):
                break
            elif(rawByte == b'\n'):
                break
            else:
                message = message + rawByte
        if message == b'':
            pass
        else:
            timestamp = str(datetime.datetime.now())
            messageInAscii = message.decode('ascii')
            print(timestamp + '    '+messageInAscii)
        #f.write(message.decode('utf-8'))
except KeyboardInterrupt:
    pass

ser.close()
#f.close()
