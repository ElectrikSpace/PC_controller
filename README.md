# A simple project to remotly switch ON and reset a PC

This uses a raspberry pi 1B as server (running a docker container) and a custom circuit board that must be setup inside the PC case.

It uses HC12 modules for communication between raspberry and custom board, so they need to be not so far one from each other.

### The project include :
- A hardware folder containing schematics and PC mask
- An app folder containing an Android studio project for custom application as client for raspberry pi
- An AVR folder containing an Atmel studio project for custom board microcontroller programmation
- A server API folder containing dockerfile and Python API for raspberry pi server container

## NOTE: This project is pretty old and only dedicated to a special use case, but you can ask me questions 
