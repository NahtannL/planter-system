/**
 * @file rest_api.h
 * @brief Firebase REST API Client for planter system
 * @author Nathan Lieu
 * @date August 10, 2025
 * @version 1.0
 * 
 * @details Provides HTTP client functionality for communication with Firebase
 * database. Handles secure HTTPS connections using certificates and managees
 * JSON data transmission for sensor readings.
 * 
 */

#ifndef REST_API_H
#define REST_API_H

#include <string.h>

#include "esp_http_client.h"

/**
 * @brief Start of SSL certificate for Firebase HTTPS connections
 * 
 * @note certificate must be downloaded and placed in the "cert/" folder
 * 
 */
extern const char certificate_pem_start[] asm("_binary_certificate_pem_start");

/**
 * @brief End of SSL certificate for Firebase HTTPS connections
 * 
 * @note certificate must be downloaded and placed in the "cert/" folder
 * 
 */
extern const char certificate_pem_end[]   asm("_binary_certificate_pem_end");

/**
 * @brief Configures HTTP client for Firebase communication
 * 
 * Initializes and configures the ESP HTTP client handle for communication with
 * Firebase Realtime Database. Firebase URL and authentication headers are
 * configured.
 * 
 * @param data_table_name Name of database table for data storage
 * @param firebase_url Base URL of database
 * @param firebase_api_key Firebase API key for authentication
 * 
 * @return esp_http_client_handle_t: Configured HTTP client handle
 * @retval NULL Client: Setup failed
 * 
 * @note Client must be closed with close_client function when done
 * @warning Ensure certificate is added to certs folder before function call
 *
 * @see close_client()
 * 
 */
esp_http_client_handle_t setup_client(char* data_table_name, char* firebase_url, 
    char* firebase_api_key);

/**
 * @brief Send JSON data to Firebase Realtime Database
 * 
 * Performs HTTP POST request to Firebase Realtime Database with sensor data in
 * JSON format.
 * 
 * @param client HTTP client handle
 * @param json_data JSON formatted string containing sensor data
 * 
 * @retval 0 POST request successful
 * @retval -1 POST request failed
 * 
 * @note Client must be configured before function call
 * 
 * @see setup_client()
 * 
 */
int post_data(esp_http_client_handle_t client, const char* json_data);

/**
 * @brief Clean up and close HTTP client
 * 
 * @param client HTTP client handle
 * 
 * @note Must be called to prevent memory leaks
 * @see setup_client()
 */
void close_client(esp_http_client_handle_t client);

#endif