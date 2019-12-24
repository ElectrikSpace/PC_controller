#!/usr/bin/env python

# import libraries
import time
import serial
import RPi.GPIO as GPIO
import sys

GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT, initial=GPIO.HIGH)
time.sleep(0.05)
HC12 = serial.Serial("/dev/ttyAMA0", baudrate=9600, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=1)
time.sleep(0.05)
HC12.write(str.encode("test"))
