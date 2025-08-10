# Planter System

A simple, scalable planter system that integrates soil moisture sensors and a
single 4-way solenoid valve to automate watering plants based on sensor values.
Sensor values from the system are published on a database to track moisture
levels with capabilities to create graphs based on the values.

## Equipment
* Espressif ESP32-S3-DevkitC-1-N8R8
* Gikfun Capacitive Soil Moisture Sensor v1.2
* Magnetic Solenoid Water Drain Valve
* Elegoo 4 Channel 5V 10A Relay Module 
* DC Power Supply Adapter System (more info later)

### Espressif ESP32-S3-KevkitC-1-N8R8
The ESP32-S3 was chosen given its multitude of GPIO pins and its wireless
capabilities.

### Elegoo 4 Channel 5V 10A Relay Module
Since the solenoid valves operate on a higher voltage compared to the ESP32
microcontroller, a relay module must be used to operate it. The relay module
used in this system is equipped with an octocoupler, keeping the voltages
between the solenoid valve and ESP32 isolated.

### DC Power Supply Adapter System
Powering the entire system requires two different voltages, one for the solenoid
and the other for the microcontroller. The option we opted for is a step-down
module with a 19V barrel plug power supply. The 19V is connected to the
solenoid valves, and the output from the step-down module is connected to the
microcontroller.

## Circuit
Here is the complete circuit that was used in this system:
TBD