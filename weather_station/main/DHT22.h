/**
 * @file DHT22.h
 * @brief DHT22 (AM2302) Temperature and Humidity Sensor Driver - Header File
 *
 * This header provides the public interface for the DHT22 digital temperature
 * and humidity sensor driver. The DHT22 uses a proprietary single-wire serial
 * communication protocol to transmit 40 bits of data (16-bit humidity,
 * 16-bit temperature, 8-bit checksum).
 *
 * @details
 * - Measurement Range: Humidity 0-100% RH, Temperature -40 to 80°C
 * - Accuracy: Humidity ±2% RH, Temperature ±0.5°C
 * - Minimum sampling interval: 2 seconds
 * - Operating Voltage: 3.3V - 5.5V
 *
 * The driver runs as a FreeRTOS task that periodically reads the sensor
 * and stores the latest temperature and humidity values in global variables
 * accessible via getter functions.
 *
 * @note The DHT22 requires precise timing for the single-wire protocol.
 *       The GPIO pin used for communication must support both input and output modes.
 */

#ifndef DHT22_H_  
#define DHT22_H_

/**
 * @defgroup DHT22_Status DHT22 Status/Error Codes
 * @brief Return codes from the readDHT() function indicating success or failure type.
 * @{
 */
#define DHT_OK 0                /**< Successful read - data is valid */
#define DHT_CHECKSUM_ERROR -1   /**< Data received but checksum verification failed */
#define DHT_TIMEOUT_ERROR -2    /**< Sensor did not respond within expected timeframe */
/** @} */

/**
 * @brief GPIO pin number connected to the DHT22 sensor data line.
 * @note This pin must be capable of both input and output (open-drain recommended).
 *       Ensure a 4.7k-10k pull-up resistor is connected between this pin and VCC.
 */
#define DHT_GPIO			25

/**
 * @brief Starts the DHT22 sensor reading task.
 *
 * Creates a FreeRTOS task pinned to a specific core that continuously reads
 * the DHT22 sensor at a regular interval (every 4 seconds). The task updates
 * internal global variables for temperature and humidity that can be retrieved
 * using getTemperature() and getHumidity().
 *
 * @note Must be called after FreeRTOS scheduler is running.
 *       Task parameters (stack size, priority, core) are defined in tasks_common.h.
 */
void DHT22_task_start(void);

/**
 * @brief Sets the GPIO pin used for DHT22 communication.
 * @param gpio GPIO pin number to use for the DHT22 data line.
 */
void 	setDHTgpio(int gpio);

/**
 * @brief Handles and logs errors returned by readDHT().
 * @param response Error/status code returned from readDHT() (DHT_OK, DHT_CHECKSUM_ERROR, or DHT_TIMEOUT_ERROR).
 */
void 	errorHandler(int response);

/**
 * @brief Reads 40 bits of data from the DHT22 sensor.
 *
 * Performs the complete single-wire communication protocol:
 * 1. Sends start signal (pull low for 3ms, then high for 25us)
 * 2. Waits for sensor acknowledgment (80us low + 80us high)
 * 3. Reads 40 data bits (16-bit humidity + 16-bit temperature + 8-bit checksum)
 * 4. Validates checksum and updates global temperature/humidity variables
 *
 * @return DHT_OK on success, DHT_TIMEOUT_ERROR if sensor didn't respond,
 *         DHT_CHECKSUM_ERROR if data integrity check failed.
 */
int 	readDHT();

/**
 * @brief Gets the last successfully read humidity value.
 * @return Relative humidity in percent (0.0 - 100.0% RH).
 */
float 	getHumidity();

/**
 * @brief Gets the last successfully read temperature value.
 * @return Temperature in degrees Celsius (-40.0 to 80.0°C).
 */
float 	getTemperature();

/**
 * @brief Waits for the GPIO signal to change from the specified state.
 *
 * Polls the DHT GPIO pin in a tight loop, counting microseconds until the
 * signal level changes from the specified state or until timeout is reached.
 *
 * @param usTimeOut Maximum number of microseconds to wait before returning timeout.
 * @param state The current GPIO state to wait for change from (true = high, false = low).
 * @return Number of microseconds elapsed before state change, or -1 if timeout occurred.
 */
int 	getSignalLevel( int usTimeOut, bool state );

#endif
