#include "planter_utils.h"

int button_interrupt(char* str) {
    // Print message
    printf("%s", str);

    // Loop waiting for button press
    while (true) {
        if (gpio_get_level(GPIO_NUM_1)) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            return 1;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    return 0;
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
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t default_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&default_config));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = USER_SSID,
            .password = USER_PASS,
            .scan_method = WIFI_FAST_SCAN,
            .sae_pwe_h2e = WPA3_SAE_PWE_HASH_TO_ELEMENT,
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    if (esp_wifi_connect() != ESP_OK) {
        printf("WiFi connect unsuccessful.\n");
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