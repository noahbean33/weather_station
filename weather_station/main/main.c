/**
 * @file main.c
 * @brief ESP32 Weather Station - Application Entry Point
 *
 * This is the main application file for the ESP32-based weather station project.
 * It initializes all system components and starts the application tasks.
 *
 * @details
 * System Initialization Sequence:
 * 1. Initialize Non-Volatile Storage (NVS) flash partition
 * 2. Start the WiFi application (AP+STA mode with HTTP server)
 * 3. Configure the WiFi reset button (BOOT button interrupt)
 * 4. Start the DHT22 temperature/humidity sensor reading task
 * 5. Register a callback for WiFi connection events
 *
 * Post-WiFi-Connection Actions (triggered by callback):
 * - Start SNTP time synchronization with pool.ntp.org
 * - Start AWS IoT MQTT client for cloud telemetry
 *
 * Hardware Requirements:
 * - ESP32 DevKit with DHT22 sensor on GPIO 25
 * - RGB LED on GPIOs 21, 22, 23
 * - BOOT button (GPIO 0) used for WiFi reset
 * - WiFi access point within range (for STA mode)
 */

#include "esp_log.h"
#include "nvs_flash.h"

#include "aws_iot.h"
#include "DHT22.h"
#include "sntp_time_sync.h"
#include "wifi_app.h"
#include "wifi_reset_button.h"

/** @brief Log tag for main application ESP_LOG messages */
static const char TAG[] = "main";

/**
 * @brief Callback function invoked when WiFi STA successfully connects and obtains an IP.
 *
 * This function is registered with wifi_app_set_callback() and is called once
 * the ESP32 has connected to an external access point and received an IP address.
 * It triggers the start of network-dependent services:
 * - SNTP time synchronization (for accurate timestamps)
 * - AWS IoT MQTT client (for publishing sensor data to the cloud)
 */
void wifi_application_connected_events(void)
{
	ESP_LOGI(TAG, "WiFi Application Connected!!");
	sntp_time_sync_task_start();
	aws_iot_start();
}

/**
 * @brief Main application entry point (called by ESP-IDF after system initialization).
 *
 * Performs the complete system initialization in the following order:
 * 1. NVS flash init (required for WiFi and credential storage)
 * 2. WiFi application start (AP mode + attempt STA connection from saved credentials)
 * 3. WiFi reset button configuration (BOOT button ISR for credential clearing)
 * 4. DHT22 sensor task start (begins periodic temperature/humidity readings)
 * 5. WiFi connected callback registration (enables cloud services after connection)
 *
 * @note If NVS flash is corrupted or has incompatible version, it is erased and re-initialized.
 */
void app_main(void)
{
    // Initialize NVS - required for WiFi driver and credential persistence
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start WiFi application (creates AP, attempts STA from saved creds, starts HTTP server)
	wifi_app_start();

	// Configure WiFi reset button (BOOT button GPIO 0 with negative-edge interrupt)
	wifi_reset_button_config();

	// Start DHT22 sensor task (reads temperature and humidity every 4 seconds)
	DHT22_task_start();

	// Register callback for WiFi connected event (starts SNTP + AWS IoT)
	wifi_app_set_callback(&wifi_application_connected_events);
}

