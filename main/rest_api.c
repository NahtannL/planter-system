#include "rest_api.h"

esp_http_client_handle_t setup_client(char* data_table_name, char* firebase_url, 
    char* firebase_api_key) {
        // Construct full URL
        char url[512];
        snprintf(url, 512, "%s%s.json?auth=%s", firebase_url, data_table_name,
            firebase_api_key);
        
        // Configuration for HTTP client
        esp_http_client_config_t config = {
            .url = url,
            .method = HTTP_METHOD_POST,
            .cert_pem = certificate_pem_start
        };

        // Client creation
        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == NULL) {
            printf("Error initializing client.\n");
            return -1;
        }

        // Set client header
        ESP_ERROR_CHECK(esp_http_client_set_header(client, "Content-Type", 
            "application/json"));
        
        return client;
    }

int post_data(esp_http_client_handle_t client, const char* json_data) {
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

void close_client(esp_http_client_handle_t client) {
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}