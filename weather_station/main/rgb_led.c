/**
 * @file rgb_led.c
 * @brief RGB LED Status Indicator - Implementation
 *
 * This file implements PWM-based RGB LED control using the ESP32's LEDC peripheral.
 * The LED provides visual feedback about the weather station's operational state
 * through different color combinations.
 *
 * @details
 * Implementation Notes:
 * - Uses LEDC high-speed mode with Timer 0 for all three channels
 * - PWM frequency: 100 Hz (sufficient for LED dimming, no visible flicker)
 * - Duty resolution: 8-bit (values 0-255 map to 0-100% duty cycle)
 * - Lazy initialization: PWM hardware is configured on first use
 * - Each public function checks initialization state before setting color
 *
 * Channel Mapping:
 * - Channel 0: Red   (GPIO 21)
 * - Channel 1: Green (GPIO 22)
 * - Channel 2: Blue  (GPIO 23)
 */

#include <stdbool.h>

#include "driver/ledc.h"
#include "rgb_led.h"

/** @brief LEDC channel configuration array for the three RGB color channels */
ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM];

/** @brief Flag tracking whether PWM hardware has been initialized (lazy init pattern) */
bool g_pwm_init_handle = false;

/**
 * Initializes the RGB LED settings per channel, including
 * the GPIO for each color, mode and timer configuration.
 */
static void rgb_led_pwm_init(void)
{
	int rgb_ch;

	// Red
	ledc_ch[0].channel		= LEDC_CHANNEL_0;
	ledc_ch[0].gpio			= RGB_LED_RED_GPIO;
	ledc_ch[0].mode			= LEDC_HIGH_SPEED_MODE;
	ledc_ch[0].timer_index	= LEDC_TIMER_0;

	// Green
	ledc_ch[1].channel		= LEDC_CHANNEL_1;
	ledc_ch[1].gpio			= RGB_LED_GREEN_GPIO;
	ledc_ch[1].mode			= LEDC_HIGH_SPEED_MODE;
	ledc_ch[1].timer_index	= LEDC_TIMER_0;

	// Blue
	ledc_ch[2].channel		= LEDC_CHANNEL_2;
	ledc_ch[2].gpio			= RGB_LED_BLUE_GPIO;
	ledc_ch[2].mode			= LEDC_HIGH_SPEED_MODE;
	ledc_ch[2].timer_index	= LEDC_TIMER_0;

	// Configure timer zero
	ledc_timer_config_t ledc_timer =
	{
		.duty_resolution	= LEDC_TIMER_8_BIT,
		.freq_hz			= 100,
		.speed_mode			= LEDC_HIGH_SPEED_MODE,
		.timer_num			= LEDC_TIMER_0
	};
	ledc_timer_config(&ledc_timer);

	// Configure channels
	for (rgb_ch = 0; rgb_ch < RGB_LED_CHANNEL_NUM; rgb_ch++)
	{
		ledc_channel_config_t ledc_channel =
		{
			.channel	= ledc_ch[rgb_ch].channel,
			.duty		= 0,
			.hpoint		= 0,
			.gpio_num	= ledc_ch[rgb_ch].gpio,
			.intr_type	= LEDC_INTR_DISABLE,
			.speed_mode = ledc_ch[rgb_ch].mode,
			.timer_sel	= ledc_ch[rgb_ch].timer_index,
		};
		ledc_channel_config(&ledc_channel);
	}

	g_pwm_init_handle = true;
}

/**
 * Sets the RGB color.
 */
static void rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
	// Value should be 0 - 255 for 8 bit number
	ledc_set_duty(ledc_ch[0].mode, ledc_ch[0].channel, red);
	ledc_update_duty(ledc_ch[0].mode, ledc_ch[0].channel);

	ledc_set_duty(ledc_ch[1].mode, ledc_ch[1].channel, green);
	ledc_update_duty(ledc_ch[1].mode, ledc_ch[1].channel);

	ledc_set_duty(ledc_ch[2].mode, ledc_ch[2].channel, blue);
	ledc_update_duty(ledc_ch[2].mode, ledc_ch[2].channel);
}

/**
 * @brief Sets LED to purple - indicates WiFi application has started.
 * Initializes PWM on first call. Color: R=255, G=102, B=255 (purple/magenta).
 */
void rgb_led_wifi_app_started(void)
{
	if (g_pwm_init_handle == false)
	{
		rgb_led_pwm_init();
	}

	rgb_led_set_color(255, 102, 255);
}

/**
 * @brief Sets LED to yellow-green - indicates HTTP server is running.
 * Initializes PWM on first call. Color: R=204, G=255, B=51 (yellow-green).
 */
void rgb_led_http_server_started(void)
{
	if (g_pwm_init_handle == false)
	{
		rgb_led_pwm_init();
	}

	rgb_led_set_color(204, 255, 51);
}

/**
 * @brief Sets LED to cyan-green - indicates successful WiFi STA connection.
 * Initializes PWM on first call. Color: R=0, G=255, B=153 (cyan-green).
 */
void rgb_led_wifi_connected(void)
{
	if (g_pwm_init_handle == false)
	{
		rgb_led_pwm_init();
	}

	rgb_led_set_color(0, 255, 153);
}














































