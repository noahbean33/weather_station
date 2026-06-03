/**
 * @file sntp_time_sync.c
 * @brief SNTP (Simple Network Time Protocol) Time Synchronization - Implementation
 *
 * This file implements the SNTP time synchronization functionality for the ESP32
 * weather station. It uses the lwIP SNTP client to synchronize the system clock
 * with an internet NTP server, providing accurate timestamps for sensor data
 * and the web interface.
 *
 * @details
 * Synchronization Process:
 * 1. Task starts and calls sntp_time_sync_obtain_time()
 * 2. If time is not set (year < 2016) or SNTP not initialized, init SNTP
 * 3. SNTP client contacts pool.ntp.org and updates system time
 * 4. HTTP server is notified that time service is available
 * 5. Task continues polling every 10 seconds to maintain accuracy
 *
 * Timezone Configuration:
 * Set to "CET-1CEST,M3.5.0,M10.5.0/3" which represents:
 * - CET (Central European Time, UTC+1) as standard time
 * - CEST (Central European Summer Time, UTC+2) for daylight saving
 * - DST starts: last Sunday of March at 02:00
 * - DST ends: last Sunday of October at 03:00
 *
 * @note Modify the timezone string in sntp_time_sync_obtain_time() to match your location.
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/apps/sntp.h"

#include "tasks_common.h"
#include "http_server.h"
#include "sntp_time_sync.h"
#include "wifi_app.h"

/** @brief Log tag for SNTP time sync ESP_LOG messages */
static const char TAG[] = "sntp_time_sync";

/** @brief Tracks whether SNTP operating mode has been configured (prevents re-setting mode) */
static bool sntp_op_mode_set = false;

/**
 * Initialize SNTP service using SNTP_OPMODE_POLL mode.
 */
static void sntp_time_sync_init_sntp(void)
{
	ESP_LOGI(TAG, "Initializing the SNTP service");

	if (!sntp_op_mode_set)
	{
		// Set the operating mode
		sntp_setoperatingmode(SNTP_OPMODE_POLL);
		sntp_op_mode_set = true;
	}

	sntp_setservername(0, "pool.ntp.org");

	// Initialize the servers
	sntp_init();

	// Let the http_server know service is initialized
	http_server_monitor_send_message(HTTP_MSG_TIME_SERVICE_INITIALIZED);
}

/**
 * Gets the current time and if the current time is not up to date,
 * the sntp_time_synch_init_sntp function is called.
 */
static void sntp_time_sync_obtain_time(void)
{
	time_t now = 0;
	struct tm time_info = {0};

	time(&now);
	localtime_r(&now, &time_info);

	// Check the time, in case we need to initialize/reinitialize
	if (sntp_op_mode_set == false || time_info.tm_year < (2016 - 1900))
	{
		sntp_time_sync_init_sntp();
		// Set the local time zone
		setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
		tzset();
	}
}

/**
 * The SNTP time synchronization task.
 * @param arg pvParam.
 */
static void sntp_time_sync(void *pvParam)
{
	while (1)
	{
		sntp_time_sync_obtain_time();
		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

/**
 * @brief Gets the current local time formatted as a human-readable string.
 *
 * Retrieves the system time, converts it to the local timezone, and formats
 * it as "DD.MM.YYYY HH:MM:SS". If time has not been synchronized yet
 * (year < 2016), logs a warning and returns the buffer unchanged.
 *
 * @return Pointer to a static buffer containing the formatted time string.
 * @warning Not thread-safe - uses a static buffer. Concurrent calls may overwrite results.
 */
char* sntp_time_sync_get_time(void)
{
	static char time_buffer[100] = {0};

	time_t now = 0;
	struct tm time_info = {0};

	time(&now);
	localtime_r(&now, &time_info);

	if (time_info.tm_year < (2016 - 1900))
	{
		ESP_LOGI(TAG, "Time is not set yet");
	}
	else
	{
		strftime(time_buffer, sizeof(time_buffer), "%d.%m.%Y %H:%M:%S", &time_info);
		ESP_LOGI(TAG, "Current time info: %s", time_buffer);
	}

	return time_buffer;
}

/**
 * @brief Creates and starts the SNTP time synchronization task.
 *
 * Launches the sntp_time_sync task pinned to the core defined in tasks_common.h.
 * The task will immediately begin attempting to synchronize time with pool.ntp.org.
 */
void sntp_time_sync_task_start(void)
{
	xTaskCreatePinnedToCore(&sntp_time_sync, "sntp_time_sync", SNTP_TIME_SYNC_TASK_STACK_SIZE, NULL, SNTP_TIME_SYNC_TASK_PRIORITY, NULL, SNTP_TIME_SYNC_TASK_CORE_ID);
}


















