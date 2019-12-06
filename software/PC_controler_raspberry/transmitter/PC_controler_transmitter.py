#!/usr/bin/env python

# import libraries
import time
import serial
import RPi.GPIO as GPIO
import sys

#command to be send
command = str(sys.argv[1])

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
HC12.write(command)

#enter HC12 into low consumption mode
time.sleep(0.05)
GPIO.output(12, GPIO.LOW)
time.sleep(0.05)
HC12.write("AT+FU2")
time.sleep(0.05)
GPIO.output(12, GPIO.HIGH)
time.sleep(0.05)
