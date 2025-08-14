#include "sensor.h"

adc_oneshot_unit_handle_t init_adc(adc_unit_t adc_unit, sensor* sensor_list, 
    int len) {
        adc_oneshot_unit_handle_t adc1_handle;

        adc_oneshot_unit_init_cfg_t unit_config = {
            .unit_id = ADC_UNIT_1
        };
        adc_oneshot_new_unit(&unit_config, &adc1_handle);

        adc_oneshot_chan_cfg_t channel_config = {
            .atten = ADC_ATTEN_DB_12,
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
        // Dry calibration
        button_interrupt("Press button to start DRY calibration.\n");
        for (int i = 0; i < len; i++) {
            int output;
            double dry_vals[CALIBRATION_X];

            printf("Starting DRY calibration for %s.\n", sensors[i].name);
            for (int j = 0; j < CALIBRATION_X; j++) {
                adc_oneshot_read(adc_handle, sensors[i].channel, &output);
                dry_vals[j] = (double)output;
                printf("%f\n", dry_vals[j]);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }

            sensors[i].mean_dry = arr_avg(dry_vals, CALIBRATION_X);
            printf("MEAN_DRY: %f\n", sensors[i].mean_dry);
        }

        // Wet calibration
        button_interrupt("Press button to start WET calibration.\n");
        for (int i = 0; i < len; i++) {
            int output;
            double wet_vals[CALIBRATION_X];

            printf("Starting WET calibration for %s.\n", sensors[i].name);
            for (int j = 0; j < CALIBRATION_X; j++) {
                adc_oneshot_read(adc_handle, sensors[i].channel, &output);
                wet_vals[j] = (double)output;
                printf("%f\n", wet_vals[j]);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }

            sensors[i].mean_wet = arr_avg(wet_vals, CALIBRATION_X);
            printf("MEAN_WET: %f\n", sensors[i].mean_wet);
        }
}

int read_sens(adc_oneshot_unit_handle_t handle, adc_channel_t chan) {
    int reading;
    adc_oneshot_read(handle, chan, &reading);

    return reading;
}

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

double arr_avg(double* arr, int len) {
    double total = 0;
    for (int i = 0; i < len; i++) {
        total += arr[i];
    }

    return total / (double)len;
}