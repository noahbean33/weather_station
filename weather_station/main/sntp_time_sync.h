/**
 * @file sntp_time_sync.h
 * @brief SNTP (Simple Network Time Protocol) Time Synchronization - Header File
 *
 * This module provides network time synchronization using SNTP protocol to obtain
 * accurate time from internet NTP servers. The synchronized time is used for
 * timestamping sensor data and displaying the current time on the web interface.
 *
 * @details
 * - NTP Server: pool.ntp.org (global pool of NTP servers)
 * - Operating Mode: SNTP_OPMODE_POLL (periodically polls the server)
 * - Timezone: CET-1CEST,M3.5.0,M10.5.0/3 (Central European Time with DST)
 * - Sync Interval: Every 10 seconds (task loop delay)
 *
 * The module runs as a FreeRTOS task that periodically checks if time is valid
 * and re-initializes the SNTP service if needed. Once synchronized, the local
 * time is available via sntp_time_sync_get_time().
 *
 * @note This module requires an active internet connection (WiFi STA mode connected).
 *       It should only be started after obtaining an IP address.
 */

#ifndef MAIN_SNTP_TIME_SYNC_H_
#define MAIN_SNTP_TIME_SYNC_H_

/**
 * @brief Starts the SNTP time synchronization FreeRTOS task.
 *
 * Creates a task pinned to a specific core that periodically synchronizes
 * the ESP32's system clock with an NTP server. Also notifies the HTTP server
 * that time service has been initialized.
 *
 * @note Should only be called after WiFi STA connection is established.
 *       Task parameters are defined in tasks_common.h.
 */
void sntp_time_sync_task_start(void);

/**
 * @brief Returns the current local time as a formatted string.
 *
 * Reads the system time, converts it to local time using the configured timezone,
 * and formats it as "DD.MM.YYYY HH:MM:SS".
 *
 * @return Pointer to a static buffer containing the formatted time string.
 *         Returns an empty buffer if time has not been synchronized yet.
 * @note The returned pointer is to a static buffer - not thread-safe for concurrent access.
 */
char* sntp_time_sync_get_time(void);

#endif /* MAIN_SNTP_TIME_SYNC_H_ */
