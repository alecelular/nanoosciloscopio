# Nanooscilloscope with ATtiny85

Compact digital oscilloscope based on ATtiny85, with OLED display, integrated signal generator and frequency counter, designed to work with a minimal number of components.

👉 Versión en castellano [README.md](README.md)

👉 English version below

---

![Prototype](Fotos/prototipo.jpg)

## Specifications

- MCU: ATtiny85 (main), adaptable to ATmega328P  
- Display: OLED SSD1306 (128x64 / 128x32)  
- Time per point: 10 µs to 8160 µs  
- Maximum usable frequency: ~10 kHz  
- Resolution: 8 bits (internal ADC, effective resolution lower)  
- Input ranges: 1V, 3.3V, 5V, 12V  
- Signal type: positive values only  
- Functions:
  - Oscilloscope
  - Signal generator
  - Frequency counter

## Features
- Fast sampling using ADC.
- On-screen visualization.
- Integrated signal generator.
- Time scale: 10 µs to 8160 µs per point.
- Measures signal amplitudes of 1V, 3.3V, 5V or 12V, depending on selection.
- Can display signals from 1 Hz up to approximately 10 kHz.
- Positive signals only.

## Required Hardware
- ATtiny85 (main), adaptable to ATmega328P (preliminary support for ATtiny84).
- OLED SSD1306.

### Components
- R1 10 kΩ
- R2 10 kΩ
- R3 10 kΩ
- R4 100 kΩ
- R5 47 kΩ   (PUL1)
- R6 10 kΩ
- R7 12 kΩ
- R8 47 kΩ
- R9 0 Ω (jumper)
- R10 6.8 kΩ (PUL2) Only for 3-button version
- R11 22 kΩ  (PUL3) Only for 3-button version
- C1 15 pF
- C2 15 pF 
- C3 10 µF
- C4 100 nF
- DS1=DS2 Schottky 1N5819
- DZ1 Zener 5.6 V (or higher if powered with VCC > 5V)
- Connectors
- Push buttons

Optional:
- Switch
- PCB
- TP4056 charging module
- 3.7 V lithium battery
- Crystal (improves accuracy)

## Connections and operation
See [Schematic](hardware/Esquema_Nano-Osciloscopio.pdf) and [Operation details](Funcionamiento_en.md)

## How to use

### Compilation
1. Load the code into the Arduino IDE in folder `NOS_V1.6.0` (files `NOS_V1.6.0.ino` and `I2C.ino`).
2. Use the options indicated in `NOS_V1.6.0.ino` and `I2C.ino`.
3. Compile selecting:
   - Number of buttons
   - OLED type (128x64 = 8, 128x32 = 4)
4. Compiler options are defined inside the source files.
5. An ATmega328P can be used for testing (Arduino, Nano, Pro Mini), although no PCB is provided for it.
6. If using a crystal, calibrate input voltages in the CONFIG menu.
7. If not using a crystal, also calibrate frequency using a 50 Hz or 60 Hz signal.
8. Connect the input signal (supported ranges: 1V, 3.3/5V or 12V depending on configuration).
9. Adjust parameters.

### Startup
1. An ATmega328P can be used for testing (Arduino/Nano/Pro Mini).
2. If using a crystal: calibrate voltages in the CONFIG menu.
3. If not using a crystal: also calibrate frequency (50 Hz or 60 Hz).
4. Connect the input signal according to the selected range.
5. Adjust parameters from the device.

## Limitations
- Limited bandwidth (~10 kHz).
- Effective resolution lower than 8 bits.
- Positive signals only.
- No acquisition memory.
- Basic trigger.
- Accuracy depends on crystal or calibration when using internal oscillator.

## Author
Alejandro F. Fernández  
nanoosciloscopio@gmail.com

## License
Non-commercial use.

If you want to use it commercially, contact:
nanoosciloscopio@gmail.com

Feedback, improvements and bug reports are welcome.

## Support the project

If you found this useful, you can buy me a coffee:
[![Buy me a coffee](https://cdn.cafecito.app/img/buttons/button_1.svg)](https://cafecito.app/rsp148)
