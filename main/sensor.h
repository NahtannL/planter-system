/**
 * @file sensor.h
 * @brief ADC sensor management for moisture monitoring
 * @author Nathan Lieu
 * @date August 10, 2025
 * @version 1.0
 * 
 * @details Provides ADC sensor initialization, calibration, and data processing
 * functions for monitoring. Supports multiple sensor channels and capabilities
 * to add additional ADC units (if installed on board). Adds calibration
 * functionality for accurate sensor readings.
 * 
 */

#ifndef SENSOR_H
#define SENSOR_H

#include "esp_adc/adc_oneshot.h"
#include "planter_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"

/**
 * @def CALIBRATION_X
 * @brief Number of readings to take during sensor calibration
 * 
 */
#define CALIBRATION_X 5

/**
 * @def DEFAULT_DRY
 * @brief Default dry soil ADC reading value
 * 
 * Default calibration value for dry soil conditions when calibration data is
 * not available.
 * 
 * @warning Value must be found through calibration when first running the
 * system and should only be used when calibration is unavailable.
 * 
 * @note Sensor readings vary for each sensor and an average value must be
 * gauged by the user.
 * 
 */
#define DEFAULT_DRY 2615

/**
 * @def DEFAULT_WET
 * @brief Default wet soil ADC reading value
 * 
 * Default calibration value for wet soil conditions when calibration data is
 * not available.
 * 
 * @warning Value must be found through calibration when first running the
 * system and should only be used when calibration is unavailable.
 * 
 * @note Sensor readings vary for each sensor and an average value must be
 * gauged by the user.
 * 
 */
#define DEFAULT_WET 1040

/**
 * @brief Soil moisture sensor configuration structure
 * 
 * Contains necessary information for one moisture sensor including
 * identification, ADC channel assignment, and calibration parameters.
 * 
 */
typedef struct {
    char name[50];              /**< Sensor identification string */
    adc_channel_t channel;      /**< ADC channel number (e.g. ADC_CHANNEL_3) */
    double mean_dry;            /**< Calibrated dry ADC reading */
    double mean_wet;            /**< Calibrated wet ADC reading */
} sensor;

/**
 * @brief Initialize ADC pins on ESP32
 * 
 * Configures specified ADC unit(s) and channel(s) for soil moisture sensors.
 * ADC pins are setup with appropriate bit width and attenuation for accurate
 * moisture level measurements.
 * 
 * @param adc_unit ADC pin unit (e.g. ADC_UNIT_1)
 * @param sensor_list Array of sensor strctures
 * @param len Number of sensors in array
 * 
 * @return adc_oneshot_unit_handle object: Configured ADC unit handle
 * @retval NULL: initialization failed
 * 
 * @note Must be called before sensor readings
 * @warning Ensure all sensor units and channels are valid
 * @see read_sens()
 * 
 */
adc_oneshot_unit_handle_t init_adc(adc_unit_t adc_unit, 
    sensor* sensor_list, int len);

/**
 * @brief Calibrate moisture sensors
 * 
 * Interactive calibration of moisture sensors with user prompts for dry and wet
 * conditions. Calculates and stores mean calibration values for each sensor
 * for accurate sensor mapping.
 * 
 * @param adc_handle ADC unit handle
 * @param sensors Array of sensor structures
 * @param len Number of sensors
 * 
 * @warning init_adc() must be called before function call
 */
void sens_calibrate(adc_oneshot_unit_handle_t adc_handle, 
    sensor* sensors, int len);

/**
 * @brief Read raw ADC value from moisture sensor
 * 
 * Performs a single shot ADC reading from a specified sensor channel and
 * returns the raw digital value. Should be used with map() function.
 * 
 * @param handle ADC unit handle
 * @param chan ADC channel
 * 
 * @return Raw sensor reading
 * @retval -1: Reading failed
 * 
 * @see map()
 * @see init_adc()
 * 
 */
int read_sens(adc_oneshot_unit_handle_t handle, adc_channel_t chan);

/**
 * @brief Converts raw sensor reading to a moisture percentage
 * 
 * Maps raw sensor reading to moisture percentage (0 - 1) using calibrated
 * data.
 * 
 * @param sens Sensor structure
 * @param val Raw ADC reading value
 * 
 * @return Moisture percentage (0.0 - 1.0)
 * 
 * @warning Mapped values are clamped from 0 to 1
 * @note Return value must be multiplied by 100 for percentage
 * 
 * @see sens_calibrate()
 * 
 */
double map(sensor sens, double val);

/**
 * @brief Calculate average of array values
 * 
 * Utility function to compute the mean of an array of double values.
 * 
 * @param arr Array of double values
 * @param len Number of elements in array
 * 
 * @return Mean of array values
 * 
 */
double arr_avg(double* arr, int len);

#endif