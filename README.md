# ESP32-S3 Plant Monitoring System

A scalable planter system integrating soil moisture sensor(s) and solenoid
valve(s) with a Firebase Realtime Database for periodic tracking and simple
control.

## Features

- Soil moisture monitoring with capacitive sensor(s)
- Solenoid valve control
- Wi-Fi connectivity for communication
- Firebase Realtime Database integration
- Sensor calibration
- Periodic Data logging with timestamps

## Equipment

### Required Hardware
* ESP32-S3-DevkitC-1-N8R8
* Gikfun Capacitive Soil Moisture Sensor v1.2
* Magnetic Solenoid Water Drain Valve
* Elegoo 4 Channel 5V 10A Relay Module 
* DC Power Supply Adapter System (more info later)

### Optional Hardware
* RGB LED
* Push Button

## Installation

### Prerequisites
- ESP-IDF v5.5
- Firebase Realtime Database
- WiFi connection

### Setup steps
1. Clone repository
```bash
git clone https://github.com/NahtannL/planter-system.git
cd planter-system
```

2. Configure credentials
Replace the following values in `main/secrets.h`
```c
#define USER_SSID "SSID"
#define USER_PASS "PASS"
#define FIREBASE_URL "https://<database_name>.firebaseio.com/"
#define FIREBASE_API_KEY "SAMPLE_API_KEY"
```

4. Configure components
In the main file, update `sensors[]`, `num_channels`, `valves[]`, `num_valves`
with the information that matches your setup

5. Initial values (First-time setup)
Before the first time you flash and execute the program on the ESP32 board,
uncomment the calibrate_sens() line, if not already uncommented, and find the
wet and dry values for your soil moisture sensors. These values are
automatically updated when calibrate_sens() is run, but if you want to remove
this step, set the mean values for each sensor and state in the `sensors[]`
list.

6. Build and flash