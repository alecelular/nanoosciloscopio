# Nano Oscilloscope with ATtiny85
![Prototipo](Fotos/prototipo.jpg)

Simple digital oscilloscope based on ATtiny85 with OLED display.

---

## Features
- Fast ADC sampling
- On-screen waveform display
- Built-in signal generator
- Time scale: 10 µs to 8160 µs per point
- Input ranges: 1 V, 3.3 V, 5 V or 12 V (selectable)
- Can display signals from 1 Hz up to approximately 10 kHz
- Positive signals only
- Initial public version (features will be improved over time)

---

## Required Hardware
- ATtiny85 (main), adaptable to ATmega328P
- OLED SSD1306

### Components
- R1 10 kΩ
- R2 10 kΩ
- R3 10 kΩ
- R4 100 kΩ
- R5 47 kΩ   (Button 1)
- R6 4.7 kΩ
- R7 12 kΩ
- R8 47 kΩ
- R9 8.2 kΩ
- R10 6.8 kΩ (Button 2, only for 3-button version)
- R11 22 kΩ  (Button 3, only for 3-button version)
- C1 15 pF
- C2 15 pF
- C3 10 µF
- C4 100 nF
- DS1, DS2 Schottky diodes
- DZ1 3.3 V Zener diode
- Connectors
- Push buttons

Optional:
- Power switch
- PCB
- TP4056 charger module
- Lithium battery (3.7 V)
- Crystal oscillator  
  (8 MHz works well with battery, 16 MHz requires 5 V but may work with a fully charged battery)

---

## Connections
see ![PDF](hardware/Esquematico_Nano-Osciloscopio_2026-04-01.png) and ![Details](Funcionamiento_es)

---

## How to use

1. Upload the code using Arduino IDE in a folder named `NOS_V1.5.0` (files: NOS_V1.5.0.ino and I2C.ino)

2. Configure:
   - Number of buttons (1, 2 or 3)
   - OLED type (128x64 or 128x32)

3. Compiler options are defined inside the source files

4. Optional:
   - You can use an ATmega328P (Arduino, Nano, Pro Mini) for testing

5. Calibration:
   - With crystal: use input voltage calibration
   - Without crystal: calibrate frequency using a 50 Hz or 60 Hz signal

6. Connect the input signal (1 V, 3.3/5 V or 12 V depending on configuration)

7. Adjust parameters

---

## Limitations
- Limited bandwidth
- ADC resolution

---

## Author
Alejandro F. Fernández  
nanoosciloscopio@gmail.com

---

## License
Non-commercial use only.

For commercial use, contact:
nanoosciloscopio@gmail.com

Feedback and improvements are welcome.

## Support the project

If you find this project useful, you can support it here:

[![Buy me a coffee](https://cdn.cafecito.app/img/buttons/button_1.svg)](https://cafecito.app/rsp148)
