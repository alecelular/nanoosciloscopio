# Nanooscilloscope with ATtiny85

Compact digital oscilloscope based on ATtiny85, with OLED display,
integrated signal generator, frequency counter and temperature
sensor mode, designed to work with a minimal number of components
(only 5 I/O pins on the ATtiny85). It also compiles, without
changing the program logic, for **ATmega328P** (Arduino Nano / Pro
Mini).

👉 Versión en castellano [README.md](README.md)

👉 English version below

> **Note:** this repository replaces the previous prototype of this
> same project. The current version (**V1.9.5**) has its own PCB
> (**NOS41**), a different push-button layout than the original
> prototype, and adds the temperature sensor mode. Some details of
> the previous design (PCB, number and type of push-buttons,
> photos) no longer apply as-is — only what's still valid for this
> version is documented here.

Assembly details:
[![NOS41 prototype with battery](Fotos/ATtiny85/IMG_20260527_131120.jpg)](Fotos/ATtiny85/IMG_20260527_131120.jpg)

Prototype with Arduino Nano and 128x32 display
[![Arduino nano](Fotos/Arduino_NANO_128x32/IMG_20260410_170244.jpg)](Fotos/Arduino_NANO_128x32/IMG_20260410_170244.jpg)

Assembled unit displaying the 50 Hz line frequency.
Note the battery charging port, the input connector, and the side push-buttons.
[![NOS41 50 Hz](Fotos/50Hz.png)](Fotos/50Hz.png)

---

## Specifications

- **MCU:** ATtiny85 (main), adaptable to ATmega328P.
- **Display:** OLED SSD1306 (128x64 / 128x32) via bit-banged I2C
  (no `Wire.h`, no graphics library).
- **Time base:** adjustable, specifically calibrated for 8, 12, 16
  and 20 MHz crystals (with phase correction for the ones that
  don't divide evenly into whole microseconds).
- **Resolution:** 8 bits (internal AVR ADC).
- **Mode selection:** a single ADC pin with a resistive selector —
  no dedicated pin per function.
- **Built-in functions:**
  * Oscilloscope (autoscale, edge trigger, line or dot mode, free
    or triggered sweep, x4 "stretch", freeze capture).
  * Signal generator (square wave, 1 Hz to 25 kHz).
  * Frequency counter (up to ~1 MHz on square signals).
  * Temperature sensor mode (DS18B20 or DHT11/DHT22, configurable).
- Full on-screen menu for calibration, no PC required.

---

## Required hardware

- ATtiny85 (main), adaptable to ATmega328P. Requires an **external
  crystal** for accurate time/frequency measurements (does not work
  with the internal RC oscillator).
- OLED SSD1306 (128x64 or 128x32, configurable via `#define`).
- Own PCB design: **NOS41** — see [`hardware/`](hardware/) (source
  in `nos41.xcf`).
- 2 push-buttons.
- Optional: TP4056-type charging module + 3.7 V lithium battery,
  power switch.

### Pins — ATtiny85

| Pin | Function             | Signal       |
|-----|-----------------------|--------------|
| 1   | RESET / Selector      | PB5/ADC0     |
| 2   | Crystal A              | PB3/ADC3     |
| 3   | Crystal B              | PB4/ADC2     |
| 4   | GROUND                | 0 V          |
| 5   | SDA' (I2C bit-bang)    | PB0/AIN0     |
| 6   | SCL' (I2C bit-bang)    | PB1/AIN1     |
| 7   | Out/In/Freq/Sensor     | PB2/ADC1/T0  |
| 8   | Power supply           | VCC          |

### Pins — ATmega328P (Arduino Nano, tested)

| Function          | Pin   | Signal |
|-------------------|-------|--------|
| Out/In/Sensor      | A0    | PC0    |
| Frequency counter  | D5    | PD5    |
| SDA' (bit-bang)    | D9    | PB1    |
| SCL' (bit-bang)    | D10   | PB2    |
| Push-button 1      | D2    | PD2    |
| Push-button 2      | D7    | PD7    |
| Selector           | A1    | PC1    |
| AREF capacitor     | AREF  | VREF   |

> ⚠️ **Correction relative to earlier documentation:** on the
> ATmega328P, the **signal generator output uses the same A0/PC0
> pin** as the oscilloscope input and the sensor — not an
> independent D8/PB0 pin as described at some point previously. See
> `activarGenerador()` in the code: for ATmega328P it toggles
> `PINC=(1<<PC0)` (configured as output with `DDRC|=(1<<PC0)`), and
> only the ATtiny85 uses a dedicated pin (PB2), being the only one
> available besides the selector and the I2C bus.

Full schematics in [`hardware/`](hardware/):
- `Esquema_Nano-Osciloscopio_ATtiny85.pdf` / `.json`
- `Esquema_Nano-Osciloscopio_ATmega328P.pdf` / `.json`
- `nos41.xcf` — NOS41 PCB design.

### Mode selector (a single resistor does it all)

| Resistance  | Approx. ADC | Mode              |
|-------------|------------:|-------------------|
| Open (∞)    | ~245–255    | Oscilloscope 'X'  |
| —           | ~144–158    | Oscilloscope 'Y'  |
| 3.9 kΩ      | ~167        | Oscilloscope 'Z'  |
| 12 kΩ       | ~188        | Generator         |
| 33 kΩ       | ~214        | Frequency counter |
| 100 kΩ      | ~236        | Sensor            |

Values are stored in EEPROM and can be recalibrated from the menu
itself, to compensate for the real tolerance of the resistors used.

---

## 📷 Prototype evolution

The photos in [`Fotos/ATtiny85/`](Fotos/ATtiny85/) document three
stages of development:

1. **Etched NOS41 board** (05/14) — the freshly made PCB, still
   without components.
2. **Earlier protoboard prototype** (05/15) — an earlier build on
   perforated board, used to test the logic before moving to the
   final PCB.
3. **Assembled NOS41** (05/27 onward) — the final board with all
   components soldered, OLED and battery.

[`Fotos/Arduino_NANO_128x32/`](Fotos/Arduino_NANO_128x32/) also has
photos of the test build on an Arduino Nano with a 128x32 OLED.

---

## ⚡ Signal Generator section

Generates square waves between **1 Hz and over 20 kHz**.
* **How it works:** Reconfigures the Timer and uses the input pin
  as output.
* **Accuracy:** If the frequency is exact, the `=` symbol is shown.
  If it's an approximation, `#` is shown instead.
* **ATmega328P:** The output is generated on the **same input/
  sensor pin (A0/PC0)** — not on an independent pin — so as not to
  add a dedicated pin beyond the ones already used by the
  oscilloscope and sensor.

## 📈 Frequency Counter section

Measures the frequency of square waves (0 to VCC logic level)
injected into the input pin, easily reaching **1 MHz**.
* **ATtiny85:** Uses the internal **T0** counter. Compiling without
  `millis()` is essential to avoid conflicts with the time-base
  counter.
* **ATmega328P:** Uses the **T1** counter on pin **D5**, which lets
  the measurement input be separate from the frequency input.
* *Note:* For non-square signals below 10 kHz, using
  **Oscilloscope Mode** is recommended.

---

## 🚀 Introduction

The documentation and code are mainly focused on the **ATtiny85**.
However, the system has been adapted to be compatible with the
**ATmega328P**.

> **Note:** A tentative description exists for using an
> **ATtiny84**, although no specific code adapted for this model is
> included at this time.

### Software requirements

To compile the code for the **ATtiny85**, **Arduino IDE 1.8.19**
was used, with the following indispensable settings:
* **Core:** [ATtinyCore 1.4.1 / 1.5.2](http://drazzy.com/package_drazzy.com_index.json)
* **Compile options:**
    * `No millis()` (Mandatory to maximize Flash and avoid
      conflicts with the counters used by the frequency counter
      and generator).
    * `LTO enabled`.
    * `No bootloader`.

For **ATmega328P**, simply select that board (Arduino Nano / Pro
Mini) in the IDE — the same `.ino` detects the MCU via
`#if defined(__AVR_ATtiny85__)` and automatically adjusts pins,
timers and peripherals.

### Crystal recommendation ATtiny85

- **8 MHz**: recommended when powering the device with a battery (lower power consumption).
- **16 MHz**: recommended when powering it with an external 5V source (higher precision/speed).

### Getting started

1. Load the code from [`src/`](src/) (`NOS_V1.9.5.ino` +
   `I2C.ino`).
2. Program the ATtiny85 via ISP (see the ISP pin table at the top
   of the `.ino`), or upload directly if using an Arduino Nano/Pro
   Mini.
3. If using an external crystal: calibrate input voltages from the
   configuration menu.
4. Also calibrate the frequency using a 50 Hz or 60 Hz reference
   signal if needed.
5. Connect the signal to measure and adjust parameters from the
   device itself.

---

## Repository structure

```
.
├── src/
│   ├── NOS_V1.9.5.ino   # Main program: ADC, timers, menus, modes
│   └── I2C.ino          # Software I2C driver (bit-banging) for the OLED
├── hardware/
│   ├── Esquema_Nano-Osciloscopio_ATtiny85.pdf/.json
│   ├── Esquema_Nano-Osciloscopio_ATmega328P.pdf/.json
│   └── nos41.xcf         # Graphic source (GIMP) related to the design
├── Fotos/
│   ├── ATtiny85/              # Assembled NOS41 prototype
│   └── Arduino_NANO_128x32/   # Test prototype on Arduino Nano
├── README.md
├── README_en.md
├── LICENSE_es
└── LICENSE_en
```

---

## Known limitations

- Requires an external crystal: reliable time calibration is not
  possible with the internal RC oscillator.
- The frequency counter at 20 MHz is functionally correct but
  somewhat less precise than at other crystal frequencies.
- Basic trigger, no extended acquisition memory.
- ATmega328P support tested less extensively than ATtiny85.

## Contributions

Suggestions, fixes and hardware variants are welcome via issues or
pull requests. Feedback on improvements or bugs is appreciated.

## Author

Alejandro F. Fernández (alecelular)
[nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)

## License

Non-commercial use — see [`LICENSE_es`](LICENSE_es) /
[`LICENSE_en`](LICENSE_en).

If you want to use it commercially, contact:
[nanoosciloscopio@gmail.com](mailto:nanoosciloscopio@gmail.com)

## Support the project

If you found this useful, you can buy me a coffee:
[![Buy me a coffee](https://cdn.cafecito.app/img/buttons/button_1.svg)](https://cafecito.app/rsp148)

---

*I hope you enjoy this project as much as I enjoyed developing it.*
