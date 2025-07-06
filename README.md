FPGA-Accelerated Digital Radio Receiver
A proof-of-concept digital radio that uses a Xilinx Artix-7 FPGA for real-time DSP and an ESP32 microcontroller for system control. The project demonstrates core competencies in FPGA design, embedded C, and RF system integration using off-the-shelf hardware modules.

This project is intentionally scoped for rapid development and focuses on creating a demonstrable system that showcases key engineering skills for a resume or portfolio.

Core Features
FPGA DSP: Real-time mono FM demodulation implemented in SystemVerilog.

MCU Control: An ESP32 running C (ESP-IDF) controls the radio tuner and user interface.

User Interface: A rotary encoder for tuning and an OLED screen to display the current frequency.

System Integration: All components are integrated on a breadboard or protoboard for straightforward assembly and debugging.

🛠️ Hardware Components
FPGA Board: Digilent Nexys A7 (or similar Artix-7 board).

Microcontroller: ESP32 Development Board (e.g., ESP32-DevKitC).

RF Tuner: Si4703 Breakout Board. This module simplifies the RF stage by handling tuning and providing a standard I2S digital audio output.

Display: I2C OLED Display (e.g., SSD1306 128x64).

Audio Output: I2S Amplifier Breakout Board (e.g., MAX98357A). This accepts the digital audio stream directly from the FPGA.

Input Control: One rotary encoder for tuning.

Power Supply: USB power or a benchtop power supply.

🏗️ System Architecture
Code snippet

graph TD
    A[Antenna] --> RF_TUNER;
    RF_TUNER[Si4703 Tuner Breakout];
    UI_Knob[Tuning Knob] --> ESP32;
    ESP32[ESP32 Dev Board (C Firmware)];
    ESP32 -- I2C --> RF_TUNER;
    ESP32 -- I2C --> OLED[OLED Display];

    subgraph "FPGA DSP (Artix-7)"
        ARTIX7[Xilinx Artix-7 FPGA]
        I2S_IN[I2S Input]
        FM_DEMOD[Mono FM Demodulator]
        I2S_OUT[I2S Output]

        I2S_IN --> FM_DEMOD;
        FM_DEMOD --> I2S_OUT;
    end

    RF_TUNER -- Digital Audio (I2S) --> I2S_IN;
    I2S_OUT --> DAC_AMP[I2S DAC/Amplifier];
    DAC_AMP --> Speaker;
System Flow:

The ESP32 instructs the Si4703 Tuner via I2C to tune to a specific frequency.

The tuner receives the radio signal and outputs a digital I2S audio stream.

The Artix-7 FPGA receives the I2S stream, performs mono FM demodulation in real-time, and outputs a processed I2S stream.

An I2S Amplifier receives the clean digital audio from the FPGA, converts it to analog, and drives a speaker.

The ESP32 displays the current frequency on the OLED screen.

💻 Software & Tools
FPGA Development:

Language: SystemVerilog

Toolchain: Xilinx Vivado Design Suite

Simulation: Vivado Simulator (XSIM)

Embedded Firmware (ESP32):

Language: C / C++

Framework: ESP-IDF (Espressif IoT Development Framework)

Tools: idf.py command-line tool

🚀 Project Plan
Phase 1: Core Controller Setup

[ ] Set up the ESP-IDF environment.

[ ] Interface the ESP32 with the OLED display and rotary encoder.

[ ] Write firmware to control the Si4703 tuner via I2C and display the frequency.

Milestone: Tune to an FM station and listen to the raw (undemodulated) output if possible, proving tuner control.

Phase 2: FPGA DSP Implementation

[ ] Set up the Xilinx Vivado environment.

[ ] Design and simulate a mono FM demodulator in SystemVerilog.

[ ] Implement I2S input and output interfaces on the FPGA.

[ ] Synthesize the design and program the Nexys A7 board.

Milestone: Pass a test I2S signal through the FPGA without modification (pass-through test).

Phase 3: System Integration & Testing

[ ] Connect all hardware modules on a breadboard.

[ ] Route the I2S signal from the tuner, through the FPGA, to the I2S amplifier.

[ ] Debug the complete system to achieve clear audio output.

Milestone: A fully functional radio that tunes via the knob and plays audio processed by the FPGA.

📁 Repository Structure
.
├── firmware/esp32             # ESP32 ESP-IDF C/C++ firmware
│   ├── main/
│   │   └── main.c
│   ├── components/
│   └── CMakeLists.txt
├── fpga/artix7                # Xilinx Artix-7 SystemVerilog code
│   ├── src/                   # Source files (.sv)
│   ├── sim/                   # Simulation files and testbenches
│   └── constraints/           # Pin and timing constraints (.xdc)
├── docs/                      # Documentation, diagrams, images
└── README.md
⚡ How to Build & Use
Firmware (ESP32)
Install the ESP-IDF toolchain from Espressif.

Navigate to firmware/esp32.

Build: idf.py build

Flash & Monitor: idf.py -p /dev/ttyUSB0 flash monitor (adjust port as needed).

FPGA (Artix-7)
Install the Xilinx Vivado Design Suite.

Open the Vivado project from the fpga/artix7 directory.

Run Synthesis, Implementation, and Generate Bitstream.

Program the FPGA using the Vivado Hardware Manager.

Acknowledgements
Digilent for the Nexys A7 board and support.

Espressif Systems for the ESP32 and the ESP-IDF.
