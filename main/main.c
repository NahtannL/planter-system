/**
 * @file main.c
 * @brief ESP32-S3 Plant Auto Watering System - Main Application
 * @author Nathan Lieu
 * @date August 10, 2025
 * @version 1.0
 * 
 * @details Monitors soil moisture levels using multiple ADC-enabled sensors and
 * sends a brief snippet of information to a firebase project realtime database
 * through a WiFi connection.
 * 
 * @note Hardware Requirements:
 * - ESP32-S3 microcontroller (e.g. Espressif DevkitC-1 N8R8)
 * - moisture sensors
 * - Push button
 * 
 * @see sensor.h
 * @see planter_units.h
 * @see rest_api.h
 * @see secrets.h
 * 
 */

#include <string.h>
#include <time.h>

#include "sensor.h"
#include "planter_utils.h"
#include "rest_api.h"
#include "secrets.h"

/**
 * @def RECORD_DELAY
 * @brief Delay interval between data transmission (in ms)
 * 
 */
#define RECORD_DELAY 3600000

/**
 * @brief Main application
 * 
 * Initializes plant monitoring and watering systems, such as the WiFi 
 * connection and time synchronization. Sensor data is read every hour and sent
 * to a Firebase Realtime Database.
 * 
 * @details System startup sequence:
 * 1. Configure GPIO button input
 * 2. Initialize ADC sensor inputs
 * 3. Connect to WiFi network
 * 4. Synchronize system time
 * 5. Enter monitoring loop
 * 
 * @note Information regarding each sensor has to be configured before it can
 * properly send the information to the server. Example of a sensor 
 * configuration:
 * @code
 * {
 *     .name = "SENSOR_1",
 *     .channel = ADC_CHANNEL_3,
 *     .mean_dry = DEFAULT_DRY,
 *     .mean_wet = DEFAULT_WET
 * }
 * @endcode
 * 
 * @warning Ensure WiFi credentials and the Firebase API keys are properly
 * configured in "secrets.h" file before deployment.
 * 
 */
void app_main(void) {
    // GPIO button configuration
    printf("GPIO setup... ");  
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLDOWN_ONLY);
    printf("DONE.\n");

    
    // ADC Sensor Configuration
    printf("ADC setup... ");
    sensor sensors[] = {
        {
            .name = "SENSOR_1",
            .channel = ADC_CHANNEL_3,
            .mean_dry = DEFAULT_DRY,
            .mean_wet = DEFAULT_WET
        },
        {
            .name = "SENSOR_2",
            .channel = ADC_CHANNEL_4,
            .mean_dry = DEFAULT_DRY,
            .mean_wet = DEFAULT_WET
        },
        {
            .name = "SENSOR_3",
            .channel = ADC_CHANNEL_5,
            .mean_dry = DEFAULT_DRY,
            .mean_wet = DEFAULT_WET
        },
        {
            .name = "SENSOR_4",
            .channel = ADC_CHANNEL_6,
            .mean_dry = DEFAULT_DRY,
            .mean_wet = DEFAULT_WET
        }
    };
    int num_channels = 4;
    adc_oneshot_unit_handle_t adc1_handle = init_adc(ADC_UNIT_1, sensors, 
        num_channels);
    printf("DONE.\n");


    // WiFi initialization
    printf("WiFi setup... ");
    if (init_wifi()) {
        return;
    }
    printf("DONE.\n");


    // Time synchronization
    printf("Calibrating time... ");
    calibrate_time();
    printf("DONE.\n");

    // Wait for user input to start recording
    for (int i = 0; i < 3; i++) {
        display_rgb(0, 0, 255, 500);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    button_interrupt("Press button to start.\n");
    printf("Starting...\n");
    for (int i = 0; i < 3; i++) {
        display_rgb(255, 0, 0, 500);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }


    // Data collection loop
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        // Delay
        vTaskDelayUntil(&xLastWakeTime, RECORD_DELAY/portTICK_PERIOD_MS);
        
        esp_http_client_handle_t client = setup_client("sensor_data", 
            FIREBASE_URL, FIREBASE_API_KEY);
        // Read & transmit data from all sensors
        for (int i = 0; i < num_channels; i++) {
            // Formatted JSON for transmission
            char post_json[256];
            snprintf(post_json, 256,
                "{\"Name\": \"%s\", "
                "\"Month\": \"%d\", "
                "\"Day\": \"%d\", "
                "\"Hour\": \"%d\", "
                "\"Minute\": \"%d\", "
                "\"Moisture\": \"%f\"}",
                sensors[i].name,
                get_current_month(),
                get_current_day(),
                get_current_hour(),
                get_current_min(),
                map(sensors[i], read_sens(adc1_handle, sensors[i].channel))*100
            );
        
            // Attempt to send data if failed once
            if (post_data(client, post_json) == -1) {
                printf("ERROR during POST request. Retrying... ");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                if (post_data(client, post_json) == -1) {
                    printf("FAIL.\n");
                    continue;
                }
                else {
                    printf("SUCCESS.\n");
                    continue;
                }
            }
            
            // Small delay between sensor transfers
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        esp_http_client_cleanup(client);
        client = NULL;

    }
}