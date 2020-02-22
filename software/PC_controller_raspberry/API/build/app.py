#!/usr/bin/env python

import serial
import flask
from flask import request, jsonify
import time
import sys
import os
import RPi.GPIO as GPIO

def send_request(action) : # communication to micro-controller
    response = {
        'status': 200, # HTTP status OK
    }
    if action == "power" or action == "reset" or action == "battery_level": # three possible actions
        # init HC12 serial communication
        GPIO.setmode(GPIO.BCM) # use GPIO from RPi
        GPIO.setup(18, GPIO.OUT, initial=GPIO.HIGH) # init SET pin to HIGH
        time.sleep(0.05)
        HC12 = serial.Serial("/dev/ttyAMA0", baudrate=4800, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=1) # init serial
        time.sleep(0.05)
        HC12.write(str.encode("a")) # impulsion to make micro-controller wake up
        time.sleep(1) # wait until wake up complete

        # send data : action + key
        key = "a35df02ffc4b9fadd1b3f" # security key for authentification with micro-controller
        if action == "power" :
            HC12.write(str.encode("p" + key))
        elif action == "reset" :
            HC12.write(str.encode("r" + key))
        elif action == "battery_level" :
            HC12.write(str.encode("b"))

        # ear of any reponse for 3 seconds
        t0 = time.time()
        buffer = []
        while (time.time() < t0 + 3) :
            for element in HC12.read():
                buffer.append(element)

        # close serial Bus and GPIO use
        HC12.close()
        GPIO.cleanup()

        # reading of the response
        if len(buffer) == 0 : # no response received (problem with antenna ?)
            response['success'] = False
            response['error'] = "no response"
        else :
            if buffer[0] == 111 : # 111 = 0x6f which is ASCII for o
                response['success'] = True # o is for OK
            elif buffer[0] == 98 : # battery level code
                # 2 bytes must be received
                try:
                    response["success"] = True
                    battery_value = buffer[1]*256 + buffer[2]
                    if battery_value >= 975 :
                        response["value"] = 100
                    elif battery_value <= 750 :
                        response["value"] = 0
                    else :
                        response["value"] = ( battery_value - 750) // 2.24
                except:
                    response["success"] = False
                    response["error"] = "invalid reponse"
            elif buffer[0] == 97 : # should not by possible (incorrect action in request)
                response["success"] = False
                response["error"] = "bad action (uC)"
            elif buffer[0] == 107 : # should not by possible (incorrect key in request)
                response["success"] = False
                response["error"] = "bad key (uC)"
            else : # should not be possible
                response["success"] = False
                response["error"] = "unknown response"
    else : # invalid action sent to API
        response['success'] = False
        response['error'] = "bad action (server)"
    return response

# init flask app
app = flask.Flask(__name__)
app.config['DEBUG'] = False # change to True for debug, but need more CPU

# GET method : check if PC is ready with a ping
@app.route('/PCcontroller', methods=['GET'])
def API_get():
    content = request.get_json()
    response = { # HTTP status OK
        'status': 200,
    }
    if 'API_key' in content and content['API_key'] == 'a067db7c': # API authentification
        ping = os.system("ping -c 1 192.168.1.5") # ping with local IP
        if ping == 0 : # OK
            response['success'] = True
        else : # no response or error
            response['success'] = False
    else :
        response['status'] = 403
        response['success'] = False
        response['error'] = "bad API key"
    return jsonify(response)

 # POST method : do a specific action
@app.route('/PCcontroller', methods=['POST'])
def API_post():
    content = request.get_json()
    response = { # HTTP status OK
        'status': 200,
    }
    if 'API_key' in content and content['API_key'] == 'a067db7c-bffd-4f26-8151-20a487679dc3': # API authentification
        if 'action' in content : # must be in request body
            response = send_request(content['action'])
        else :
            response['success'] = False
            response['error'] = "action not defined"
    else :
        response['status'] = 403
        response['success'] = False
        response['error'] = "bad API key"
    return jsonify(response)

# run flask App
app.run(host='0.0.0.0', port=80)
