/**
 * @file app_nvs.h
 * @brief Non-Volatile Storage (NVS) Interface for WiFi Credentials - Header File
 *
 * This module provides an interface to persist and retrieve WiFi station mode
 * credentials (SSID and password) in the ESP32's Non-Volatile Storage (NVS) flash
 * partition. This allows the device to automatically reconnect to a previously
 * configured WiFi network after power cycling or reset.
 *
 * @details
 * The NVS namespace "stacreds" is used to store:
 * - "ssid": The WiFi network name (up to 32 bytes per IEEE 802.11 standard)
 * - "password": The WiFi password (up to 64 bytes per IEEE 802.11 standard)
 *
 * Typical usage flow:
 * 1. On boot: app_nvs_load_sta_creds() attempts to load saved credentials
 * 2. On successful WiFi connection via HTTP server: app_nvs_save_sta_creds() persists them
 * 3. On user-requested disconnect/reset: app_nvs_clear_sta_creds() erases them
 *
 * @note NVS must be initialized (nvs_flash_init()) before calling any of these functions.
 */

#ifndef MAIN_APP_NVS_H_
#define MAIN_APP_NVS_H_

/**
 * @brief Saves the current WiFi station mode credentials to NVS flash.
 *
 * Retrieves the current WiFi configuration from the wifi_app module and
 * persists the SSID and password as binary blobs in the "stacreds" NVS namespace.
 * The data is committed to flash to ensure persistence across reboots.
 *
 * @return ESP_OK if credentials were successfully saved to NVS.
 * @return Other esp_err_t values if NVS open, write, or commit operations fail.
 */
esp_err_t app_nvs_save_sta_creds(void);

/**
 * @brief Loads previously saved WiFi station credentials from NVS flash.
 *
 * Opens the "stacreds" NVS namespace in read-only mode and attempts to
 * retrieve the stored SSID and password. If found, the credentials are
 * copied into the wifi_app module's WiFi configuration structure.
 *
 * @return true if valid credentials were found and loaded successfully.
 * @return false if no credentials exist, the namespace cannot be opened,
 *         or the stored SSID is empty.
 */
bool app_nvs_load_sta_creds(void);

/**
 * @brief Erases all WiFi station mode credentials from NVS flash.
 *
 * Opens the "stacreds" NVS namespace and erases all key-value pairs,
 * effectively removing the stored SSID and password. Changes are committed
 * to flash immediately.
 *
 * @return ESP_OK if credentials were successfully cleared from NVS.
 * @return Other esp_err_t values if NVS open, erase, or commit operations fail.
 */
esp_err_t app_nvs_clear_sta_creds(void);

#endif /* MAIN_APP_NVS_H_ */
