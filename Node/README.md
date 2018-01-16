# Prototype of network of sensors
The goal of this prototype is to architect and prototype the design of a network of sensors (nodes) that can record and process various measurements collectively. 
This project achieves the following:
* Power and space efficient task scheduler
* Well designed classes/routines. Focused on good encapsulation and abstraction.
* Logging/Testing Tools
* High level interface for common interfaces UART, I2C, co-processor, RF. 
This code was tested on TI CC2650 device but is designed to be portable.

## Repository Structure
* Node: Code for each individual sensor 
* Python: Logging Tools
