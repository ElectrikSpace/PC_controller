#!/usr/bin/env python

# import libraries
import time
import serial
import RPi.GPIO as GPIO
import sys

# key to send
key = [chr(0x61), chr(0x33), chr(0x35), chr(0x64), chr(0x66), chr(0x30), chr(0x32), chr(0x66), chr(0x66), chr(0x63), chr(0x34), chr(0x62), chr(0x39), chr(0x66), chr(0x61), chr(0x64), chr(0x64), chr(0x31), chr(0x62), chr(0x33), chr(0x65), chr(0x63), chr(0x39), chr(0x37), chr(0x37), chr(0x66), chr(0x63), chr(0x33), chr(0x62), chr(0x33)]

# command to send
if str(sys.argv[1]) == "power" :
    command = chr(0x01)
elif str(sys.argv[1]) == "reset" :
    command = chr(0x02)
else :
    command = chr(0x00)

#setup GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT, initial=GPIO.LOW)

#setup HC12 UART
HC12 = serial.serial("/dev/ttyAMA0", baudrate=9600, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=1)

#set FU3 mode for HC12
time.sleep(0.05)
HC12.write("AT+FU3")

#send command
time.sleep(0.05)
GPIO.output(12, GPIO.HIGH)
time.sleep(0.05)
for i in range(120) : #on envoie 120 messages
    HC12.write(command)
    HC12.write(key)
    time.sleep(0.25) #espacés de 250 ms

#enter HC12 into low consumption mode
time.sleep(0.05)
GPIO.output(12, GPIO.LOW)
time.sleep(0.05)
HC12.write("AT+FU2")
time.sleep(0.05)
GPIO.output(12, GPIO.HIGH)
time.sleep(0.05)
