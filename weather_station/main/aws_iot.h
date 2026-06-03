/**
 * @file aws_iot.h
 * @brief AWS IoT Core MQTT Client Interface - Header File
 *
 * This module provides the interface for connecting the ESP32 to AWS IoT Core
 * via MQTT protocol. Once connected, the device publishes sensor data (temperature,
 * humidity, WiFi RSSI) to an MQTT topic and subscribes to receive commands.
 *
 * @details
 * The AWS IoT connection uses mutual TLS authentication with:
 * - CA root certificate (aws-root-ca.pem)
 * - Device certificate (certificate.pem.crt)
 * - Device private key (private.pem.key)
 *
 * These certificates must be placed in the project's "certs/" directory and are
 * embedded into the firmware binary at compile time.
 *
 * MQTT Configuration:
 * - Endpoint/Host: Defined in aws_iot_config.h (AWS_IOT_MQTT_HOST)
 * - Port: Defined in aws_iot_config.h (AWS_IOT_MQTT_PORT, typically 8883 for TLS)
 * - Client ID: CONFIG_AWS_EXAMPLE_CLIENT_ID (the "Thing" name in AWS IoT)
 * - Topic: "test_topic/esp32"
 *
 * @note The AWS IoT task should only be started after a successful WiFi connection
 *       has been established (i.e., after obtaining an IP address).
 */

#ifndef MAIN_AWS_IOT_H_
#define MAIN_AWS_IOT_H_

/**
 * @brief AWS IoT Client ID - corresponds to the "Thing" name registered in AWS IoT Core.
 * @note This must match the Thing name configured in your AWS IoT console.
 */
#define CONFIG_AWS_EXAMPLE_CLIENT_ID "Udemy_ESP32_Test"

/**
 * @brief Starts the AWS IoT MQTT client task.
 *
 * Creates a FreeRTOS task that initializes the MQTT client, connects to AWS IoT Core
 * using TLS mutual authentication, subscribes to the configured topic, and enters
 * a loop that periodically publishes sensor data (WiFi RSSI, temperature, humidity).
 *
 * The task supports auto-reconnect with exponential backoff if the connection is lost.
 *
 * @note This function is safe to call multiple times - it will only create the task
 *       once (subsequent calls are ignored if the task already exists).
 * @note Must be called only after WiFi connection is established and IP is obtained.
 */
void aws_iot_start(void);

#endif /* MAIN_AWS_IOT_H_ */
