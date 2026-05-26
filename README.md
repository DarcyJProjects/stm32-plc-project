# Industrial Automation Embedded Controller

### Introduction

This project was a deep dive into 4-layer PCBs, galvanic isolation, strict EMI filtering, and mixed-signal design with the goal to create a robust, industrial-style controller from scratch.

Built around an STM32 Cortex-M4, this embedded controller handles galvanically isolated field I/O using a custom DMA-driven MODBUS RTU stack. It also features a "no-code" automation engine, letting you configure logic remotely via a web panel.

I've implemented several hardware features including an RTC with battery backup, HMI OLED, microSD logging, an EEPROM, efficient multi-stage power distribution & conversion with real time power monitoring, as well as many protection features.

The entire ecosystem - the custom PCB, schematics, firmware, and the web panel - is completely open-source right here, so feel free to look into it and see how it works!

#### Datasheet: Read the full datasheet [here](https://github.com/DarcyJProjects/embedded-controller/blob/main/Documentation/datasheet/datasheet.pdf).

<img src="https://raw.githubusercontent.com/DarcyJProjects/embedded-controller/refs/heads/main/Media/1.png" title="" alt="IMG: Main" data-align="center">

<p>
  This project is open source hardware under the <strong>CERN Open Hardware Licence Version 2 - Strongly Reciprocal (CERN-OHL-S v2)</strong>.
</p>
<img src="https://resources.oshwa.org/files/assets/oshw-logo-filled-color.png" alt="Open Source Hardware Logo" height="64" style="vertical-align: middle; margin-right: 8px;">

The editable hardware source files are provided so that the design can be studied, modified, and manufactured in accordance with the licence.

⚖️**Disclaimer:** *This hardware is a personal engineering project created solely for **educational purposes and skill development**. While it explores industrial design techniques, it has not been tested against or certified for any specific industrial standards (e.g., IEC, UL, CE). It is provided ”AS IS” without warranty. For full licence details, design files, and terms of use, please see the **[licence section of this document](#-licence)**.*

---

## 📑 Table of Contents

* 📷 [Demo](#-demo)

* 🎛 [Features and Specifications](#-features-and-specifications)

* ⚡[How It Works](#-how-it-works)

* 🧠 [Why I Built This](#-why-i-built-this)

* 🛠️ [Getting Started](#-getting-started)

* 📂 [Repo Structure](#-repo-structure)

* 📜 [Licence](#-licence)

* ⚠️ [Safety Disclaimer](#-safety-disclaimer)

* 🔗 [Acknowledgements](#-acknowledgements)

* 💬 [Feedback & Contributions](#-feedback--contributions)

---

## 📷 Demo

For the demo, I mounted the controller on a DIN rail and wired it into a small irrigation-style test rig. The goal was to demonstrate as many parts of the board as possible, including relay outputs, digital inputs, analogue voltage signals, 4-20 mA current loops, and MODBUS-based monitoring/control.

Coming soon...

---

## 🎛 Features and Specifications

I tried to pack a wide range of industrial-style hardware and firmware features into this controller so I could learn as much as possible across the realms of PCB design, embedded firmware, power electronics, isolation, and automation protocols.

<img title="" src="https://raw.githubusercontent.com/DarcyJProjects/embedded-controller/refs/heads/main/Media/5.png" alt="IMG: PCB" data-align="center">

### **Core Specifications:**

- **Core:** STM32G431 Cortex-M4 MCU running at up to 170 MHz
- **Power input:** 7–28 V DC input range
- **PCB:** Custom 4-layer PCB with separated logic and field domains
- **Protection:** Reverse polarity protection, over-current protection, TVS protection, and ESD protection
- **Isolation:** Galvanic isolation between the logic side and field-side interfaces
- **Memory:** 4 KB EEPROM with CRC-backed configuration and automation rule storage
- **Connectivity:** Isolated RS485 for MODBUS RTU, plus USB-C for DFU flashing and serial debugging
- **Control:** Web panel with custom "no-code" automation rules and MODBUS register access
- **Local interface:** OLED display, tactile buttons, status LEDs, and hardware mode switches, on-board configuration factory reset
- **Expansion:** Isolated I²C header for supported field-side peripherals

### Isolated Field I/O:

- **4x Analogue inputs**
  
  - 12-bit measurement
  - Each configurable for 0–5 Vor 4–20 mA operation

- **2x Analogue outputs**
  
  - 12-bit output
  - Each configurable for 0–5 V voltage output or 4–20 mA current-loop operation

- **4x Digital inputs**
  
  - Designed for 24 V DC signalling

- **2x Digital outputs**
  
  - Low-side sinking outputs for 24 V DC loads

- **2x Relay outputs**
  
  - Intended for ELV switching only
  - Rated in the datasheet for 30 V DC / 50 V AC application limits

- **Isolated I²C expansion header**
  
  - Allows supported external devices to be mapped into the MODBUS register space with custom firmware drivers

### Power:

- Buck Converter for High Efficiency 24 V to 5 V conversion
- Linear Regulator for a clean 3.3 V supply to the MCU
- Switching Isolation Power Supply to power the field side circuitry
- PPTC Fuses, "Ideal Diode" Reverse Polarity Protected Input

### On Board HMI:

- 0.96" OLED display
- 4x tactile buttons for menu navigation, reset, DFU entry, and factory reset
- Status LEDs for power, isolated power, communication, relay state, and general status
- Hardware mode switches for analogue current/voltage configuration

### Block Diagram:

<img src="https://raw.githubusercontent.com/DarcyJProjects/embedded-controller/refs/heads/main/Documentation/datasheet/general_block_diagram.png" title="" alt="IMG: Block diagram" data-align="center">

---

## ⚡ How It Works

The controller is split into two main electrical domains:

1. **Logic Domain:** STM32G4 MCU, EEPROM, OLED, RTC, buttons, USB-C DFU, isolated signals coming from the I/O isolators, etc.

2. **Field Domain:** I/O Inputs and Outputs, Relays, RS485, Isolated I²C expansion header

These two domains are separated by a galvanic isolation barrier. If you look at the PCB, you can actually see a moat separating the two sides copper pours.

The field side is powered through an isolated 5V DC/DC converter, and the signals that cross the barrier use appropriate isolators depending on the interface (capacitive, optical, etc).

### MODBUS register model

The firmware exposes the controller's I/O through a MODBUS register map:

- **Discrete inputs** for digital input states
- **Coils** for digital outputs and relay outputs
- **Input registers** for analogue input readings
- **Holding registers** for analogue output setpoints

There are also **virtual registers**, which can be created dynamically and used as intermediate values in the automation logic. This is useful for when you need to link many rules together for more complex automation logic.

### Custom automation engine

One of the main firmware features is the local automation engine. Instead of reflashing the STM32 every time you want to update its configuration or more importantly its automation logic, rules can be configured directly over MODBUS using custom vendor-defined (that's me!) function codes.

The idea is to make the board behave a little more like a tiny configurable PLC. A rule can read an input, compare a value, update an output, or write to a virtual register. Once configured, the STM32 runs the logic locally.

### Hybrid MODBUS master/slave concept

The firmware is also designed around a hybrid MODBUS architecture allowing the controller to act as both a master and slave at the same time. The controller can report data to an upstream system while also communicating with downstream MODBUS devices.

For instance, [I reverse engineered an RS485 SHT30 sensor](https://darcyjprojects.xyz/index.php/2025/07/08/reverse-engineering-a-cheap-sht30-rs485-modbus-sensor/) used for reading temperature and humidity. By connecting this to the RS485 line of the embedded controller, it can read from the sensor over MODBUS (as a master to the slave sensor) while also reporting information requested back to the web panel (as a slave to the master web panel). 

This was mainly an experiment in building something more flexible than a fixed slave board, while still keeping the system simple enough to understand and debug.

![IMG: Web Panel](https://raw.githubusercontent.com/DarcyJProjects/embedded-controller/refs/heads/main/Media/web-panel1.png)

---

## 🧠 Why I Built This

This project was a major undertaking compared to my earlier electronics projects, integrating mixed signal design, complex isolation barriers, and low level C protocol implementation. I wanted to build something that forced me to deal with more realistic embedded hardware problems, relevant to industrial, mining, manufacturing and remote monitoring environments.

### What I learnt: hardware

- 4-layer PCB design
- Altium Designer workflow
- Mixed-signal PCB layout
- Analogue and digital galvanic isolation
- Isolated power supply design
- 24 V field input and output circuitry
- 4–20 mA current-loop inputs and outputs
- RS485 physical-layer design, termination, and biasing
- Relay output design and low-side switching
- Protection circuitry using PPTC fuses, TVS diodes, and MOSFET-based reverse polarity protection
- Rail-to-rail op-amp circuits for buffering, scaling, and signal conditioning
- Debugging real PCB mistakes using trace cuts, enamel wire, and a lot of patience
- Parasitic impedance and what it does to buck converters!

### **What I learnt: Software**

- STM32CubeIDE and STM32G4 firmware development
- Writing a MODBUS RTU stack from scratch
- Using DMA so communication does not block the CPU
- Building drivers for EEPROMs, RTCs, OLEDs, and other peripherals
- Storing configuration data with CRC checks
- Designing a small rule engine for local automation logic
- Mapping physical and virtual I/O into a MODBUS register model
- Building a Node.js web panel to configure and monitor an embedded device
- How to use LaTeX to write documentation

<img src="https://raw.githubusercontent.com/DarcyJProjects/embedded-controller/refs/heads/main/Media/prototyping1.png" title="" alt="IMG: Prototyping" data-align="center">

---

## 🛠️ Getting Started

> **Note:** This project is not a polished commercial product or a beginner-friendly kit. It is a learning project and should be treated carefully, especially when connecting external power supplies, field wiring, or loads.

### Hardware

Before powering the board:

1. Inspect the PCB for solder bridges, assembly mistakes, damaged connectors, or incorrect component values.
2. Check the analogue input/output mode switches before connecting sensors or actuators.
3. Confirm the input supply is within the supported DC range.
4. Use a current-limited bench supply for first power-up.
5. Do not connect mains voltage or safety-critical loads.
6. Check the datasheet before connecting external devices.

### Firmware

The STM32 firmware project is intended to be opened in STM32CubeIDE.

Typical workflow:

1. Open the firmware project
2. Build the project in STM32CubeIDE.
3. Flash the board using USB DFU (hold down BOOT0 and press NRST while powered to enter DFU mode)
4. Connect over serial/MODBUS for debugging or configuration.

### Web panel

The web panel is intended to run locally and communicate with the controller over a USB/serial MODBUS connection.

```bash
cd "Web Panel"
npm install
npm start
```

Then open the local address shown in the terminal in a web browser. You will need an RS485 to USB-Serial converter so that you can select a COM port to communicate with the embedded controller.

---

## 📂 Repo Structure

```text
stm32-plc-project/
├── Documentation/              # Datasheet, drawings
├── Firmware/                   # STM32CubeIDE firmware project
├── Hardware/                   # Altium Designer project: PCB design files, schematics
├── Web Panel/                  # Node.js web configuration panel
├── Media/                      # Photos, renders, screenshots, and demo images
├── LICENCE-CERN-OHL-S-2.0.txt  # CERN-OHL-S v2 licence text for hardware
├── LICENCE-MIT.txt             # MIT licence text for software
└── README.md                   # this file!
```

---

#### 📜 Licence

This repository contains both hardware design files and software.

### Hardware

The hardware design files are licenced under the **CERN Open Hardware Licence Version 2 - Strongly Reciprocal (CERN-OHL-S v2)**.

This applies to the PCB design files, schematics, hardware source files, mechanical drawings, and other files required to study, modify, manufacture, and distribute the hardware design.

You are free to:

- **Study** and **modify** the design
- **Make** your own copies
- **Distribute** modified versions, as long as you follow the licence terms

In general, the strongly reciprocal licence means that modified versions of the covered hardware design must also be shared under the same licence, with attribution and source files provided.

For the full legal terms, see [LICENCE-CERN-OHL-S-2.0.txt](https://github.com/DarcyJProjects/embedded-controller/blob/main/LICENCE-CERN-OHL-S-2.0.txt).

### Firmware and Software

Unless otherwise stated, the firmware and software in this repository are licenced under the **MIT Licence**.

This applies to the STM32 firmware and the web configuration panel.

For the full legal terms, see [LICENCE-MIT.txt](https://github.com/DarcyJProjects/embedded-controller/blob/main/LICENCE-MIT.txt).

### Third-Party Code and Resources

Some parts of this project make use of third-party open-source code and resources. These remain under their original licences and are acknowledged in the [Acknowledgements](#-acknowledgements) section.

<img src="https://raw.githubusercontent.com/DarcyJProjects/embedded-controller/refs/heads/main/Media/3.png" title="" alt="IMG: Close up" data-align="center">

---

## ⚠️ Safety Disclaimer

This hardware is a personal engineering project created solely for educational purposes, experimentation, and skill development.  

Although it uses industrial-style design techniques, it has **not** been tested, approved, certified, or validated against industrial, safety, EMC, electrical, environmental, functional safety, or regulatory standards, including IEC, UL, CE, RCM, or any similar certification scheme.  

**LIMITATION OF LIABILITY:**

**THIS PROJECT IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE COPYRIGHT HOLDER ACCEPTS NO LIABILITY FOR ANY DAMAGE, INJURY, DEATH, LOSS, NON-COMPLIANCE, EQUIPMENT FAILURE, DATA LOSS, BUSINESS INTERRUPTION, OR ANY OTHER ISSUE RESULTING FROM THE USE, MODIFICATION, MANUFACTURE, ASSEMBLY, DISTRIBUTION, OR ATTEMPTED DEPLOYMENT OF THIS DESIGN.**

Do **not** use this project in:  

- real industrial installations  
- safety-critical systems  
- mains-voltage systems  
- life-support, medical, automotive, aviation, mining, or hazardous environments  
- commercial products without proper engineering review, testing, certification, and regulatory approval  
- any system where failure could cause injury, property damage, environmental harm, financial loss, or unsafe operation  

Anyone using, modifying, manufacturing, or testing this design does so entirely at their own risk and is responsible for ensuring compliance with all applicable laws, standards, and safety requirements.

---

## 🔗 Acknowledgements

- The datasheet for this project was created using the [LaTeX Datasheet Template by Petteri Aimonen](https://github.com/PetteriAimonen/latex-datasheet-template), which is licenced under the LPPL-1.3c licence.

- The firmware directly uses the [stm32-ssd1306 OLED display driver by afiskon](https://github.com/afiskon/stm32-ssd1306) for the SSD1306-based OLED display, which is licenced under the MIT licence.

- The MicroSD / FatFS implementation was helped by this tutorial kiwih: [An SD card over SPI using STM32CubeIDE and FatFS](https://01001000.xyz/2020-08-09-Tutorial-STM32CubeIDE-SD-card/).

- Some parts of the firmware were written early in the project while the hardware was still being tested on breadboards. I have tried to acknowledge all external resources used, but if I have missed anything, please let me know and I will be happy to add the appropriate credit.

---

### 💬 Feedback & Contributions

Feel free to fork, share, and build your own!

If you build one, adapt part of the design, or use it to help with one of your own projects, I would love to see it - tag me on [LinkedIn](https://www.linkedin.com/in/darcywdjohnson/).

---

🔧 Darcy @ [www.darcyjprojects.xyz](https://www.darcyjprojects.xyz)
