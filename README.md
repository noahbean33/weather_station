 # ESP32 Weather Station

A feature-rich IoT weather station built on the ESP32 microcontroller using ESP-IDF (Espressif IoT Development Framework) and FreeRTOS. This project monitors temperature and humidity via a DHT22 sensor, provides a web interface for configuration and monitoring, and publishes sensor data to AWS IoT Core via MQTT.

## Features

### Core Functionality
- **DHT22 Temperature & Humidity Monitoring** - Reads environmental data from DHT22 sensor
- **WiFi Connectivity** - Dual-mode operation (Access Point + Station mode)
- **Web-Based Configuration** - HTTP server with responsive web interface for device setup
- **AWS IoT Integration** - Publishes sensor data to AWS IoT Core using MQTT protocol
- **OTA Firmware Updates** - Upload new firmware over-the-air via web interface
- **RGB LED Status Indicators** - Visual feedback for connection status and states
- **WiFi Reset Button** - Hardware button with interrupt-based WiFi credential reset

### Advanced Features
- **Non-Volatile Storage (NVS)** - Persistent storage of WiFi credentials
- **SNTP Time Synchronization** - Automatic network time sync for timestamped data
- **Dual-Core Utilization** - Optimized task distribution across both ESP32 cores
- **FreeRTOS Task Management** - Multi-threaded architecture with message queues and semaphores
- **State Machine Design** - Robust WiFi connection management with retry logic

## Hardware Requirements

- **ESP32 Development Board** (any ESP32 DevKit)
- **DHT22 Temperature/Humidity Sensor**
- **RGB LED** (common cathode or anode with appropriate resistors)
- **Push Button** (for WiFi reset functionality)
- **Resistors** (220Ω-330Ω for LED current limiting)
- **Breadboard and Jumper Wires**

### Default GPIO Pin Configuration

```
DHT22 Sensor:     GPIO 25
RGB LED Red:      GPIO 21
RGB LED Green:    GPIO 22
RGB LED Blue:     GPIO 23
Reset Button:     (Configure in wifi_reset_button.c)
```

## Software Requirements

- **ESP-IDF v4.4 or later** (Espressif IoT Development Framework)
- **CMake** (version 3.5 or later)
- **Python 3.x** (for ESP-IDF build tools)
- **AWS Account** (for AWS IoT Core connectivity)
- **Web Browser** (Chrome/Firefox recommended for web interface)

## Installation & Setup

### Configure AWS IoT Credentials

Create a `main/certs/` directory and add your AWS IoT certificates:

```bash
mkdir -p main/certs
```

Place the following files in `main/certs/`:
- `aws_root_ca_pem` - Amazon Root CA certificate
- `certificate_pem_crt` - Device certificate
- `private_pem_key` - Private key

**Note:** These certificate files are gitignored for security.

### Configure Project Settings

Edit `main/aws_iot.h` to set your AWS IoT client ID:

```c
#define CONFIG_AWS_EXAMPLE_CLIENT_ID "Your_ESP32_Client_ID"
```

Optionally modify WiFi AP settings in `main/wifi_app.h`:

```c
#define WIFI_AP_SSID         "ESP32_AP"
#define WIFI_AP_PASSWORD     "password"
```

### 5. Build and Flash

```bash
# Set ESP-IDF environment variables
. $HOME/esp/esp-idf/export.sh

# Build the project
idf.py build

# Flash to ESP32 (replace PORT with your serial port)
idf.py -p PORT flash monitor
```

## Usage

### Initial Setup

1. **Power on the ESP32** - The RGB LED will indicate status
2. **Connect to ESP32 Access Point** - Look for WiFi network "ESP32_AP" (password: "password")
3. **Open Web Interface** - Navigate to `http://192.168.0.1` in your browser
4. **Configure WiFi** - Enter your home/office WiFi credentials through the web interface
5. **Connect** - ESP32 will connect to your WiFi network and begin operation

### Web Interface Features

- **WiFi Configuration** - Connect ESP32 to your wireless network
- **Connection Status** - View current WiFi connection details and signal strength (RSSI)
- **Sensor Data** - Real-time temperature and humidity readings
- **Time Display** - Synchronized network time
- **OTA Updates** - Upload new firmware binaries for wireless updates
- **WiFi Disconnect** - Manually disconnect from current network

### AWS IoT Integration

Once connected to WiFi, the device will:
- Connect to AWS IoT Core using MQTT over TLS
- Publish sensor data (temperature, humidity, RSSI) to configured topics
- Subscribe to command topics for remote control

## Project Structure

```
weather_station/
├── main/
│   ├── main.c                  # Application entry point
│   ├── wifi_app.c/h            # WiFi application and state machine
│   ├── http_server.c/h         # HTTP server implementation
│   ├── aws_iot.c/h             # AWS IoT MQTT client
│   ├── DHT22.c/h               # DHT22 sensor driver
│   ├── rgb_led.c/h             # RGB LED control
│   ├── app_nvs.c/h             # Non-volatile storage management
│   ├── sntp_time_sync.c/h      # SNTP time synchronization
│   ├── wifi_reset_button.c/h   # Reset button with ISR
│   ├── tasks_common.h          # FreeRTOS task configuration
│   ├── webpage/                # Web interface files
│   │   ├── index.html
│   │   ├── app.js
│   │   ├── app.css
│   │   └── jquery-3.3.1.min.js
│   └── certs/                  # AWS IoT certificates (gitignored)
├── CMakeLists.txt              # Root CMake configuration
├── partitions.csv              # Custom partition table for OTA
└── README.md
```

## Architecture

### FreeRTOS Task Distribution

**Core 0:**
- WiFi Application Task
- HTTP Server Task
- HTTP Server Monitor Task
- WiFi Reset Button Task

**Core 1:**
- DHT22 Sensor Task
- SNTP Time Sync Task
- AWS IoT Task

### Communication

- **Message Queues** - Inter-task communication for WiFi events and HTTP server messages
- **Event Groups** - WiFi connection synchronization
- **Binary Semaphores** - ISR signaling from hardware button

## Troubleshooting

### WiFi Connection Issues
- Verify SSID and password are correct
- Check WiFi signal strength
- Press and hold the reset button to clear stored credentials

### AWS IoT Connection Failures
- Verify certificates are correctly placed in `main/certs/`
- Check AWS IoT thing name and endpoint configuration
- Ensure device certificate is activated in AWS IoT console

### OTA Update Failures
- Ensure firmware binary is built for OTA partitions
- Check that new firmware size fits within partition size (1984KB)