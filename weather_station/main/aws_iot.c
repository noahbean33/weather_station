/**
 * @file aws_iot.c
 * @brief AWS IoT Core MQTT Client - Implementation
 *
 * This file implements the AWS IoT Core MQTT client for the ESP32 weather station.
 * It establishes a secure TLS connection to AWS IoT Core, subscribes to a topic
 * for receiving commands, and periodically publishes sensor telemetry data.
 *
 * @details
 * Functionality:
 * - Initializes the AWS IoT MQTT client with TLS mutual authentication
 * - Connects to AWS IoT Core endpoint with configurable retry logic
 * - Subscribes to "test_topic/esp32" for incoming messages
 * - Publishes WiFi RSSI (QoS 0) and temperature/humidity data (QoS 1) every 3 seconds
 * - Supports automatic reconnection with exponential backoff on disconnect
 *
 * Security:
 * - Uses embedded certificates (compiled into the binary from the certs/ directory)
 * - CA root cert, device cert, and private key are required for mutual TLS
 * - SSL hostname verification is enabled
 *
 * Data Published:
 * - QoS 0: "WiFi RSSI : <value>" (signal strength of connected access point)
 * - QoS 1: "Temperature : <value>, Humidity : <value>" (DHT22 sensor readings)
 *
 * @note Based on AWS IoT SDK sample code.
 *
 * @copyright
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0
 */

/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the build configuration and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "test_topic/esp32"
 *
 * Some setup is required. See example README for details.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"

#include "aws_iot.h"
#include "DHT22.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "tasks_common.h"
#include "wifi_app.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

static const char *TAG = "aws_iot";  /**< Log tag for AWS IoT ESP_LOG messages */

/** @brief Task handle for the AWS IoT MQTT client task (NULL if not started) */
static TaskHandle_t task_aws_iot = NULL;

/**
 * CA Root certificate, device ("Thing") certificate and device ("Thing") key.
 * "Embedded Certs" are loaded from files in "certs/" and embedded into the app binary.
 */
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

/**
 * @brief Callback function invoked when a message is received on a subscribed MQTT topic.
 *
 * Logs the received topic name and message payload to the serial console.
 * This handler is registered during the MQTT subscription setup.
 *
 * @param pClient Pointer to the AWS IoT MQTT client instance.
 * @param topicName Name of the topic the message was received on.
 * @param topicNameLen Length of the topic name string.
 * @param params Message parameters including payload data and length.
 * @param pData User-defined callback data (unused).
 */
void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    ESP_LOGI(TAG, "Subscribe callback Test: %.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
}

/**
 * @brief Callback function invoked when the MQTT connection is unexpectedly lost.
 *
 * Handles disconnection events by either relying on the auto-reconnect mechanism
 * (if enabled) or attempting a manual reconnection. This callback is registered
 * during MQTT client initialization.
 *
 * @param pClient Pointer to the AWS IoT MQTT client that was disconnected.
 * @param data User-defined data passed during callback registration (unused).
 */
void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
    ESP_LOGW(TAG, "MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if(NULL == pClient) {
        return;
    }

    if(aws_iot_is_autoreconnect_enabled(pClient)) {
        ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
    } else {
        ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if(NETWORK_RECONNECTED == rc) {
            ESP_LOGW(TAG, "Manual Reconnect Successful");
        } else {
            ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
        }
    }
}

/**
 * @brief Main AWS IoT MQTT client task.
 *
 * This FreeRTOS task performs the following sequence:
 * 1. Initializes the MQTT client with TLS parameters and embedded certificates
 * 2. Connects to AWS IoT Core endpoint with retry logic
 * 3. Enables auto-reconnect with exponential backoff
 * 4. Subscribes to "test_topic/esp32" topic
 * 5. Enters main loop: yields for incoming messages, publishes sensor data every 3s
 *    - Publishes WiFi RSSI at QoS 0 (fire-and-forget)
 *    - Publishes temperature and humidity at QoS 1 (acknowledged delivery)
 *
 * @param param Unused task parameter (NULL).
 * @note Calls abort() on critical failures (init, auto-reconnect setup, subscription).
 */
void aws_iot_task(void *param) {
    char cPayload[100];

    int32_t i = 0;

    IoT_Error_t rc = FAILURE;

    AWS_IoT_Client client;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    IoT_Publish_Message_Params paramsQOS0;
    IoT_Publish_Message_Params paramsQOS1;

    ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = HostAddress;
    mqttInitParams.port = port;

    mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
    mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
    mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;

    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
        abort();
    }

    connectParams.keepAliveIntervalInSec = 10;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    /* Client ID is set in aws_iot.h and AKA your Thing's Name in AWS IoT */
    connectParams.pClientID = CONFIG_AWS_EXAMPLE_CLIENT_ID;
    connectParams.clientIDLen = (uint16_t) strlen(CONFIG_AWS_EXAMPLE_CLIENT_ID);
    connectParams.isWillMsgPresent = false;

    ESP_LOGI(TAG, "Connecting to AWS...");
    do {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if(SUCCESS != rc) {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    } while(SUCCESS != rc);

    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
        abort();
    }

    const char *TOPIC = "test_topic/esp32";
    const int TOPIC_LEN = strlen(TOPIC);

    ESP_LOGI(TAG, "Subscribing...");
    rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS0, iot_subscribe_callback_handler, NULL);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
        abort();
    }

    sprintf(cPayload, "%s : %ld ", "hello from SDK", i);

    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void *) cPayload;
    paramsQOS0.isRetained = 0;

    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *) cPayload;
    paramsQOS1.isRetained = 0;

    while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)) {

        //Max time the yield function will wait for read messages
        rc = aws_iot_mqtt_yield(&client, 100);
        if(NETWORK_ATTEMPTING_RECONNECT == rc) {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
        }

        ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        sprintf(cPayload, "%s : %d ", "WiFi RSSI", wifi_app_get_rssi());
        paramsQOS0.payloadLen = strlen(cPayload);
        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS0);

        sprintf(cPayload, "%s : %.1f, %s : %.1f", "Temperature", getTemperature(), "Humidity", getHumidity());
        paramsQOS1.payloadLen = strlen(cPayload);
        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS1);
        if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
            ESP_LOGW(TAG, "QOS1 publish ack not received.");
            rc = SUCCESS;
        }
    }

    ESP_LOGE(TAG, "An error occurred in the main loop.");
    abort();
}

/**
 * @brief Creates and starts the AWS IoT MQTT client task.
 *
 * Launches the aws_iot_task pinned to the core specified in tasks_common.h.
 * Only creates the task if it hasn't been created already (checked via task handle).
 * This prevents multiple instances of the AWS IoT task from running.
 */
void aws_iot_start(void)
{
	if (task_aws_iot == NULL)
	{
		xTaskCreatePinnedToCore(&aws_iot_task, "aws_iot_task", AWS_IOT_TASK_STACK_SIZE, NULL, AWS_IOT_TASK_PRIORITY, &task_aws_iot, AWS_IOT_TASK_CORE_ID);
	}
}




