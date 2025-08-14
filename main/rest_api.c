#include "rest_api.h"

esp_http_client_handle_t setup_client(char* data_table_name, char* firebase_url, 
    char* firebase_api_key) {
        // Construct full URL
        char url[256];
        snprintf(url, 256, "%s%s.json?auth=%s", firebase_url, data_table_name,
            firebase_api_key);

        // Configuration for HTTP client
        esp_http_client_config_t config = {
            .url = url,
            .cert_pem = certificate_pem_start
        };

        // Client creation
        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == NULL) {
            printf("Error initializing client.\n");
            return NULL;
        }

        // Set client header
        ESP_ERROR_CHECK(esp_http_client_set_header(client, "Content-Type", 
            "application/json"));
        
        return client;
    }

int post_data(esp_http_client_handle_t client, const char* json_data) {
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    // Set client post field
    esp_http_client_set_post_field(client, json_data, strlen(json_data));

    // Execute POST request
    esp_err_t status = esp_http_client_perform(client);
    if (status != ESP_OK) {
        printf("HTTP POST Unsuccessful.\n");
        printf("Status Code: %s\n", esp_err_to_name(status));
        return -1;
    }

    return 0;
}

int patch_data(esp_http_client_handle_t client, const char* json_data) {
    // Set client method to PATCH
    esp_http_client_set_method(client, HTTP_METHOD_PATCH);
    esp_http_client_set_post_field(client, json_data, strlen(json_data));

    esp_err_t status = esp_http_client_perform(client);
    if (status != ESP_OK) {
        printf("HTTP PATCH Unsuccessful.\n");
        printf("Status Code: %s\n", esp_err_to_name(status));
        return -1;
    }

    return 0;
}

int get_data(esp_http_client_handle_t client, char* buffer, int len) {
    // Set client method to GET
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    // Clear buffer
    memset(buffer, 0, len);

    // Execute GET request
    esp_http_client_open(client, 0);
    esp_http_client_fetch_headers(client);

    int content_length = esp_http_client_read(client, buffer, len-1);
    if (content_length <= 0) {
        printf("ERROR GET request failed.\n");
        return -1;
    }

    return 0;
}

void close_client(esp_http_client_handle_t client) {
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}