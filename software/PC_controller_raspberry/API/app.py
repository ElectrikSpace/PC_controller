#!/usr/bin/env python

import serial
import flask
from flask import request, jsonify
import time
import sys
import os
import RPi.GPIO as GPIO

def send_request(action) :
    response = {
        'status': 200,
    }
    if action == "power" or action == "reset" or action == "battery_level":
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(18, GPIO.OUT, initial=GPIO.HIGH)
        time.sleep(0.05)
        HC12 = serial.Serial("/dev/ttyAMA0", baudrate=4800, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=1)
        time.sleep(0.05)
        HC12.write(str.encode("a"))
        time.sleep(1)
        key = "a35df02ffc4b9fadd1b3f"
        if action == "power" :
            HC12.write(str.encode("p" + key))
        elif action == "reset" :
            HC12.write(str.encode("r" + key))
        elif action == "battery_level" :
            HC12.write(str.encode("b"))
        t0 = time.time()
        buffer = []
        while (time.time() < t0 + 3) :
            for element in HC12.read():
                buffer.append(element)
        HC12.close()
        GPIO.cleanup()
        if len(buffer) == 0 :
            response['success'] = False
            response['error'] = "no response"
        else :
            if buffer[0] == 111 : # 111 = 0x6f which is ASCII for o
                response['success'] = True # o is for OK
            elif buffer[0] == 98 : # batterie level
                try:
                    response["success"] = True
                    response["value"] = buffer[1]*256 + buffer[2]
                except:
                    response["success"] = False
                    response["error"] = "invalid reponse"
            elif buffer[0] == 97 : # should not by possible
                response["success"] = False
                response["error"] = "bad action (uC)"
            elif buffer[0] == 107 : # should not by possible
                response["success"] = False
                response["error"] = "bad action (uC)"
            else :
                response["success"] = False
                response["error"] = "unknown response"
    else :
        response['success'] = False
        response['error'] = "bad action (server)"
    return response

app = flask.Flask(__name__)
app.config['DEBUG'] = True

@app.route('/PCcontroller', methods=['GET'])
def ping():
    response = {
        'status': 200
    }
    if 'API_key' in request.form and request.form['API_key'] == 'a067db7c':
        ping = os.system("ping -c 1 192.168.1.21") # replace by PC local ip adress
        if ping == 0 :
            response['success'] = True
        else :
            response['success'] = False
    else :
        response['status'] = 403
        response['success'] = False
        response['error'] = "bad API key"
    return jsonify(response)

@app.route('/PCcontroller', methods=['POST'])
def API():
    response = {
        'status': 200,
    }
    if 'API_key' in request.form and request.form['API_key'] == 'a067db7c-bffd-4f26-8151-20a487679dc3':
        if 'action' in request.form :
            response = send_request(request.form['action'])
        else :
            response['success'] = False
            response['error'] = "action not defined"
    else :
        response['status'] = 403
        response['success'] = False
        response['error'] = "bad API key"
    return jsonify(response)

app.run(host='0.0.0.0', port=80)
