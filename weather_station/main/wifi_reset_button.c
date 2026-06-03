/**
 * @file wifi_reset_button.c
 * @brief WiFi Reset Button - Implementation
 *
 * This file implements the WiFi reset button functionality using the ESP32's
 * BOOT button (GPIO 0). It provides a hardware mechanism for users to clear
 * saved WiFi credentials and disconnect from the current network without
 * needing the web interface.
 *
 * @details
 * Architecture (ISR + Task pattern):
 * 1. GPIO ISR fires on button press (negative edge on GPIO 0)
 * 2. ISR gives a binary semaphore (minimal ISR processing time)
 * 3. Button task blocks on semaphore, wakes on button press
 * 4. Task sends WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT to WiFi app
 * 5. Task delays 2 seconds for debounce before accepting next press
 *
 * This ISR+Task pattern is the recommended approach for handling GPIO
 * interrupts in FreeRTOS - keep ISR minimal, do real work in a task context.
 *
 * @note The IRAM_ATTR on the ISR handler ensures it resides in internal RAM
 *       for reliable execution during flash operations.
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "tasks_common.h"
#include "wifi_app.h"
#include "wifi_reset_button.h"

/** @brief Log tag for WiFi reset button ESP_LOG messages */
static const char TAG[] = "wifi_reset_button";

/** @brief Binary semaphore used for ISR-to-task notification (given from ISR, taken in task) */
SemaphoreHandle_t wifi_reset_semphore = NULL;

/**
 * ISR handler for the Wifi reset (BOOT) button
 * @param arg parameter which can be passed to the ISR handler.
 */
void IRAM_ATTR wifi_reset_button_isr_handler(void *arg)
{
	// Notify the button task
	xSemaphoreGiveFromISR(wifi_reset_semphore, NULL);
}

/**
 * Wifi reset button task reacts to a BOOT button event by sending a message
 * to the Wifi application to disconnect from Wifi and clear the saved credentials.
 * @param pvParam parameter which can be passed to the task.
 */
void wifi_reset_button_task(void *pvParam)
{
	for (;;)
	{
		if (xSemaphoreTake(wifi_reset_semphore, portMAX_DELAY) ==  pdTRUE)
		{
			ESP_LOGI(TAG, "WIFI RESET BUTTON INTERRUPT OCCURRED");

			// Send a message to disconnect Wifi and clear credentials
			wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT);

			vTaskDelay(2000 / portTICK_PERIOD_MS);
		}
	}
}

/**
 * @brief Configures the WiFi reset button hardware and starts the monitoring task.
 *
 * Sets up the complete button handling chain:
 * - Binary semaphore for ISR→task communication
 * - GPIO pin configuration (input mode, negative-edge interrupt)
 * - FreeRTOS task to process button events
 * - GPIO ISR service installation and handler attachment
 *
 * After this function returns, pressing the BOOT button will trigger a WiFi
 * disconnect and credential clear operation.
 */
void wifi_reset_button_config(void)
{
	// Create the binary semaphore for ISR-to-task signaling
	wifi_reset_semphore = xSemaphoreCreateBinary();

	// Configure the button GPIO pin as input
	esp_rom_gpio_pad_select_gpio(WIFI_RESET_BUTTON);
	gpio_set_direction(WIFI_RESET_BUTTON, GPIO_MODE_INPUT);

	// Enable interrupt on the negative edge (button press = HIGH→LOW transition)
	gpio_set_intr_type(WIFI_RESET_BUTTON, GPIO_INTR_NEGEDGE);

	// Create the WiFi reset button monitoring task
	xTaskCreatePinnedToCore(&wifi_reset_button_task, "wifi_reset_button", WIFI_RESET_BUTTON_TASK_STACK_SIZE, NULL, WIFI_RESET_BUTTON_TASK_PRIORITY, NULL, WIFI_RESET_BUTTON_TASK_CORE_ID);

	// Install the GPIO ISR service (shared across all GPIO interrupts)
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	// Attach the ISR handler to the specific button GPIO pin
	gpio_isr_handler_add(WIFI_RESET_BUTTON, wifi_reset_button_isr_handler, NULL);
}

