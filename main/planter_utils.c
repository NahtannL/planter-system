#include "planter_utils.h"

int watering_times[2] = {-1, -1};
int water_duration = 1;

void button_interrupt(char* str) {
    // Print message
    printf("%s", str);

    // Loop waiting for button press
    while (true) {
        if (gpio_get_level(GPIO_NUM_1)) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void display_rgb(const int r, const int g, const int b, const int delay) {
    static led_strip_handle_t led_strip;

    // Configure strip to include RGB pin on board
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_PIN,
        .max_leds = 1
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000
    };
    led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);

    // Set pixel to RGB values
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);

    // Keep RGB on for specified number of seconds
    vTaskDelay(delay / portTICK_PERIOD_MS);

    // Turn off RGB
    led_strip_clear(led_strip);
    
    // Remove LED strip device
    led_strip_del(led_strip);
}

int init_wifi(void) {
    // Initialize NVS partition
    if (nvs_flash_init() != ESP_OK) {
        printf("ERROR initializing NVS flash.\n");
        return -1;
    }

    // Initialize TCP/IP stack
    if (esp_netif_init() != ESP_OK) {
        printf("ERROR initializing NETIF.\n");
        return -1;
    }

    // Initialize event loop for creating wifi station
    if (esp_event_loop_create_default() != ESP_OK) {
        printf("ERROR creating event loop.\n");
        return -1;
    }
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi with default config
    wifi_init_config_t default_config = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&default_config) != ESP_OK) {
        printf("ERROR initializing WiFi.\n");
        return -1;
    }

    // Set WiFi parameters
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = USER_SSID,
            .password = USER_PASS,
            .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
            .failure_retry_cnt = 3
        }
    };
    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        printf("ERROR setting WiFi mode.\n");
        return -1;
    }
    if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK) {
        printf("ERROR setting WiFi config.\n");
        return -1;
    }

    // Start WiFi
    if (esp_wifi_start() != ESP_OK) {
        printf("ERROR starting WiFi.\n");
        return -1;
    }
    
    // Attemp WiFi connection
    if (esp_wifi_connect() != ESP_OK) {
        printf("WiFi connect unsuccessful.\n");
        return -1;
    }

    vTaskDelay(pdMS_TO_TICKS(7000));

    return 0;
}

int check_wifi(void) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK) {
        if (esp_wifi_connect() != ESP_OK) {
            return -1;
        }
        return 1;
    }

    return 0;
}

void calibrate_time(void) {
    sntp_set_sync_interval(15000);
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
    
    // 16 second delay to account for sync time
    vTaskDelay(16000 / portTICK_PERIOD_MS);

    // Set timezone to PST
    setenv("TZ", "PST8PDT,M3.2.0,M11.1.0", 1);
    tzset();
}

void print_current_time(void) {
    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    printf("The current time in Los Angeles is: %s\n", strftime_buf);
}

int get_current_month(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;
    struct tm* timeinfo = localtime(&now);

    return timeinfo->tm_mon+1;
}

int get_current_day(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;
    struct tm* timeinfo = localtime(&now);

    return timeinfo->tm_mday;
}

int get_current_hour(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;
    struct tm* timeinfo = localtime(&now);

    return timeinfo->tm_hour;
}

int get_current_min(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;
    struct tm* timeinfo = localtime(&now);

    return timeinfo->tm_min;
}

float get_chip_temp() {
    temperature_sensor_handle_t temp_handle = NULL;
    temperature_sensor_config_t temp_sensor_config_low = 
        TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
    if (temperature_sensor_install(&temp_sensor_config_low, &temp_handle) != ESP_OK) {
        return -1.00;
    }

    temperature_sensor_enable(temp_handle);

    float tsens_out = -1;    
    temperature_sensor_get_celsius(temp_handle, &tsens_out);
    temperature_sensor_disable(temp_handle);

    temperature_sensor_uninstall(temp_handle);
    return tsens_out;
}

int parameter_comms() {
    // Create client for GET request
    esp_http_client_handle_t client = setup_client("parameters", 
            FIREBASE_URL, FIREBASE_API_KEY);
        
    char return_data[150];
    if (get_data(client, return_data, 150)== -1) {
        printf("ERROR executing GET request.\n");
        printf("%s\n", return_data);
        return -1;
    }

    // Values returned from server
    int nums_set[2];
    int water_time;

    char* duration_set_ptr = strstr(return_data, "\"Water_Duration_Set\"");
    if (duration_set_ptr == NULL) {
        printf("ERROR when parsing duration JSON: \"Water_Duration_Set\"");
        return -1;
    }
    char* duration_set_comma_ptr = strstr(duration_set_ptr, ":");
    char* duration_set_val = duration_set_comma_ptr + 1;
    water_time = atoi(duration_set_val);

    // Find first occurance of "Water_Times_Set"
    char* times_set_ptr = strstr(return_data, "\"Water_Times_Set\"");
    if (times_set_ptr == NULL) {
        printf("ERROR when parsing parameter JSON: \"Water_Times_Set\".\n");
        return -1;
    }
    char* times_set_val1_ptr = strstr(times_set_ptr, "[");
    if (times_set_val1_ptr == NULL) {
        printf("ERROR when parsing parameter JSON: '['.\n");
        return -1;
    }
    
    // Get first hour
    char* times_set_num_first = times_set_val1_ptr + 1;
    nums_set[0] = atoi(times_set_num_first);

    // Get second hour
    char* times_set_val2_ptr = strstr(times_set_num_first, ",");
    if (times_set_val2_ptr == NULL) {
        printf("ERROR when parsing parameter JSON: ','.\n");
        return -1;
    }
    char* times_set_num_second = times_set_val2_ptr + 1;
    nums_set[1] = atoi(times_set_num_second);

    // Update hours on ESP32 if needed
    for (int i = 0; i < 2; i++) {
        if (watering_times[i] != nums_set[i]) {
            watering_times[i] = nums_set[i];
        }
    }
    if (water_time != water_duration) {
        water_duration = water_time;
    }

    // Cleanup GET client
    esp_http_client_cleanup(client);

    // Create PATCH client
    client = setup_client("parameters", FIREBASE_URL, FIREBASE_API_KEY);

    char patch_json[150];
    snprintf(patch_json, 150, 
        "{\"Chip_Temp\": %f, "
        "\"Water_Duration_Confirm\": %d, "
        "\"Water_Times_Confirm\": [%d, %d]}",
        get_chip_temp(),
        water_duration,
        watering_times[0],
        watering_times[1]
    );
    
    if (patch_data(client, patch_json) == -1) {
        printf("ERROR patching parameters.\n");
        return -1;
    }

    esp_http_client_cleanup(client);

    return 0;

}