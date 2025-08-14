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
 * @details Monitors soil moisture levels using multiple ADC-enabled sensors and
 * communicates with a Firebase Realtime Database through HTTP requests (GET,
 * POST, PATCH). The time and amount of watering can be controlled through the
 * database.
 * 
 * @note Hardware Requirements:
 * - ESP32-S3 microcontroller (e.g. Espressif DevkitC-1 N8R8)
 * - Soil moisture sensors
 * - Push button
 * - Magnetic solenoid water valve
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
#include "solenoid.h"

/**
 * @def RECORD_DELAY
 * @brief Delay interval between data transmission (in ms)
 * 
 */
#define RECORD_DELAY 3600000

/**
 * @brief Array of soil moisture sensors used
 * 
 */
sensor sensors[] = {
    {
        .name = "SENSOR_1",
        .channel = ADC_CHANNEL_3,
        .mean_dry = 2712,
        .mean_wet = 970
    },
    {
        .name = "SENSOR_2",
        .channel = ADC_CHANNEL_4,
        .mean_dry = 2710,
        .mean_wet = 1059
    },
    {
        .name = "SENSOR_3",
        .channel = ADC_CHANNEL_5,
        .mean_dry = 2721,
        .mean_wet = 1072
    },
    {
        .name = "SENSOR_4",
        .channel = ADC_CHANNEL_6,
        .mean_dry = 4095,
        .mean_wet = 2040
    }
};

/**
 * @brief Total number of channels
 */
const int num_channels = 4;

/**
 * @brief Array of solenoid valves
 */
valve valves[] = {
    {
        .name = "VALVE_1",
        .pin = GPIO_NUM_15,
        .sensor_obj = &sensors[0]
    },
    {
        .name = "VALVE_2",
        .pin = GPIO_NUM_16,
        .sensor_obj = &sensors[1]
    },
    {
        .name = "VALVE_3",
        .pin = GPIO_NUM_17,
        .sensor_obj = &sensors[2]
    },
    {
        .name = "VALVE_4",
        .pin = GPIO_NUM_18,
        .sensor_obj = &sensors[3]
    }
};

/**
 * @brief Total number of valves
 */
const int num_valves = 4;

/**
 * @brief Periodically update WiFi and watering parameters
 * 
 * Checks the WiFi connection and updates the watering parameters in the
 * database every minute.
 * 
 * @param[in] pvParameters unused
 */
void update_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t delay = pdMS_TO_TICKS(60000);

    while (1) {
        check_wifi();
        parameter_comms();

        xTaskDelayUntil(&xLastWakeTime, delay);
    }
}

/**
 * @brief Monitor soil moisture levels and control watering.
 * 
 * Reads sensor data and controls solenoid valves based on the scheduled
 * watering times. Sensor data is sent to the Firebase Realtime Database every
 * hour.
 * 
 * @param[in] pvParameters unused 
 */
void watering_task(void *pvParameters) {
    adc_oneshot_unit_handle_t adc1_handle = init_adc(ADC_UNIT_1, sensors, 
        num_channels);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t delay = pdMS_TO_TICKS(RECORD_DELAY);
    
    while (1) {
        check_wifi();
        if (get_current_hour() == watering_times[0] || 
            get_current_hour() == watering_times[1]) {
                for (int i = 0; i < num_valves; i++) {
                    set_valve_position(valves[i], VALVE_LOW);
                    vTaskDelay(pdMS_TO_TICKS(water_duration*1000));
                    set_valve_position(valves[i], VALVE_HIGH);
                    vTaskDelay(pdMS_TO_TICKS(water_duration*2000));
                }
        }
    
        esp_http_client_handle_t client = setup_client("sensor_data", 
            FIREBASE_URL, FIREBASE_API_KEY);
        // Read & transmit data from all sensors
        for (int i = 0; i < num_channels; i++) {
            // Formatted JSON for transmission
            char post_json[256];
            snprintf(post_json, 256,
                "{\"Name\": \"%s\", "
                "\"Month\": %d, "
                "\"Day\": %d, "
                "\"Hour\": %d, "
                "\"Moisture\": %.2f}",
                sensors[i].name,
                get_current_month(),
                get_current_day(),
                get_current_hour(),
                map(sensors[i], read_sens(adc1_handle, sensors[i].channel))*100
            );
        
            // Attempt to send data if failed once
            if (post_data(client, post_json) == -1) {
                printf("ERROR during POST request. Retrying... ");
                vTaskDelay(pdMS_TO_TICKS(5000));
                if (post_data(client, post_json) == -1) {
                    printf("FAIL.\n");
                }
                else {
                    printf("SUCCESS.\n");
                    continue;
                }
            }
    
            // Small delay between sensor transfers
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        esp_http_client_cleanup(client);
        client = NULL;
    
        // Delay
        xTaskDelayUntil(&xLastWakeTime, delay);
    }
}

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
    // ADC Sensor Configuration
    printf("ADC setup... ");
    printf("DONE.\n");


    // GPIO Configuration
    printf("GPIO setup... ");  
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLDOWN_ONLY);
    
    if (setup_valve(valves, num_valves) == -1) {
        printf("FAIL.\n");
    }
    printf("DONE.\n");


    // WiFi initialization
    // TODO: Add offline functionality
    printf("WiFi setup... ");
    for (int i = 0; i < 3; i++) {
        if (init_wifi()) {
            printf("RETRY.\n");
            printf("WiFi setup... ");
            continue;
        }
        break;
    }
    printf("DONE.\n");


    // Time synchronization
    printf("Calibrating time... ");
    calibrate_time();
    printf("DONE.\n");

    // Wait for user input to start recording
    for (int i = 0; i < 3; i++) {
        display_rgb(0, 0, 255, 500);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    // button_interrupt("Press button to start.\n");
    printf("Starting...\n");
    for (int i = 0; i < 3; i++) {
        display_rgb(255, 0, 0, 500);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    // Calibrate sensors
    // Default is to run calibration, but can be commented out for DEFAULT
    // values found in sensor.h file
    // sens_calibrate(adc1_handle, sensors, num_channels);
    
    // Set initial parameters
    parameter_comms();

    // Start background tasks
    xTaskCreate(watering_task, "HourlyWateringTask", 10240, NULL, 5, NULL);
    xTaskCreate(update_task, "MinutelyUpdatingTask", 5830, NULL, 5, NULL);
}