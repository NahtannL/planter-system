#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "hd44780.h"
#include "led_strip.h"

#define RGB_PIN 38

#define LCD_RS GPIO_NUM_0
#define LCD_E GPIO_NUM_1
#define LCD_D4 GPIO_NUM_4
#define LCD_D5 GPIO_NUM_5
#define LCD_D6 GPIO_NUM_6
#define LCD_D7 GPIO_NUM_7

// amount of values to read for calibration
#define CALIBRATION_X 5

/**
 * @brief Sensor description
 */
typedef struct {
    char name[50];
    adc_channel_t channel;
    double mean_dry;
    double mean_wet;
} sensor;

/**
 * @brief Get average of array values
 * 
 * @param arr: array of values
 * @param len: number of values
 */
double arr_avg(double* arr, int len) {
    double total = 0;
    for (int i = 0; i < len; i++) {
        total += arr[i];
    }

    return total / (double)len;
}

/**
 * @brief Get mapped value based on min and max calibrated sensor values
 * 
 * @note Mapped value cannot be greater than 1 or less than 0 (only between
 * -1 < value*100 < 101)
 * 
 * @param sens: sensor object
 * @param val: value to map
 */
double map(sensor sens, double val) {
    double mapped_val = ((val - sens.mean_dry) / (sens.mean_wet - 
        sens.mean_dry));
    
    if (mapped_val < 0) {
        mapped_val = 0.00;
    }
    else if (mapped_val > 1) {
        mapped_val = 1.00;
    }

    return mapped_val;
}

/**
 * @brief Display RGB value on ESP32
 * 
 * @param r: red value (0-255)
 * @param g: green value (0-255)
 * @param b: blue value (0-255)
 * @param delay: duration to leave LED on (in ms)
 */
void display_rgb(const int r, const int g, const int b, const int delay) {
    static led_strip_handle_t led_strip;

    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_PIN,
        .max_leds = 1
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000
    };
    led_strip_clear(led_strip);
    led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);

    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);

    vTaskDelay(delay / portTICK_PERIOD_MS);

    led_strip_clear(led_strip);
}

/**
 * @brief Wait for button press
 * 
 */
void button_interrupt(char* str) {
    printf("%s\n", str);
    while (true) {
        if (gpio_get_level(GPIO_NUM_1)) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return;
}

/**
 * @brief Initialize ADC pins on ESP32
 * 
 * @param adc_unit: ADC pin unit (eg. ADC_UNIT_1)
 * @param sensor_list: sensors to initialize
 * @param len: number of channels
 * 
 * @return adc_oneshot_unit_handle object
 */
adc_oneshot_unit_handle_t init_adc(adc_unit_t adc_unit, 
    sensor* sensor_list, int len) {
        adc_oneshot_unit_handle_t adc1_handle;

        adc_oneshot_unit_init_cfg_t unit_config = {
            .unit_id = ADC_UNIT_1
        };
        adc_oneshot_new_unit(&unit_config, &adc1_handle);

        adc_oneshot_chan_cfg_t channel_config = {
            .atten = ADC_ATTEN_DB_11,
            .bitwidth = ADC_BITWIDTH_12
        };

        for (int i = 0; i < len; i++) {
            adc_oneshot_config_channel(adc1_handle, sensor_list[i].channel, 
                &channel_config);
        }

        return adc1_handle;
}

void sens_calibrate(adc_oneshot_unit_handle_t adc_handle, 
    sensor* sensors, int len) {
        for (int i = 0; i < len; i++) {
        int output;
        double dry_vals[CALIBRATION_X];
        double wet_vals[CALIBRATION_X];

        printf("Starting calibration for sensor %d.\n", i+1);
        for (int j = 0; j < CALIBRATION_X; j++) {
            adc_oneshot_read(adc_handle, sensors[i].channel, &output);
            dry_vals[j] = (double)output;
            printf("%f\n", dry_vals[j]);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        button_interrupt("Press button to start WET calibration.");
        for (int j = 0; j < CALIBRATION_X; j++) {
            adc_oneshot_read(adc_handle, sensors[i].channel, &output);
            wet_vals[j] = (double)output;
            printf("%f\n", wet_vals[j]);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        sensors[i].mean_dry = arr_avg(dry_vals, CALIBRATION_X);
        printf("MEAN_DRY: %f\n", sensors[i].mean_dry);
        sensors[i].mean_wet = arr_avg(wet_vals, CALIBRATION_X);
        printf("MEAN_WET: %f\n", sensors[i].mean_wet);
    }
}

void app_main(void) {
    printf("Start setup... ");  
    // setup GPIO1 for button event
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLDOWN_ONLY);

    // setup ADC for moisture sensors
    adc_oneshot_unit_handle_t adc1_handle;
    sensor sensors[] = {
        {
            .name = "SENSOR_1",
            .channel = ADC_CHANNEL_3,
            .mean_dry = -1,
            .mean_wet = -1
        }
    };
    int num_channels = 1;
    adc1_handle = init_adc(ADC_UNIT_1, sensors, num_channels);

    printf("SUCCESS\n");

    button_interrupt("Press button to start calibration.");
    sens_calibrate(adc1_handle, sensors, num_channels);

    button_interrupt("Press button to trial.");

    int output;
    printf("Trialing dry:\n");
    adc_oneshot_read(adc1_handle, sensors[0].channel, &output);
    printf("Actual: %d\n", output);
    printf("Mapped: %f%%\n", map(sensors[0], (double)output) * 100);

    button_interrupt("Press button to trial wet.");

    printf("Trialing wet:\n");
    adc_oneshot_read(adc1_handle, sensors[0].channel, &output);
    printf("Actual: %d\n", output);
    printf("Mapped: %f%%\n", map(sensors[0], (double)output) * 100);


    // hd44780_t lcd = {
    //     .pins = {
    //         .rs = LCD_RS,
    //         .e = LCD_E,
    //         .d4 = LCD_D4,
    //         .d5 = LCD_D5,
    //         .d6 = LCD_D6,
    //         .d7 = LCD_D7,
    //     },
    //     .lines = 4,
    //     .backlight = true,
    // };

    // printf("Initializing LCD...\n");
    // esp_err_t ret = hd44780_init(&lcd);
    // if (ret != ESP_OK) {
    //     printf("LCD initialization failed: %s\n", esp_err_to_name(ret));
    //     return;
    // }
    // printf("Finished initialization.\n");
    
    // // Wait a bit after initialization
    // vTaskDelay(pdMS_TO_TICKS(2000));

    // printf("Writing to LCD...\n");
    // hd44780_clear(&lcd);
    // hd44780_puts(&lcd, "Testing");
    // printf("Task finished.\n");
}