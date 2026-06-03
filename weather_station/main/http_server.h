/**
 * @file http_server.h
 * @brief HTTP Server Interface for ESP32 Weather Station - Header File
 *
 * This module implements a lightweight HTTP server running on the ESP32 that provides:
 * - A web-based user interface for WiFi configuration and monitoring
 * - Over-The-Air (OTA) firmware update capability
 * - REST API endpoints for sensor data, WiFi status, and device information
 *
 * @details
 * The HTTP server serves embedded static files (HTML, CSS, JS, jQuery) from flash
 * and exposes the following API endpoints:
 *
 * Static Resources:
 * - GET /                    → index.html (main web page)
 * - GET /jquery-3.3.1.min.js → jQuery library
 * - GET /app.css             → Application stylesheet
 * - GET /app.js              → Application JavaScript
 * - GET /favicon.ico         → Site favicon
 *
 * REST API Endpoints:
 * - GET  /dhtSensor.json        → DHT22 temperature and humidity readings
 * - GET  /wifiConnectInfo.json  → Current WiFi connection details (IP, netmask, gateway)
 * - GET  /localTime.json        → Current local time (SNTP synchronized)
 * - GET  /apSSID.json           → Access Point SSID
 * - POST /wifiConnect.json      → Initiate WiFi connection with provided credentials
 * - POST /wifiConnectStatus     → Get current WiFi connection status
 * - POST /OTAupdate             → Upload and flash new firmware binary
 * - POST /OTAstatus             → Get OTA update status and firmware build info
 * - DELETE /wifiDisconnect.json → Disconnect from current WiFi network
 *
 * Architecture:
 * The server uses a monitor task with a message queue to track asynchronous events
 * (WiFi connection changes, OTA results, time sync) and update internal state that
 * is reported via the REST API.
 *
 * @note The HTTP server runs on ESP-IDF's httpd component (lightweight HTTP/1.1 server).
 */

#ifndef MAIN_HTTP_SERVER_H_
#define MAIN_HTTP_SERVER_H_

/**
 * @defgroup OTA_Status OTA Firmware Update Status Codes
 * @brief Status values for tracking Over-The-Air firmware update progress.
 * @{
 */
#define OTA_UPDATE_PENDING 		0   /**< OTA update has not been attempted yet */
#define OTA_UPDATE_SUCCESSFUL	1   /**< OTA update completed successfully, device will restart */
#define OTA_UPDATE_FAILED		-1  /**< OTA update failed (write error, validation error, etc.) */
/** @} */

/**
 * @brief WiFi connection status as tracked by the HTTP server.
 *
 * These states are reported to the web UI via the /wifiConnectStatus endpoint
 * to show the user the current state of the WiFi STA connection.
 */
typedef enum http_server_wifi_connect_status
{
	NONE = 0,                           /**< No connection attempt has been made */
	HTTP_WIFI_STATUS_CONNECTING,        /**< Connection attempt is in progress */
	HTTP_WIFI_STATUS_CONNECT_FAILED,    /**< Connection attempt failed (wrong credentials, AP not found) */
	HTTP_WIFI_STATUS_CONNECT_SUCCESS,   /**< Successfully connected and obtained IP address */
	HTTP_WIFI_STATUS_DISCONNECTED,      /**< User-initiated disconnection completed */
} http_server_wifi_connect_status_e;

/**
 * @brief Message IDs for the HTTP server monitor task queue.
 *
 * These messages are sent from other modules (wifi_app, sntp, OTA handler)
 * to notify the HTTP server monitor of state changes that should be reflected
 * in the web UI.
 */
typedef enum http_server_message
{
	HTTP_MSG_WIFI_CONNECT_INIT = 0,     /**< WiFi connection attempt initiated */
	HTTP_MSG_WIFI_CONNECT_SUCCESS,      /**< WiFi connection succeeded (got IP) */
	HTTP_MSG_WIFI_CONNECT_FAIL,         /**< WiFi connection failed after max retries */
	HTTP_MSG_WIFI_USER_DISCONNECT,      /**< User requested WiFi disconnection */
	HTTP_MSG_OTA_UPDATE_SUCCESSFUL,     /**< OTA firmware flash completed successfully */
	HTTP_MSG_OTA_UPDATE_FAILED,         /**< OTA firmware flash failed */
	HTTP_MSG_TIME_SERVICE_INITIALIZED,  /**< SNTP time synchronization is active */
} http_server_message_e;

/**
 * @brief Message structure for the HTTP server monitor queue.
 *
 * Contains the message ID that identifies the event type. Can be expanded
 * with additional fields if events need to carry data payloads.
 */
typedef struct http_server_queue_message
{
	http_server_message_e msgID;    /**< Event/message identifier */
} http_server_queue_message_t;

/**
 * @brief Sends a message to the HTTP server monitor task queue.
 *
 * Used by other modules to notify the HTTP server of state changes
 * (WiFi events, OTA results, time sync status).
 *
 * @param msgID Message ID from the http_server_message_e enum.
 * @return pdTRUE if the message was successfully sent to the queue, pdFALSE otherwise.
 * @note This function blocks until space is available in the queue (portMAX_DELAY).
 */
BaseType_t http_server_monitor_send_message(http_server_message_e msgID);

/**
 * @brief Starts the HTTP server and its monitor task.
 *
 * Configures and launches the ESP-IDF HTTP server with all URI handlers registered.
 * Also creates the HTTP server monitor task and its message queue. Safe to call
 * multiple times - will only start the server once.
 */
void http_server_start(void);

/**
 * @brief Stops the HTTP server and cleans up resources.
 *
 * Stops the httpd server instance and deletes the monitor task.
 * After calling this function, the web UI will no longer be accessible.
 */
void http_server_stop(void);

/**
 * @brief Timer callback that restarts the ESP32 after a successful OTA update.
 *
 * Called by a one-shot timer (8 second delay) after a successful firmware update.
 * The delay allows the web page to receive the success acknowledgment before
 * the device restarts with the new firmware.
 *
 * @param arg Timer callback argument (unused).
 */
void http_server_fw_update_reset_callback(void *arg);

#endif /* MAIN_HTTP_SERVER_H_ */
