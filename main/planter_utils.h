/**
 * @file planter_utils.h
 * @brief System utility functions
 * @author Nathan Lieu
 * @date August 10, 2025
 * @version 1.0
 * 
 * @details Provides essential utility functions for GPIO, time, and RGB.
 * 
 */

#ifndef PLANTER_H
#define PLANTER_H

#include <stdio.h>

#include "driver/gpio.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "led_strip.h"
#include "esp_sntp.h"
#include "esp_tls.h"
#include "nvs_flash.h"
#include "secrets.h"


/**
 * @def RGB_PIN
 * @brief GPIO pin number for RGB control
 * 
 * @note This pin number can be different for every development board. Please
 * check to make sure this matches your boards.
 */
#define RGB_PIN 38

/**
 * @brief Wait for button press with prompt message
 * 
 * Displays a prompt message and blocks execution of code until the button is
 * pressed.
 * 
 * @param str Prompt message to display
 * 
 */
void button_interrupt(char* str);

/**
 * @brief Display RGB value on ESP32
 * 
 * Sets the RGB to the specified color value and keeps the RGB on for a given
 * duration before turning it off.
 * 
 * @param r Red color intensity (0-255)
 * @param g Green color intensity (0-255)
 * @param b Blue color intensity (0-255)
 * @param delay Duration to keep RGB on (in ms)
 * 
 * @warning Ensure RGB_PIN is configured before calling.
 */
void display_rgb(const int r, const int g, const int b, const int delay);

/**
 * @brief Initialize WiFi module on ESP32
 * 
 * Configures and establishes WiFi connection on ESP32 using credentials
 * specified in secrets.h file. Initializes NVS flash and WiFi driver.
 * 
 * @retval 0 WiFi connection successful
 * @retval 1 WiFi connection unsuccessful
 * 
 * @note WiFi credentials must be defined in secrets.h
 * @warning Must be called before communicating with Firebase servers
 * 
 */
int init_wifi(void);


/**
 * @brief Synchronize system time using SNTP
 * 
 * Configures SNTP client and synchronizes ESP32's system time with the network
 * server, "pool.ntp.org", for timestamp generation.
 * 
 * @note WiFi must be initialized before calibration.
 * 
 */
void calibrate_time(void);

/**
 * @brief Print current system time to console
 * 
 * Outputs formatted current date and time in Los Angeles to console.
 * 
 * @note Mainly for debugging purposes
 * 
 */
void print_current_time(void);


/**
 * @brief Get current month
 * 
 * @return Current month as integer (1-12)
 * 
 */
int get_current_month(void);

/**
 * @brief Get current day of month
 * 
 * @return Current day as integer (1-31)
 * 
 */
int get_current_day(void);

/**
 * @brief Get current hour
 * 
 * @return Current hour as integer (0-23)
 * 
 */
int get_current_hour(void);

/**
 * @brief Get current minute
 * 
 * @return Current minute as integer (0-59)
 * 
 */
int get_current_min(void);

#endif