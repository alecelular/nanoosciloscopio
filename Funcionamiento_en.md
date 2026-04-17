# Nano-Oscilloscope

This project describes the principles of operation, construction process, and technical solutions implemented for a ultra-compact oscilloscope based on microcontrollers.

> **Note:** In case of any doubt or ambiguity in the interpretation of this document, the original Spanish version shall prevail.

---

## 🚀 Introduction

The documentation and code are primarily focused on the **ATtiny85**. However, the system is also adapted for the **ATmega328P**. 

*Note: A tentative pinout description for the ATtiny84 is included, though specific code for that model is not yet provided.*

---

## 🛠️ General Specifications & Compilation

### Software Requirements
To compile the code for the **ATtiny85**, use the **Arduino IDE 1.8.19** with the following indispensable settings:
* **Core:** [ATtinyCore 1.4.1 / 1.5.2](http://drazzy.com/package_drazzy.com_index.json)
* **Settings:** 
    * `No millis()` (Mandatory to maximize Flash).
    * `LTO enabled`.
    * `No bootloader`.

### Memory and Frequency Management
* **Internal 8 MHz (No Crystal):** The code nearly reaches the Flash limit of the ATtiny85. This mode requires manual frequency calibration (50/60 Hz).
* **External Crystal:** Significantly reduces code size by removing calibration routines, allowing for more features and 3 pushbuttons.
* **ATmega328P:** Flash and RAM are more than sufficient, allowing for further modifications or extra code.

---

## 📺 Display Control (SSD1306 OLED)

The system supports **SSD1306** controllers (128x64 or 128x32).

### Configuration via `#define OLED`
Resolution is adjusted using numerical values:
* **128x64:** Values `5` to `8`, default: `8`. (Value `7` adds an empty bottom row for extra info).
* **128x32:** Values `2` to `4`, default: `4`. (Value `3` adds an empty bottom row).

*Hardware tip:* Lowering the define value can "rescue" damaged screens by shifting the active area away from broken pixels at the edges.

### I2C Bitbang Implementation
Standard I2C libraries are not used because the external crystal occupies the required pins. Instead, a custom **Bitbang** mode is used:
* **SDA:** PB0 (AIN0)
* **SCL:** PB1 (AIN1)
* **Font:** To save Flash, the character set is restricted to code `0x5A` (uppercase only). This can be modified by changing `FUENTE_MAX` in `I2C.ino`.

---

## 🕹️ User Interface and Pushbuttons

The number of buttons is set via `#define PULSADORES` (1, 2, or 3).

* **Button 1 (Main):** Detected through a voltage imbalance using the internal comparator on the SDA/SCL lines. It allows screen captures or menu access during long measurements.
* **Buttons 2 & 3 (Optional):** 
    * Without a crystal, dedicated pins can be used.
    * With a crystal, the **RESET pin** is used in analog mode. Different resistor values identify which button is pressed. Pressing both simultaneously triggers a hardware reset.

---

## 🔍 Oscilloscope Mode

Samples the input signal on every **Timer 1** interrupt.
* **Time Scales:** From **1 (10 µs)** to **816 (8160 µs)** per point.
* **16 MHz Boost:** Scale 1 drops to **5 µs** (ATtiny85) or **6 µs** (ATmega328P).
* **Voltage Reference:** Uses internal **2.56V** (ATtiny85) or **1.1V** (ATmega328P). Input dividers must ensure the signal does not exceed vref for safety.

### On-Screen Information
The status bar displays:
`[Mag x1-3] [Voltage Range 1,3,5,C] [Auto/Manual Scale] [x4 Detail/Normal] [Trigger/Free] [µs/pt] [Frequency] [Vpp]`

---

## ⚡ Generator & Frequency Meter Modes

### Signal Generator
Generates square waves from **1 Hz to over 20 kHz**.
* Displays `=` for exact frequencies and `#` for approximations.
* **ATmega328P:** Uses a dedicated output pin (**D8/PB0**).

### Frequency Meter
Measures square wave logic levels (0 to VCC) up to **1 MHz**.
* **ATtiny85:** Uses internal counter **T0** (must compile without `millis()`).
* **ATmega328P:** Uses counter **T1** on pin **D5**.

---

## 🔌 Hardware & Construction

* **PCB:** Designs (approx. 5x5 cm) for 1 or 3 buttons are provided as `.jpg` files.
* **Power:** Designed for Li-ion batteries with a **TP4056** charger. 16 MHz operation requires a well-charged battery to meet processor tolerances.
* **Programming:** Features a bottom-side connector for **"Arduino as ISP"** programming, allowing the ATtiny85 to be soldered directly for a slimmer profile.

---

## 📝 Final Notes

This project aims for the absolute minimum component count. 
* **Battery Life:** Over **12 hours** with a 250 mA battery.
* **Software:** Zero external libraries. Only standard AVR headers included with the Arduino IDE are used.

### Contact & Credits
**Author:** Alejandro F. Fernández  
**Email:** [nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)  
**Year:** 2026

*I hope you enjoy this project as much as I enjoyed the two months of coding and design work.*
