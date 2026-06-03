/**
 * @file wifi_app.h
 * @brief WiFi Application Manager - Header File
 *
 * This module implements the core WiFi management functionality for the ESP32
 * weather station. It operates in AP+STA (Access Point + Station) mode simultaneously,
 * allowing users to connect to the device's AP for configuration while the device
 * connects to an external WiFi network as a station.
 *
 * @details
 * Architecture:
 * The WiFi application runs as a FreeRTOS task with a message queue-based state machine.
 * Other modules communicate with it by sending messages to the queue, and it processes
 * them sequentially to manage WiFi state transitions.
 *
 * Operating Modes:
 * - Access Point (AP): Always active, provides configuration interface
 *   - SSID: "ESP32_AP", Password: "password"
 *   - IP: 192.168.0.1, hosts HTTP server for web UI
 * - Station (STA): Connects to external WiFi for internet access
 *   - Credentials loaded from NVS or provided via HTTP server
 *   - Up to 5 retry attempts on connection failure
 *
 * State Machine Events:
 * - LOAD_SAVED_CREDENTIALS → Attempt connection with NVS-stored credentials
 * - START_HTTP_SERVER → Launch web configuration interface
 * - CONNECTING_FROM_HTTP_SERVER → New credentials received from web UI
 * - STA_CONNECTED_GOT_IP → Successfully connected, save credentials
 * - USER_REQUESTED_STA_DISCONNECT → User pressed disconnect/reset
 * - STA_DISCONNECTED → Connection lost after max retries
 *
 * Event Group Bits:
 * Used to track concurrent state without race conditions between the WiFi event
 * handler (ISR context) and the main task.
 *
 * @note The WiFi application must be started before any other network-dependent module.
 */

#ifndef MAIN_WIFI_APP_H_
#define MAIN_WIFI_APP_H_

#include "esp_netif.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"

/**
 * @brief Callback function type for WiFi connected events.
 * Called when the ESP32 successfully connects to an AP and obtains an IP address.
 */
typedef void (*wifi_connected_event_callback_t)(void);

/**
 * @defgroup WIFI_AP_CONFIG WiFi Access Point Configuration
 * @brief Configuration parameters for the ESP32's SoftAP (configuration interface).
 * @{
 */
#define WIFI_AP_SSID				"ESP32_AP"			/**< AP network name visible to clients */
#define WIFI_AP_PASSWORD			"password"			/**< AP WPA2 password (min 8 characters) */
#define WIFI_AP_CHANNEL				1					/**< WiFi channel (1-13, avoid congested channels) */
#define WIFI_AP_SSID_HIDDEN			0					/**< 0 = visible, 1 = hidden SSID */
#define WIFI_AP_MAX_CONNECTIONS		5					/**< Maximum simultaneous client connections */
#define WIFI_AP_BEACON_INTERVAL		100					/**< Beacon broadcast interval in milliseconds */
#define WIFI_AP_IP					"192.168.0.1"		/**< Static IP address for the AP interface */
#define WIFI_AP_GATEWAY				"192.168.0.1"		/**< Gateway IP (same as AP IP for simple networks) */
#define WIFI_AP_NETMASK				"255.255.255.0"		/**< Subnet mask for the AP network */
#define WIFI_AP_BANDWIDTH			WIFI_BW_HT20		/**< Channel bandwidth: 20 MHz (HT20) or 40 MHz (HT40) */
/** @} */

/**
 * @defgroup WIFI_STA_CONFIG WiFi Station Configuration
 * @brief Configuration parameters for the ESP32's STA (client) mode.
 * @{
 */
#define WIFI_STA_POWER_SAVE			WIFI_PS_NONE		/**< Power save mode: NONE for lowest latency */
#define MAX_SSID_LENGTH				32					/**< Maximum SSID length per IEEE 802.11 standard */
#define MAX_PASSWORD_LENGTH			64					/**< Maximum password length per IEEE 802.11 standard */
#define MAX_CONNECTION_RETRIES		5					/**< Number of reconnection attempts before giving up */
/** @} */

/**
 * @brief Network interface objects for Station and Access Point modes.
 * @note These are externally accessible for IP info queries (e.g., by HTTP server).
 */
extern esp_netif_t* esp_netif_sta;  /**< Station mode network interface (connects to external AP) */
extern esp_netif_t* esp_netif_ap;   /**< Access Point mode network interface (hosts clients) */

/**
 * @brief Message IDs for the WiFi application task state machine.
 *
 * These messages drive the WiFi application's state transitions. They are sent
 * from various sources: the event handler (WiFi/IP events), HTTP server (user actions),
 * and internal logic (startup sequence).
 */
typedef enum wifi_app_message
{
	WIFI_APP_MSG_START_HTTP_SERVER = 0,             /**< Start the HTTP web server */
	WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER,       /**< New credentials received from web UI, attempt connection */
	WIFI_APP_MSG_STA_CONNECTED_GOT_IP,             /**< STA connected and IP address obtained */
	WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT,    /**< User requested disconnection (button or web UI) */
	WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS,           /**< Load and attempt connection with NVS credentials */
	WIFI_APP_MSG_STA_DISCONNECTED,                 /**< STA disconnected after exhausting retries */
} wifi_app_message_e;

/**
 * @brief Message structure for the WiFi application task queue.
 *
 * Contains the message ID that identifies the requested state transition.
 * Can be extended with additional fields for messages that carry data.
 */
typedef struct wifi_app_queue_message
{
	wifi_app_message_e msgID;   /**< Message/event identifier */
} wifi_app_queue_message_t;

/**
 * @brief Sends a message to the WiFi application task queue.
 *
 * Used by event handlers and other modules to trigger WiFi state transitions.
 * This function blocks until the message can be queued (portMAX_DELAY).
 *
 * @param msgID Message ID from the wifi_app_message_e enum.
 * @return pdTRUE if the message was successfully queued, pdFALSE otherwise.
 */
BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

/**
 * @brief Initializes and starts the WiFi application.
 *
 * Performs the following initialization:
 * 1. Lights the "WiFi started" RGB LED color
 * 2. Allocates WiFi configuration memory
 * 3. Creates the message queue and event group
 * 4. Launches the WiFi application FreeRTOS task
 *
 * The task then handles event handler setup, TCP/IP init, AP config, and WiFi start.
 */
void wifi_app_start(void);

/**
 * @brief Gets a pointer to the WiFi configuration structure.
 *
 * Returns the shared wifi_config_t structure used for both loading credentials
 * from NVS and setting new credentials from the HTTP server. Allocates memory
 * on first call if not already allocated.
 *
 * @return Pointer to the wifi_config_t structure (dynamically allocated).
 */
wifi_config_t* wifi_app_get_wifi_config(void);

/**
 * @brief Registers a callback function for the WiFi connected event.
 *
 * The registered callback is invoked when the ESP32 successfully connects
 * to an external AP and obtains an IP address. Used to trigger dependent
 * services (SNTP, AWS IoT).
 *
 * @param cb Function pointer to the callback (must match wifi_connected_event_callback_t signature).
 */
void wifi_app_set_callback(wifi_connected_event_callback_t cb);

/**
 * @brief Invokes the registered WiFi connected event callback.
 *
 * Called internally by the WiFi app task after successful STA connection.
 * Only calls the callback if one has been registered via wifi_app_set_callback().
 */
void wifi_app_call_callback(void);

/**
 * @brief Gets the current WiFi signal strength (RSSI) of the STA connection.
 *
 * Queries the ESP32 WiFi driver for the RSSI value of the currently connected
 * access point. This value is published to AWS IoT for signal quality monitoring.
 *
 * @return RSSI value in dBm (typically -30 to -90, where -30 is excellent).
 * @note Only valid when connected in STA mode. May assert if called while disconnected.
 */
int8_t wifi_app_get_rssi(void);

#endif /* MAIN_WIFI_APP_H_ */




























