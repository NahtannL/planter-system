/**
 * @file solenoid.h
 * @brief Solenoid valve control interface
 * @author Nathan Lieu
 * @date August 10, 2025
 * 
 * Privdes functions for controlling solenoid valves via GPIO.
 */

#ifndef SOLENOID_H
#define SOLENOID_H

#include "driver/gpio.h"
#include "sensor.h"

#define VALVE_HIGH_NUM 1
#define VALVE_LOW_NUM 0

typedef enum {
    VALVE_HIGH = VALVE_HIGH_NUM,
    VALVE_LOW = VALVE_LOW_NUM
} valve_level;

typedef struct {
    char name[50];
    gpio_num_t pin;
    sensor* sensor_obj;
} valve;

/**
 * @brief Setup solenoid valve for GPIO contrl
 * 
 * Configures an array of GPIO pins as control pins for solenoid valves. Each
 * pin is set to output, pulldown, and HIGH.
 * 
 * @param[in] valve_obj Array of valves
 * @param[in] len Number of valves
 * 
 * @retval 0 Success
 * @retval -1 Fail
 * 
 */
int setup_valve(const valve *valve_obj, int len);

/**
 * @brief Set the position of a solenoid valve
 * 
 * Sets the specified valve to the given level (HIGH/LOW).
 * 
 * @param[in] valve_obj Valve struct
 * @param[in] level Desired valve level (VALVE_HIGH/VALVE_LOW)
 * 
 * @retval 0 Success
 * @retval -1 Fail
 * 
 */
int set_valve_position(valve valve_obj, valve_level level);

#endif