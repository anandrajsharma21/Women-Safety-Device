# Women-Safety-Device-Documentation


# 🚨 TinyML Dual-Unit Acoustic Distress & Anti-Tamper Safety Wearable

> **Automated Edge-AI Personal Protection System with Audio Feature Pipeline, Biometric Cross-Validation, Cell-Tower Triangulation, and Anti-Forced Detachment System**

An end-to-end wearable personal safety architecture engineered for hazardous environments. The system utilizes an **ESP32 Microcontroller** running an on-device **TensorFlow Lite for Microcontrollers (TFLite Micro)** model that continuously processes ambient audio via an **MFCC feature extraction engine**. To guarantee zero false positives, acoustic distress signals are cross-validated against real-time physiological stress metrics (BPM / PPG Waveform) captured via an ergonomic smart wristband.

---

## 🏗️ 1. Two-Part System Architecture & Block Diagram

The device uses a discrete two-part architecture to maximize user safety and ensure reliable operation during emergencies:

1. **Hidden Main Unit (Base Module):** Concealed in a locket, bag, or inner clothing. Houses the ESP32 microcontroller, INMP441 I2S Digital Microphone, NEO-6M GPS module, SIM800L GSM module, and main LiPo power supply.
2. **Smart Wristband (Hand Bracelet):** Ergonomic wearable housing the **MAX30102 PPG Heart Rate Sensor** and a hardware-level **Interlock Continuity Loop (Anti-Tamper Circuit)**.

```
+-----------------------------------------------------------------------------------+
|                            HIDDEN BASE MODULE (Locket / Bag)                      |
|                                                                                   |
|  [ INMP441 Mic ] ──► [ DMA I2S Buffer ] ──► [ MFCC Extraction ] ──► [ TinyML ]   |
|                                                                        │          |
|  [ NEO-6M GPS ]  ──┐                                                   ▼          |
|                    ├──► [ Multi-Modal Decision Engine ] ──► [ SIM800L GSM Module ] |
|  [ LBS Cell-Loc ] ──┘                 ▲                                │          |
|                                       │                                ▼          |
|  [ Power / Battery ] ──► [ ESP32 Deep/Light Sleep ]           [ Emergency Contacts ]
+---------------------------------------┼-------------------------------------------+
                                        │ (Physical Hardware Interlock / I2C Bus)
+---------------------------------------┴-------------------------------------------+
|                          SMART WRISTBAND (Hand Bracelet)                          |
|                                                                                   |
|  [ MAX30102 PPG Sensor ]    ──► Real-Time Heart Rate & Stress Waveform Analysis    |
|  [ Hardware Interlock Loop ]──► Anti-Forced Detachment Interrupt Circuit          |
+-----------------------------------------------------------------------------------+
```

---

## 🔌 2. Complete Circuit Pin Matrix & Wiring Guide

### Detailed Pin Connections

| Module / Component | Module Pin | ESP32 GPIO | Operating Voltage | Current Draw | Circuit Description / Signal Type |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **INMP441 Digital Mic** | VDD | 3.3V | 3.3V | ~1.4 mA | Power Supply |
| | GND | GND | 0V | - | Common Ground |
| | SD | GPIO 32 | 3.3V Logic | - | I2S Serial Data Line |
| | WS | GPIO 25 | 3.3V Logic | - | I2S Word Select (L/R Clock) |
| | SCK | GPIO 33 | 3.3V Logic | - | I2S Bit Clock |
| | L/R | GND | 0V | - | Selects Left Audio Channel |
| **MAX30102 PPG (Wrist)** | VCC | 3.3V | 3.3V | ~600 µA | Power Supply |
| | GND | GND | 0V | - | Common Ground |
| | SDA | GPIO 21 | 3.3V Logic | - | I2C Serial Data |
| | SCL | GPIO 22 | 3.3V Logic | - | I2C Serial Clock |
| **NEO-6M GPS Module** | VCC | 3.3V / 5V | 3.3V - 5V | ~45 mA | Power Supply |
| | GND | GND | 0V | - | Common Ground |
| | TX | GPIO 16 | 3.3V Logic | - | Hardware Serial RX2 |
| | RX | GPIO 17 | 3.3V Logic | - | Hardware Serial TX2 |
| **SIM800L GSM Module** | VCC | External 3.7V - 4.2V | 3.7V - 4.2V | **2A Peak** | **Requires Dedicated LiPo Battery Line** |
| | GND | GND | 0V | - | Must Share Common Ground with ESP32 |
| | TX | GPIO 26 | 3.3V Logic | - | Hardware Serial RX1 |
| | RX | GPIO 27 | 3.3V Logic | - | Hardware Serial TX1 |
| **Anti-Tamper Interlock** | Signal Line | **GPIO 5** | 3.3V Logic | - | **Pull-Up Loop (Active HIGH on Removal)** |
| **Manual Panic Switch** | Signal Line | GPIO 4 | 3.3V Logic | - | Push Switch (Internal Pull-Up) |

### Power Supply Circuit Layout
* **ESP32 Core & Low-Power Sensors:** Powered via a regulated 3.3V rail fed by a single 3.7V 18650/LiPo cell through an AP2112K-3.3 LDO regulator.
* **SIM800L Power Rail:** Connected directly to the 3.7V - 4.2V LiPo battery terminal with a 1000µF low-ESR capacitor placed parallel to pin connections to handle burst current spikes up to 2A during cellular transmission.

---
