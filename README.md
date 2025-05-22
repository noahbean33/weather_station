# Compact Digital Radio: iCE40 FPGA DSP & ESP32-S3 Rust Control 

[![Rust CI](https://github.com/YOUR_USERNAME/YOUR_REPONAME/actions/workflows/rust.yml/badge.svg)](https://github.com/YOUR_USERNAME/YOUR_REPONAME/actions/workflows/rust.yml)
[![Verilog/VHDL CI](https://github.com/YOUR_USERNAME/YOUR_REPONAME/actions/workflows/fpga.yml/badge.svg)](https://github.com/YOUR_USERNAME/YOUR_REPONAME/actions/workflows/fpga.yml)
A portable digital radio (initially FM, potentially AM) featuring real-time Digital Signal Processing (DSP) on a Lattice iCE40 FPGA. An ESP32-S3 microcontroller running Rust firmware manages the user interface, controls a radio tuner IC, and orchestrates the system. The project includes a custom-designed PCB housing the RF frontend, audio circuitry, and system integration components.

This project serves as a capstone-level endeavor, exploring embedded systems, FPGA design, DSP, RF electronics, and custom PCB development.

---

##  Features

* **Digital Radio Reception:**
    * Primary Goal: FM broadcast band (88-108 MHz) with mono audio.
    * Stretch Goal: AM broadcast band reception.
* **FPGA-Accelerated DSP:**
    * Real-time FM demodulation on the Lattice iCE40 FPGA.
    * Digital volume control implemented in hardware.
* **Microcontroller Control:**
    * ESP32-S3 with Rust firmware for robust and safe system management.
* **User Interface:**
    * LCD/OLED display for frequency, mode, and volume information.
    * Rotary encoder/knob for channel tuning.
    * Rotary encoder/knob for volume adjustment.
* **Audio Output:**
    * Speaker output via an integrated audio amplifier.
    * (Optional) Headphone jack.
* **Custom Hardware:**
    * Custom-designed PCB integrating the RF tuner, audio amplifier, power supplies, and microcontroller/FPGA board connectors.

---

## 🛠️ Hardware Components

* **Microcontroller:** ESP32-S3 Module (e.g., ESP32-S3-WROOM-1)
* **FPGA Board (Initial Prototyping):** [Nandland Go Board](https://www.nandland.com/goboard/index.html) (Lattice iCE40UP5K)
* **FPGA Chip (for Custom PCB - Optional):** Lattice iCE40UP5K (or similar)
* **RF Tuner IC:**
    * Primary: FM Receiver (e.g., Si4703, TEA5767)
    * Optional (AM/FM): (e.g., Si4730, Si4735)
    * *Chosen IC:* `[Specify your chosen tuner IC here]`
* **Display:** I2C OLED Display (e.g., SSD1306 128x64) or Character LCD
* **Audio Amplifier:** LM386, TDA2822, or a Class-D amplifier module
* **DAC (Digital-to-Analog Converter):**
    * If tuner/FPGA outputs I2S: PCM5102A or similar.
    * ESP32-S3 internal DAC (for simpler, lower-fidelity audio).
* **Input Controls:** 2x Rotary Encoders (or potentiometers)
* **Speaker:** Small 4/8 Ohm speaker
* **Antenna:** Telescopic for FM, Ferrite bar for AM (if implemented)
* **Power Supply:** LiPo battery with charger module, or USB power with LDOs for various voltage rails (3.3V, etc.)

---

## 🏗 System Architecture

```mermaid
graph TD
    A[Antenna] --> RF_TUNER;
    RF_TUNER[RF Tuner IC (e.g., Si4703)];
    UI_Knobs[Tuning/Volume Knobs] --> ESP32S3;
    ESP32S3[ESP32-S3 (Rust Firmware)];
    ESP32S3 -- I2C/SPI --> RF_TUNER;
    ESP32S3 -- I2C/SPI --> LCD[LCD/OLED Display];
    ESP32S3 -- SPI/GPIO --> ICE40;

    subgraph "FPGA Processing (iCE40)"
        ICE40[Lattice iCE40 FPGA]
        direction LR
        ADC_IF[ADC Interface (Optional)]
        I2S_IN[I2S Input (from Tuner/ADC)]
        FM_DEMOD[FM Demodulator DSP]
        VOL_CTRL[Digital Volume Control]
        I2S_OUT[I2S Output (to DAC)]
        PWM_OUT[PWM Audio (Basic)]

        I2S_IN --> FM_DEMOD;
        FM_DEMOD --> VOL_CTRL;

    end

    RF_TUNER -- Digital Audio (I2S) / Analog IF/AF --> ICE40_Input_Interface;

    subgraph "FPGA Input Interface (Select One)"
        direction TB
        ICE40_Input_Interface
        ICE40_Input_Interface --> I2S_IN;
        OR[Or]
        ICE40_Input_Interface --> ADC_IF;
        ADC_IF --> FM_DEMOD
    end


    VOL_CTRL --> Audio_Output_Path;


    subgraph "Audio Output Path (Select One)"
        direction TB
        Audio_Output_Path
        Audio_Output_Path --> I2S_OUT;
        Audio_Output_Path_OR[Or]
        Audio_Output_Path --> PWM_OUT;
    end


    I2S_OUT --> DAC[External DAC (e.g., PCM5102A)];
    DAC --> AUDIO_AMP;
    PWM_OUT --> AUDIO_AMP;
    ESP32S3 -- I2S/Analog --> AUDIO_AMP;


    AUDIO_AMP[Audio Amplifier (e.g., LM386)];
    AUDIO_AMP --> Speaker;
RF Frontend: The Tuner IC receives the radio signal, performs analog downconversion, and outputs either a digital audio stream (e.g., I2S) or an analog Intermediate Frequency (IF)/Audio Frequency (AF) signal.
ADC (Optional): If the tuner provides analog output, an ADC digitizes this signal for the FPGA. (Preference is for a tuner with I2S out).
FPGA (iCE40):
Receives digitized audio (I2S or parallel from ADC).
Performs FM demodulation (and potentially AM demodulation).
Applies digital volume control based on commands from the ESP32-S3.
Outputs a processed digital audio stream (I2S to an external DAC or PWM for direct, simpler output).
ESP32-S3 Microcontroller:
Manages the user interface (knobs, display).
Configures and controls the RF Tuner IC via I2C/SPI (sets frequency, band, etc.).
Communicates with the FPGA to send control data (like volume level) and potentially receive status.
May handle final audio output via its internal DAC or an I2S DAC if FPGA processing is minimal.
Audio Output: A DAC converts the digital audio from the FPGA/ESP32-S3 to analog, which is then amplified to drive a speaker.
 Software & Tools
FPGA Development:
Language: Verilog or VHDL
Synthesis: Yosys
Place & Route: NextPNR
Programming/Toolchain: iCEStorm project tools (iceprog, icetime, etc.) or Lattice Radiant/Diamond (if preferred and iCE40 variant is supported).
Simulation: GHDL / Icarus Verilog with GTKWave
Embedded Firmware (ESP32-S3):
Language: Rust
Environment: esp-idf-template with esp-idf-hal or esp-hal for bare-metal.
Tools: cargo, espflash, probe-rs (for debugging).
PCB Design:
KiCad, Eagle, or other PCB design software.
DSP Algorithm Simulation & Design:
MATLAB (with Signal Processing Toolbox)
Python (with NumPy, SciPy.signal, Matplotlib)
 Project Phases & Current Status
Phase 0: Setup & Familiarization (Completed / In Progress) 
[ ] ESP32-S3 Rust development environment setup.
[ ] iCE40 open-source FPGA toolchain setup (Yosys, NextPNR, etc.).
[ ] Basic DSP algorithm simulation in MATLAB/Python.
Phase 1: ESP32-S3 Core Control & UI (In Progress) 
Goal: ESP32-S3 controls tuner IC, display, and knobs. Basic audio out from tuner (bypassing FPGA).
Tasks:
[ ] Interface LCD/OLED with ESP32-S3.
[ ] Read knob inputs.
[ ] Interface and control selected RF Tuner IC via I2C/SPI.
[ ] Firmware to tune radio based on knob input, display frequency.
[ ] Milestone: Direct analog audio output from tuner to amplifier (if tuner supports).
Phase 2: FPGA DSP Implementation (To Do) 
Goal: Implement core demodulation on iCE40.
Tasks:
[ ] Select and simulate simplified FM (mono) demodulation for iCE40 in MATLAB/Python.
[ ] Design VHDL/Verilog for I2S/ADC input interface.
[ ] Implement FM demodulator on iCE40.
[ ] Implement digital volume control on iCE40.
[ ] Design VHDL/Verilog for I2S/PWM audio output.
[ ] Simulate, synthesize, and test on Nandland Go Board.
Phase 3: Custom PCB Design & Fabrication (To Do) 
Goal: Design, fabricate, and assemble the main project PCB.
Tasks:
[ ] Schematic capture (tuner, audio amp, power, connectors).
[ ] PCB layout.
[ ] Order PCB fabrication.
[ ] Assemble and test PCB incrementally (power, then MCU, then tuner, etc.).
Phase 4: System Integration & Testing (To Do) 
Goal: All hardware and software components working together on the custom PCB.
Tasks:
[ ] Port ESP32-S3 firmware to the custom PCB setup.
[ ] Integrate iCE40 (Go Board or on-PCB chip) with ESP32-S3.
[ ] Test RF Frontend -> (ADC) -> FPGA -> DAC -> Audio Amplifier chain.
[ ] Debug and iterate.
Phase 5: Final Touches & Documentation (To Do) 
Goal: Refined, documented, and demonstrable project.
Tasks:
[ ] Refine audio quality and UI.
[ ] (Stretch Goal) AM reception.
[ ] (Stretch Goal) Battery power.
[ ] Complete final documentation.

Repository Structure
.
├── firmware/esp32s3           # ESP32-S3 Rust firmware
│   ├── Cargo.toml
│   └── src/
├── fpga/ice40                 # iCE40 VHDL/Verilog code
│   ├── src/                   # Source files (.vhd, .v)
│   ├── sim/                   # Simulation files and testbenches
│   └── constraints/           # Pin constraints (.pcf)
├── hardware/pcb               # Custom PCB design files (KiCad, Eagle, etc.)
│   ├── schematic/
│   └── layout/
├── docs/                      # Documentation, block diagrams, images
├── matlab_sim/                # MATLAB/Python DSP simulation scripts
├── .github/workflows          # GitHub Actions for CI (Rust, FPGA)
├── LICENSE
└── README.md
How to Build / Use
(This section will be updated as the project progresses)

Firmware (ESP32-S3)
Install Rust and the ESP32 toolchain (see esp-rs documentation).
Navigate to firmware/esp32s3.
Build: cargo build
Flash: espflash flash /dev/ttyUSB0 target/xtensa-esp32s3-espidf/debug/your_firmware_name (adjust port and binary name).
FPGA (iCE40)
Install the open-source iCE40 toolchain (Yosys, NextPNR, iCEStorm).
Navigate to fpga/ice40.
Run make (you'll need to create a Makefile for synthesis, PnR, and programming).
Example Makefile targets: make synth, make pnr, make prog.
Contributing
Contributions, issues, and feature requests are welcome! Feel free to check the issues page.

License
This project is licensed under the MIT License / Apache 2.0 License - choose or specify.

Acknowledgements 
Nandland for the Go Board and educational content.
The esp-rs (Rust on ESP32) community.
The open-source FPGA community.
<!-- end list -->
