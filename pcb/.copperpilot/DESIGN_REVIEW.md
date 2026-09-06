# Weather Station ESP32 Sensor Board — Hardware Design Review Report

**Date:** March 2025  
**Target Platform:** ESP32-C3-WROOM-02-H4 (KiCad 10 / Windows x64)  
**Scope:** Schematic Netlist, Component Datasheet Compliance, Bootstrapping, Analog Front-End, Power Integrity, and PCB Manufacturing DRC.

---

## 1. Executive Summary

A comprehensive electrical rule, netlist, component datasheet, and physical layout review was conducted for the **ESP32 Sensor Board** project. 

The review identified **4 Critical P0 Blockers** that prevent system operation, flashing, or cause short circuits, alongside **4 P1 High-Priority Flaws** affecting stability, bootstrapping, and SD card reliability. Additionally, the PCB layout currently reports **248 DRC violations** primarily due to unfilled internal copper planes and routing across switch terminals.

### Subsystem Evaluation Matrix

| Subsystem | Schematic Reference | Status | Primary Finding / Risk |
| :--- | :--- | :--- | :--- |
| **USB-C Input & ESD** | J1, F1, D5 | **PASS** | Dual 5.1 kΩ CC pull-downs, 1.1A PTC fuse, and USBLC6-2SC6 ESD clamping are properly configured. |
| **Battery Management** | U1 (MCP73871) | **ACTION REQUIRED** | Missing essential output capacitor on `VBAT`; input capacitor `C6` is undersized. |
| **3.3V Power Regulation** | U2 (LM1117-3.3) | **CRITICAL FLAW (P0)** | LM1117 dropout voltage (1.1V–1.2V) causes 3.3V rail collapse under 1S LiPo battery power (3.0V–4.2V). |
| **Auto-Programming / UART** | U4 (CP2102N-QFN20), Q2, Q3 | **CRITICAL FLAW (P0)** | CP2102N-QFN20 package lacks DTR pin; auto-reset circuit is wired to input pin `~CTS` (Pin 15). |
| **ESP32-C3 & Bootstrapping** | U3 (ESP32-C3), U6 (W25Q32) | **ACTION REQUIRED (P1)** | Floating strapping pin GPIO8; excessive capacitance (0.1 µF) on GPIO9; redundant external SPI Flash. |
| **Microphone Preamplifier** | U8 (MAX4466EXK) | **CRITICAL FLAW (P0)** | KiCad library symbol has Pin 1 (`IN+`) and Pin 2 (`GND`) inverted relative to Maxim SC-70-5 physical package. |
| **Environmental & Light Sensors** | U7 (BME280), Q1 (TEMT6000) | **PASS** | I2C pull-ups, addressing (0x76), and phototransistor analog conditioning are fully compliant. |
| **MicroSD Interface** | J3 (MicroSD Card Slot) | **ACTION REQUIRED (P1)** | Unused data lines `DAT1` and `DAT2` are tied directly to GND, violating SD SPI mode specifications. |
| **PCB Layout & DRC** | ESP32 sensor board.kicad_pcb | **CRITICAL FLAW (P0)** | 248 DRC violations: Direct copper shorts across SW1/SW2, 52 plane clearance errors on In1/In2, Edge.Cuts defect. |

---

## 2. Power Architecture & Voltage Regulation

```
[USB-C (J1)] ---> [PTC F1 (1.1A)] ---> [+5V_USB] ---> [MCP73871 (U1)] ===> [OUT] ---> [FB1] ---> [LM1117 (U2)] ---> [+3.3V]
                                                              ||                                     |
                                                     [LiPo Battery (J2)]                             +--> [ESP32 / Sensors]
```

### 2.1 Voltage Regulator Dropout Incompatibility (U2 - LM1117-3.3)
* **Observed Design:** U2 is an `LM1117MPX-3.3/NOPB` (SOT-223) linear regulator powered from the MCP73871 `OUT` rail to produce the global `+3.3V` system rail.
* **Datasheet Specification:** The LM1117 has a typical dropout voltage of $1.1\text{ V}$ at $500\text{ mA}$ ($1.2\text{ V}$ maximum at $800\text{ mA}$).
* **Operating Failure Mode:** A single-cell LiPo battery provides nominal voltage between $4.2\text{ V}$ (fully charged) and $3.0\text{ V}$ (discharged cutoff). 
  $$\text{Minimum Input for 3.3V Output} = 3.3\text{ V} + 1.1\text{ V} = 4.4\text{ V}$$
  When operating on battery, the LM1117 will enter immediate dropout across >90% of the discharge cycle. When battery voltage drops to $3.7\text{ V}$, output drops below $2.6\text{ V}$, well below the ESP32-C3 minimum operating threshold of $3.0\text{ V}$, causing brownouts and boot loops during Wi-Fi transmission bursts (peak current ~350 mA).
* **Mandatory Fix:** Replace U2 with an ultra-low-dropout regulator (e.g., **AP2112K-3.3**, **RT9013-33GB**, or **TLV75533PDBVR**) featuring $\le 250\text{ mV}$ dropout at $500\text{ mA}$ and stable operation with ceramic output capacitors.

### 2.2 Battery Charger Decoupling & Stability (U1 - MCP73871)
* **VBAT Decoupling Missing:** The MCP73871 datasheet (Section 6.1.2) mandates a minimum ceramic capacitor of $4.7\ \mu\text{F}$ directly between `VBAT` (Pins 14, 15) and `VSS`/`GND` for loop stability, battery insertion detection, and ripple suppression. Currently, `VBAT` has **0 µF** of local capacitance.
* **Charger Input Decoupling:** `C6` on `IN` (Pins 17, 18) is $1.0\ \mu\text{F}$. Microchip application notes mandate $\ge 4.7\ \mu\text{F}$ low-ESR ceramic capacitance.
* **Output LC Resonance Risk:** Ferrite bead `FB1` (`FBMJ2125HM330-T`) is placed in series between U1 `OUT` and U2 `VIN` before `C3` without a dedicated bypass capacitor directly at U1 `OUT` (Pins 11, 12). Transient load steps from the ESP32 can trigger high-Q ringing.

---

## 3. Microcontroller, Bootstrapping & Flash Subsystem

```
                          +-------------------+
                          |  ESP32-C3-WROOM   |
       +3.3V --[10k]--+---| GPIO8 (Pin 9)     | (Required HIGH at boot)
                      |   |                   |
       +3.3V --[10k]--+---| GPIO9 (Pin 10)    | (Pull HIGH, momentary LOW for boot)
             (R_boot) |   |                   |
                      +---| EN (Pin 3)        | (10k pull-up + 100nF to GND)
                          +-------------------+
```

### 3.1 Strapping Pin Analysis

| Pin | Net Name | Schematic State | Requirement (Espressif Datasheet) | Status | Corrective Action |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **GPIO2** | `SD_MISO` | Pulled to SD card / Flash | Must be floating or HIGH during boot | **PASS** | Shared on SPI bus; ensure high-Z during reset. |
| **GPIO8** | `GPIO8` | Connected to header J6, **No Pull-Up** | Must be **HIGH** at boot when GPIO9 is LOW | **FAIL (P1)** | Add external **10 kΩ pull-up** to `+3.3V`. |
| **GPIO9** | `BOOT` / `IO9` | 10k Pull-up + **C21 (0.1 µF)** to GND | Bootloader entry strapping pin | **WARNING (P1)** | Remove **C21**; capacitance delays edge transitions. |
| **CHIP_EN**| `ESP_EN` | 10k Pull-up (R17) + 100nF (C18) | Standard RC delay for power-on reset | **PASS** | Compliant with Espressif hardware design guidelines. |

### 3.2 External Flash Redundancy (U6 - W25Q32JVSSIQ)
* **Observation:** The project places an external 32 Mbit SPI Flash IC (`U6`) connected via SPI (GPIO2, GPIO7, GPIO6, GPIO10).
* **Module Conflict:** The chosen module is `ESP32-C3-WROOM-02-H4`, which internally integrates a **4 MB embedded SPI flash** connected via internal pins. 
* **Recommendation:** U6 is redundant and shares bus bandwidth with the MicroSD card slot. If secondary SPI storage is required, keep U6 isolated with dedicated CS (`GPIO10`); otherwise, mark U6 as DNP (Do Not Populate) to reduce BOM cost and routing congestion.

---

## 4. USB-to-UART Interface & Auto-Download Circuitry

```
  [CP2102N-QFN20]                                 [Auto-Download Network]
  Pin 16 (RTS) ---------\                        /------------> [ESP_EN] (Reset)
                         \                      /
                          +--[Q2 Base / Collector]
                          +--[Q3 Base / Collector]
                         /                      \
  Pin 15 (~CTS) --------/                        \------------> [BOOT / IO9]
  *** FLAW: Pin 15 is CTS INPUT, NOT DTR OUTPUT ***
```

### 4.1 CP2102N QFN-20 Pin Limitation
* **Observed Wiring:** Transistor `Q3` base-emitter network is connected to U4 Pin 15 (`~CTS`) and Pin 16 (`~RTS`).
* **Hardware Incompatibility:** The `CP2102N-A02-GQFN20` package **does not provide a DTR signal**. Pin 15 is strictly `~CTS` (Clear To Send), which is a digital **input** driven by external DCE devices.
* **Failure Mode:** `esptool.py` and the Arduino/ESP-IDF toolchains toggle `DTR` and `RTS` modem lines to sequence `CHIP_EN` and `GPIO9` into ROM serial download mode. Because Pin 15 cannot drive outputs, automatic programming will fail completely.
* **Mandatory Fix:** 
  1. Upgrade U4 to **CP2102N-A02-GQFN24** (Pin 23 = `~DTR`, Pin 19 = `~RTS`).
  2. Or substitute with **CH340C** (SOP-8, internal crystal, native DTR/RTS breakout).

---

## 5. Sensors & Analog Front-End

```
                      +-------------------+
                      |   MAX4466 (SC70-5)|
   [GND] ------------>| Pin 1 (GND)       |  <--- Physical Pinout
   [Mic Bias + Div] ->| Pin 2 (IN+)       |  <--- Physical Pinout
   [Feedback Loop] -->| Pin 3 (IN-)       |
   [ADC / IO0] <------| Pin 4 (OUT)       |
   [+3.3V] ---------->| Pin 5 (VCC)       |
                      +-------------------+
```

### 5.1 MAX4466 Microphone Preamp Symbol Inversion (U8)
* **Root Cause Analysis:** In the schematic symbol `Libraries/Symbols/MAX4466EXK.kicad_sym`, Pin 1 is defined as `IN+` and Pin 2 as `GND`.
* **Datasheet Specification (Analog Devices / Maxim SC-70-5):**
  * Pin 1: `GND` (Ground)
  * Pin 2: `IN+` (Non-Inverting Input)
  * Pin 3: `IN-` (Inverting Input / Gain Setting)
  * Pin 4: `OUT` (Amplifier Output)
  * Pin 5: `VCC` (Supply Voltage)
* **Resulting Circuit Failure:** The physical footprint connects Pin 1 to the AC mid-rail bias divider (`R26`/`R27`) and Pin 2 directly to Ground. When manufactured, the non-inverting input will be shorted directly to GND, and internal IC ground will float across the 200 kΩ bias network. The microphone will output silence/clipping.
* **Mandatory Fix:** Correct the pin numbers in `MAX4466EXK.kicad_sym` to match SC-70 standard pinout.

### 5.2 MicroSD Card Slot Wiring (J3)
* **Flaw:** Unused lines `DAT1` (Pin 8) and `DAT2` (Pin 9) are tied directly to Ground.
* **SD Card Association Standard:** In 1-bit SPI mode, `DAT1` and `DAT2` should either be left floating or pulled UP with 10 kΩ–50 kΩ resistors. Grounding data lines risks bus drive contention during power-up negotiation when the card initializes in SD native mode prior to receiving the SPI `CMD0` reset command.

### 5.3 Validated Sensors
* **BME280 (U7):** Configured in I2C mode with `CSB` tied to `+3.3V`, `SDO` grounded (Address `0x76`), and 4.7 kΩ pull-ups on `SDA`/`SCL`. Fully compliant.
* **TEMT6000 (Q1):** Ambient light sensor wired as emitter-follower with 10 kΩ load resistor (`R23`) and 100 nF smoothing filter (`C25`) feeding ADC input `IO1`. Fully compliant.

---

## 6. PCB Physical Layout & DRC Audit

KiCad 10 DRC execution on `ESP32 sensor board.kicad_pcb` identified **248 violations**.

```
========================================================================
                      DRC VIOLATION BREAKDOWN
========================================================================
[1] Copper Shorts (Critical)               : 4 instances (SW1, SW2)
[2] Plane Clearance Errors (In1.Cu, In2.Cu) : 52 instances
[3] Solder Mask Web Clearance (<0.05 mm)   : 66 instances (QFN/LGA)
[4] Silkscreen Overlap                     : 118 instances
[5] Board Outline Anomaly (Edge.Cuts)      : 1 warning (2 nm segment)
------------------------------------------------------------------------
TOTAL VIOLATIONS                           : 248
========================================================================
```

### 6.1 Critical Pushbutton Copper Shorts
* **Location:** Pushbuttons `SW1` (Reset) and `SW2` (Boot).
* **Defect:** Traces on `F.Cu` connecting to `ESP_EN` and `BOOT` pass directly through the opposite terminal pads which are tied to `GND`. This creates a dead short between `ESP_EN`/`BOOT` and Ground on the physical copper layer, holding the ESP32 in continuous reset.

### 6.2 Un-isolated Vias on Internal Power Planes
* **Location:** `In1.Cu` (GND Zone) and `In2.Cu` (3.3V Zone).
* **Defect:** 52 signal and power vias penetrating the internal layers lack cleared thermal reliefs or zone boundaries because zones have not been refilled after routing adjustments. Refilling zones resolves these violations.

### 6.3 Board Outline Defect
* **Location:** `Edge.Cuts` layer contains a 2 nm zero-length line coordinate anomaly at `(X: 145.2 mm, Y: 88.0 mm)` causing outline discontinuity warnings during Gerber generation.

---

## 7. Prioritized Corrective Action Plan

### Priority 0: Critical Blockers (Must Fix Before PCB Fabrication)

| Item | Target | Description of Action |
| :---: | :--- | :--- |
| **P0.1** | **U8 (MAX4466)** | Edit `MAX4466EXK.kicad_sym`: Remap Pin 1 to `GND` and Pin 2 to `IN+`. Update schematic and verify netlist. |
| **P0.2** | **U2 (LM1117)** | Replace `LM1117MPX-3.3` with low-dropout LDO **AP2112K-3.3** (SOT-23-5) or **TLV75533PDBVR**. |
| **P0.3** | **U4 (CP2102N)** | Replace `CP2102N-A02-GQFN20` with **CP2102N-A02-GQFN24** to obtain physical `~DTR` output for auto-flashing. |
| **P0.4** | **SW1 / SW2 Tracks** | Reroute `F.Cu` tracks at `SW1` and `SW2` to remove dead shorts between `ESP_EN`/`BOOT` and Ground pads. |

### Priority 1: High Priority (Reliability & Operating Margin)

| Item | Target | Description of Action |
| :---: | :--- | :--- |
| **P1.1** | **U1 (MCP73871)** | Add a $4.7\ \mu\text{F}$ to $10\ \mu\text{F}$ ceramic capacitor from `VBAT` to `GND`. Increase `C6` on `IN` from $1\ \mu\text{F}$ to $4.7\ \mu\text{F}$. |
| **P1.2** | **ESP32 Strapping** | Add a $10\text{ k}\Omega$ pull-up resistor from `GPIO8` to `+3.3V`. Remove capacitor `C21` ($0.1\ \mu\text{F}$) on `GPIO9`. |
| **P1.3** | **J3 (MicroSD)** | Disconnect `DAT1` (Pin 8) and `DAT2` (Pin 9) from Ground; leave floating or add $10\text{ k}\Omega$ pull-ups. |
| **P1.4** | **U6 (W25Q32)** | Mark external SPI Flash as DNP unless separate data logging partition on distinct CS is required. |

### Priority 2: PCB Layout & DFM Improvements

| Item | Target | Description of Action |
| :---: | :--- | :--- |
| **P2.1** | **Zone Fills** | Execute full zone refill across all copper layers (`F.Cu`, `B.Cu`, `In1.Cu`, `In2.Cu`) to clear plane DRC errors. |
| **P2.2** | **Edge.Cuts** | Clean 2 nm null segment on `Edge.Cuts` to establish a closed, validated board outline polygon. |
| **P2.3** | **Solder Mask** | Adjust solder mask clearance rules in `.kicad_pro` to minimum $0.05\text{ mm}$ bridge width for QFN/LGA packages. |

---

## 8. Verified Component Recommendations

| Component Role | Current Part | Recommended Alternate Part Number | Package | Key Advantage |
| :--- | :--- | :--- | :--- | :--- |
| **3.3V LDO** | LM1117MPX-3.3 | **AP2112K-3.3TRG1** / **RT9013-33GB** | SOT-23-5 | 250 mV dropout @ 500 mA, supports full LiPo range. |
| **USB-to-UART** | CP2102N-A02-GQFN20 | **CP2102N-A02-GQFN24R** | QFN-24 | Dedicated DTR + RTS hardware pins for auto-reset. |
| **LiPo Charger** | MCP73871-2CAI/ML | *Retain MCP73871* (Add decoupling caps) | QFN-20 | Integrated dynamic power path management. |
| **ESD Clamping** | USBLC6-2SC6 | **USBLC6-2SC6** | SOT-23-6 | Low capacitance (3.5 pF) USB 2.0 full-speed ESD protection. |
