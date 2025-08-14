#include "solenoid.h"

int setup_valve(const valve *valve_obj, int len) {
    for (int i = 0; i < len; i++) {
        if (gpio_set_direction(valve_obj[i].pin, GPIO_MODE_OUTPUT) != ESP_OK) {
            printf("ERROR setting gpio direction.\n");
            return -1;
        }
    
        if (gpio_set_pull_mode(valve_obj[i].pin, GPIO_PULLDOWN_ONLY) != ESP_OK) {
            printf("ERROR setting gpio pull mode.\n");
            return -1;
        }
    
        if (gpio_set_level(valve_obj[i].pin, VALVE_HIGH) != ESP_OK) {
            printf("ERROR setting gpio level.\n");
            return -1;
        }
    }

    return 0;
}

int set_valve_position(valve valve_obj, valve_level level) {
    esp_err_t err = gpio_set_level(valve_obj.pin, level);
    if (err == ESP_ERR_INVALID_ARG) {
        printf("ERROR setting valve position: Invalid Pin.\n");
        return -1;
    }
    else if (err != ESP_OK) {
        printf("ERROR setting valve position.\n");
        return -1;
    }

    return 0;
}