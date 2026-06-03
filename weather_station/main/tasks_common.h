/**
 * @file tasks_common.h
 * @brief FreeRTOS Task Configuration - Common Definitions
 *
 * This header centralizes the configuration parameters for all FreeRTOS tasks
 * in the ESP32 weather station application. It defines stack sizes, priorities,
 * and core affinities for each task, making it easy to tune resource allocation.
 *
 * @details
 * Task Distribution Across ESP32 Dual Cores:
 *
 * Core 0 (Protocol CPU - handles WiFi/BT by default):
 * - WiFi Application Task (priority 5)
 * - HTTP Server Task (priority 4)
 * - HTTP Server Monitor Task (priority 3)
 * - WiFi Reset Button Task (priority 6)
 *
 * Core 1 (Application CPU):
 * - DHT22 Sensor Task (priority 5)
 * - SNTP Time Sync Task (priority 4)
 * - AWS IoT Task (priority 6)
 *
 * Priority Guidelines (higher number = higher priority):
 * - Priority 6: Critical real-time tasks (button ISR handler, AWS IoT)
 * - Priority 5: Regular periodic tasks (WiFi management, sensor reading)
 * - Priority 4: Background tasks (HTTP server, time sync)
 * - Priority 3: Low-priority monitoring (HTTP monitor)
 *
 * Stack Size Guidelines:
 * - 2048 bytes: Simple tasks with minimal local variables
 * - 4096 bytes: Standard tasks with moderate stack usage
 * - 8192 bytes: Tasks handling large buffers (HTTP server with OTA)
 * - 9216 bytes: Tasks with TLS/crypto operations (AWS IoT with MQTT+TLS)
 *
 * @note Stack sizes should be validated using uxTaskGetStackHighWaterMark()
 *       during development to ensure adequate headroom.
 */

#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

/**
 * @defgroup WIFI_APP_TASK WiFi Application Task Configuration
 * @brief Manages WiFi connection state machine (AP+STA mode, event handling).
 * @{
 */
#define WIFI_APP_TASK_STACK_SIZE			4096    /**< Stack size in bytes */
#define WIFI_APP_TASK_PRIORITY				5       /**< Task priority (FreeRTOS) */
#define WIFI_APP_TASK_CORE_ID				0       /**< Pinned to Core 0 (protocol CPU) */
/** @} */

/**
 * @defgroup HTTP_SERVER_TASK HTTP Server Task Configuration
 * @brief Handles HTTP requests, serves web UI, processes OTA uploads.
 * @{
 */
#define HTTP_SERVER_TASK_STACK_SIZE			8192    /**< Stack size in bytes (large for OTA buffer) */
#define HTTP_SERVER_TASK_PRIORITY			4       /**< Task priority (FreeRTOS) */
#define HTTP_SERVER_TASK_CORE_ID			0       /**< Pinned to Core 0 (protocol CPU) */
/** @} */

/**
 * @defgroup HTTP_MONITOR_TASK HTTP Server Monitor Task Configuration
 * @brief Processes async event messages and updates HTTP server state.
 * @{
 */
#define HTTP_SERVER_MONITOR_STACK_SIZE		4096    /**< Stack size in bytes */
#define HTTP_SERVER_MONITOR_PRIORITY		3       /**< Task priority (FreeRTOS, lowest) */
#define HTTP_SERVER_MONITOR_CORE_ID			0       /**< Pinned to Core 0 (protocol CPU) */
/** @} */

/**
 * @defgroup WIFI_RESET_TASK WiFi Reset Button Task Configuration
 * @brief Monitors BOOT button press to trigger WiFi credential clearing.
 * @{
 */
#define WIFI_RESET_BUTTON_TASK_STACK_SIZE	2048    /**< Stack size in bytes (minimal, semaphore-based) */
#define WIFI_RESET_BUTTON_TASK_PRIORITY		6       /**< Task priority (FreeRTOS, highest on Core 0) */
#define WIFI_RESET_BUTTON_TASK_CORE_ID		0       /**< Pinned to Core 0 (protocol CPU) */
/** @} */

/**
 * @defgroup DHT22_TASK DHT22 Sensor Task Configuration
 * @brief Periodically reads temperature and humidity from DHT22 sensor.
 * @{
 */
#define DHT22_TASK_STACK_SIZE				4096    /**< Stack size in bytes */
#define DHT22_TASK_PRIORITY					5       /**< Task priority (FreeRTOS) */
#define DHT22_TASK_CORE_ID					1       /**< Pinned to Core 1 (application CPU) */
/** @} */

/**
 * @defgroup SNTP_TASK SNTP Time Sync Task Configuration
 * @brief Synchronizes system clock with NTP server over the internet.
 * @{
 */
#define SNTP_TIME_SYNC_TASK_STACK_SIZE		4096    /**< Stack size in bytes */
#define SNTP_TIME_SYNC_TASK_PRIORITY		4       /**< Task priority (FreeRTOS) */
#define SNTP_TIME_SYNC_TASK_CORE_ID			1       /**< Pinned to Core 1 (application CPU) */
/** @} */

/**
 * @defgroup AWS_IOT_TASK AWS IoT Task Configuration
 * @brief MQTT client publishing sensor data to AWS IoT Core.
 * @{
 */
#define AWS_IOT_TASK_STACK_SIZE				9216    /**< Stack size in bytes (large for TLS+MQTT) */
#define AWS_IOT_TASK_PRIORITY				6       /**< Task priority (FreeRTOS, highest on Core 1) */
#define AWS_IOT_TASK_CORE_ID				1       /**< Pinned to Core 1 (application CPU) */
/** @} */

#endif /* MAIN_TASKS_COMMON_H_ */
