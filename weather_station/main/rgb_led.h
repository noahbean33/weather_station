/**
 * @file rgb_led.h
 * @brief RGB LED Status Indicator Interface - Header File
 *
 * This module controls an RGB LED used as a visual status indicator for the
 * ESP32 weather station. Different colors represent different application states,
 * providing at-a-glance feedback about the device's operational status.
 *
 * @details
 * LED Color Meanings:
 * - Purple (255, 102, 255): WiFi application has started, awaiting connection
 * - Yellow-Green (204, 255, 51): HTTP server is running, ready for configuration
 * - Cyan-Green (0, 255, 153): Successfully connected to WiFi access point
 *
 * Hardware Configuration:
 * - Uses ESP32 LEDC (LED Control) peripheral for PWM-based color mixing
 * - 8-bit duty resolution (0-255 per channel) at 100 Hz PWM frequency
 * - Three PWM channels (one per color) on LEDC Timer 0, High-Speed Mode
 *
 * @note The RGB LED is active-high (higher duty = brighter).
 *       PWM initialization is lazy - occurs on first color set call.
 */

#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

/**
 * @defgroup RGB_LED_GPIO RGB LED GPIO Pin Assignments
 * @brief GPIO pins connected to the Red, Green, and Blue LED anodes.
 * @{
 */
#define RGB_LED_RED_GPIO		21  /**< GPIO pin for the Red LED channel */
#define RGB_LED_GREEN_GPIO		22  /**< GPIO pin for the Green LED channel */
#define RGB_LED_BLUE_GPIO		23  /**< GPIO pin for the Blue LED channel */
/** @} */

/** @brief Total number of RGB color channels (Red, Green, Blue) */
#define RGB_LED_CHANNEL_NUM		3

/**
 * @brief LEDC channel configuration structure for one RGB LED color channel.
 *
 * Stores the LEDC peripheral configuration for a single color channel,
 * including the PWM channel number, GPIO pin, speed mode, and timer index.
 */
typedef struct
{
	int channel;        /**< LEDC channel number (LEDC_CHANNEL_0, 1, or 2) */
	int gpio;           /**< GPIO pin number for this color channel */
	int mode;           /**< LEDC speed mode (LEDC_HIGH_SPEED_MODE or LEDC_LOW_SPEED_MODE) */
	int timer_index;    /**< LEDC timer index used for this channel */
} ledc_info_t;

/**
 * @brief Sets the RGB LED to purple to indicate WiFi application has started.
 *
 * Color: Purple (R=255, G=102, B=255)
 * Meaning: Device is powered on and WiFi subsystem is initializing.
 * Initializes PWM hardware on first call if not already done.
 */
void rgb_led_wifi_app_started(void);

/**
 * @brief Sets the RGB LED to yellow-green to indicate HTTP server is active.
 *
 * Color: Yellow-Green (R=204, G=255, B=51)
 * Meaning: Web configuration interface is available for WiFi setup.
 * Initializes PWM hardware on first call if not already done.
 */
void rgb_led_http_server_started(void);

/**
 * @brief Sets the RGB LED to cyan-green to indicate successful WiFi connection.
 *
 * Color: Cyan-Green (R=0, G=255, B=153)
 * Meaning: ESP32 is connected to an external access point with valid IP.
 * Initializes PWM hardware on first call if not already done.
 */
void rgb_led_wifi_connected(void);

#endif /* MAIN_RGB_LED_H_ */
