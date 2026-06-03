/**
 * @file wifi_reset_button.h
 * @brief WiFi Reset Button Interface - Header File
 *
 * This module provides a hardware button interface for resetting the WiFi
 * configuration on the ESP32. When the BOOT button (GPIO 0) is pressed,
 * it triggers a WiFi disconnection and clears any saved credentials from NVS.
 *
 * @details
 * Implementation:
 * - Uses GPIO interrupt (negative edge) to detect button press
 * - ISR gives a binary semaphore to wake the button task
 * - Button task sends disconnect message to the WiFi application
 * - 2-second debounce delay prevents multiple triggers
 *
 * Hardware:
 * - Uses the BOOT button on ESP32 DevKit boards (GPIO 0)
 * - BOOT button is active-low (pulled HIGH normally, LOW when pressed)
 * - Negative edge interrupt triggers on button press (HIGH → LOW transition)
 *
 * @note The BOOT button is shared with the serial bootloader. Only press it
 *       during normal operation (not during reset) for WiFi reset functionality.
 */

#ifndef MAIN_WIFI_RESET_BUTTON_H_
#define MAIN_WIFI_RESET_BUTTON_H_

/** @brief Default interrupt allocation flags (no special flags needed) */
#define ESP_INTR_FLAG_DEFAULT	0

/**
 * @brief GPIO pin number for the WiFi reset button.
 * @note GPIO 0 is the BOOT button on most ESP32 DevKit boards.
 *       It has an external pull-up resistor and is active-low.
 */
#define WIFI_RESET_BUTTON		0

/**
 * @brief Configures the WiFi reset button GPIO, interrupt, and monitoring task.
 *
 * Performs the following setup:
 * 1. Creates a binary semaphore for ISR-to-task communication
 * 2. Configures GPIO 0 as input
 * 3. Sets negative-edge interrupt trigger
 * 4. Creates the button monitoring FreeRTOS task
 * 5. Installs the GPIO ISR service
 * 6. Attaches the ISR handler to the button GPIO
 *
 * @note Must be called after FreeRTOS scheduler is running.
 */
void wifi_reset_button_config(void);

#endif /* MAIN_WIFI_RESET_BUTTON_H_ */
