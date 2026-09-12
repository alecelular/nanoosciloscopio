# Operation details

This document complements the [README](README_en.md): it explains how the device is used in practice — the selector, the accessory connector, the push buttons, and the calibration procedure — based on how the firmware actually implements them.

## Selector, input connector and accessories

The "selector" is not a separate component: it's the analog reading of a single shared pin:

- **ATtiny85**: pin 1 (RESET/PB5/ADC0).
- **ATmega328P**: A1 (PC1).

That pin is read between measurements (or when one finishes) to determine which accessory is plugged in, based on the resistance the accessory places between that pin and ground.

The signal input (or generator output) uses a cheap USB-type connector — not because it follows the USB standard, but because it's easy to source and provides the 4 terminals needed:

- 2 power terminals (one is VCC, also available to power an external temperature/humidity sensor).
- 1 selector terminal.
- 1 signal terminal: oscilloscope input, generator output, frequency-counter input, or sensor data, depending on the accessory.

Each accessory externally carries the selector resistor for its function and, if it's an oscilloscope accessory, the resistive divider for that scale on the signal line. Changing range or function is just a matter of swapping accessories.

### Selector table (real firmware values)

The selector pin is read against VCC as reference; the firmware compares the reading (0-255) against these windows (±7 counts tolerance):

| Resistor | Nominal ADC reading | Function |
|---|---|---|
| Short to ground (0 Ω) | ~144-158 | Oscilloscope **Y** |
| 3.9 kΩ | ~160-174 (167) | Oscilloscope **Z** |
| 12 kΩ | ~181-195 (188) | Generator |
| 33 kΩ | ~207-221 (214) | Frequency counter |
| 100 kΩ | ~229-243 (236) | Temperature/humidity sensor |
| No resistor (open circuit) | ≥245 | Oscilloscope **X** |
| Any other value (gaps between windows) | — | "INVALID" (flags a wrong resistor) |

X and Y don't need a "calibratable" selector resistor: they're the two natural extremes (open and shorted). Z, generator, frequency counter and sensor do use a real resistor, and that resistor can be recalibrated (see below) to compensate for manufacturing tolerance.

## Power-on behavior

After the splash screen (name/version), the firmware reads the push buttons:

| At power-on | Result |
|---|---|
| No button held | Mode is determined by the selector (connected accessory) |
| **"+" (plus)** held | Shows "A nuevo" (Factory reset) with a "SI/NO" (YES/NO) confirmation; if confirmed with "+", it erases the whole EEPROM back to factory defaults, and **then goes straight into CONFIG mode** |
| **"-" (minus)** held | Goes straight into CONFIG mode (calibration), without resetting anything |

## Using oscilloscope mode

Same navigation as CONFIG: hold a button (item changes every ~0.8 s) and release on the one you want to run:

- Holding **"+"**: Autoescala (Autoscale) → Sube 1 (Up 1) → Sube 10 (Up 10) → Sube toda (Up max) → Estirar (Stretch) → Grilla (Grid) → `<Volver>` (Back) (then repeats).
- Holding **"-"**: Captura (Capture) → Libre/Auto (Free/Triggered) → Baja 1 (Down 1) → Baja 10 (Down 10) → Baja toda (Down min) → Lineas o Puntos (Lines/Dots) → Flanco (Edge) → `<Volver>` (Back) (then repeats).

Note: the firmware's on-screen text is only in Spanish — there's no built-in translation switch. The English words above are just this document's explanation of what each Spanish label means, not what actually appears on the display. Anyone is welcome to adapt the source to add translated labels, as long as it still fits within the ATtiny85's 8 KB of Flash.

What each one does:

| On-screen (Spanish) | Meaning | Effect |
|---|---|---|
| Autoescala | Autoscale | Toggles automatic time-scale adjustment based on the detected frequency. Enabling it forces triggered mode. |
| Sube 1 / Sube 10 | Up 1 / Up 10 | Manually increases the time scale (more time per division) by 1 or 10 valid steps. Turns off Autoescala and Estirar. |
| Baja 1 / Baja 10 | Down 1 / Down 10 | Same, but decreases the scale (less time per division, faster sweep). |
| Sube toda / Baja toda | Up max / Down min | Jumps straight to the slowest or fastest available scale. |
| Estirar | Stretch | Toggles a ×4 zoom into the visible portion of the capture. |
| Grilla | Grid | Shows or hides the reference lines (vertical every 20 px, horizontal at 0/25/50/75/100%). |
| Lineas o Puntos | Lines/Dots | Toggles between a continuous trace (lines) and just the sampled points. |
| Flanco | Edge | Toggles the trigger between rising and falling edge. |
| Libre/Auto | Free/Triggered | Toggles between free-running sweep (no wait) and triggered mode (waits for a crossing at the mid-level to sync the waveform). |
| Captura | Capture | Freezes the screen (inverts video to show it's frozen) until any button is pressed; then resumes normally. |
| `<Volver>` | Back | Exits the menu without changing anything. |

All of these (except Captura) are saved to EEPROM as soon as they're chosen, so they persist across power cycles. The time scale itself (which "Sube/Baja" step it's on) is not saved: on restart it goes back to whatever the default/Autoescala leaves it at.

## Using generator mode

Same hold-and-release navigation:

- Holding **"+"**: Sube 1 (Up 1) → Sube 10 (Up 10) → Sube 100 (Up 100) → Sube 1000 (Up 1000) → A 50 Hz → A 100 Hz → A 25 kHz → `<Volver>` (Back) (then repeats).
- Holding **"-"**: Baja 1 (Down 1) → Baja 10 (Down 10) → Baja 100 (Down 100) → Baja 1000 (Down 1000) → A 500 Hz → A 1 kHz → A 10 kHz → `<Volver>` (Back) (then repeats).

"Sube/Baja N" (Up/Down N) adjusts the output frequency in steps of 1, 10, 100 or 1000 Hz; the "A NN Hz" (At NN Hz) options jump straight to a fixed frequency — these are already the same in both languages. The range runs from 1 Hz to 25 kHz (it clamps at those limits). After any change, the screen shows the requested frequency and the one actually achievable with the hardware (marked "=" if exact, or "#" if it's the closest approximation). `<Volver>` doesn't change anything, it just re-applies the current frequency.

## Using frequency-counter mode

Only two options, one per block:

- Holding **"+"**: "Frec 1s" (1s window) → `<Volver>` (Back).
- Holding **"-"**: "Frec 0,2s" (0.2s window) → `<Volver>` (Back).

This picks the measurement gate time: 1 second gives more resolution (better for low-frequency signals), 0.2 seconds updates faster but with less resolution. The measured value is shown on screen in Hz, updating automatically every time the chosen window completes.

## CONFIG mode (calibration)

There are only two physical buttons, used two different ways depending on context:

- **Navigating the menu**: hold a button down; every ~0.8 s the screen advances to the next item in a list; release when the desired item is shown, which runs it.
  - Holding **"+"**: Cal 0 → Cal X → Cal Y → Cal Z → `<Volver>` (Back) (then repeats).
  - Holding **"-"**: "Z 3k9/167" → "G 12k/188" → "F 33k/214" → "S100k/236" → AUTOR (About) → `<Volver>` (Back) (then repeats).
- **Confirm/abort** ("`<SI/NO>`" screen — Spanish for YES/NO): "+" confirms (SI), "-" aborts (NO).

### Calibrating a voltage scale (Cal X / Cal Y / Cal Z)

1. Design the accessory's resistive divider so that, at the maximum voltage you want to measure, the divider's midpoint delivers a voltage slightly below the internal ADC reference used in oscilloscope mode:
   - **ATtiny85**: special 2.56 V reference (no external AREF capacitor needed, which frees up that pin); target no more than ~2.3 V — that's the minimum the manufacturer guarantees for this reference, given unit-to-unit manufacturing spread.
   - **ATmega328P**: 1.1 V reference (with an external AREF capacitor, since it has more spare pins); target no more than ~1 V (not the 2.3 V above, which is only for the ATtiny85) — that 1 V is the guaranteed minimum for this 1.1 V reference.
2. Apply that known maximum voltage to the input, with the matching accessory/selector already connected.
3. Enter CONFIG mode ("-" at power-on) and select Cal X, Cal Y, or Cal Z (hold "+" until it's shown, then release).
4. The screen shows "Antes" (Before, the stored value) and "Ahora" (Now, the measured value). Confirm with "+" (SI) to save it as the new 100% for that scale, or "-" (NO) to discard it.
   - It's only saved if the reading falls within a reasonable range (neither saturated nor too low); otherwise it's rejected even if you confirm "SI".
5. Repeat for every scale in use. The divider doesn't need to be exact: calibration absorbs the actual tolerance of the resistors used.

**Example** for a 12 V accessory on ATtiny85 (targeting 2.3 V): required ratio ≈ 12/2.3 ≈ 5.2:1 — for instance R_top=42 kΩ and R_bottom=10 kΩ gives 12 V × 10/52 ≈ 2.31 V. The exact resistor values aren't critical, since step 4 calibrates against the actual applied voltage.

If you're measuring up to the reference voltage directly, no divider is needed: connect the signal straight to the input.

### Calibrating selector-resistor tolerance (Cal Sz/Sg/Sf/Ss)

If an accessory's selector doesn't fall inside the expected window from the table above (e.g., due to the real resistor's manufacturing tolerance), it can be recalibrated:

1. Connect the accessory with that selector resistor.
2. Enter CONFIG mode and, holding "-", select the matching option ("Z 3k9/167" / "G 12k/188" / "F 33k/214" / "S100k/236").
3. "Antes"/"Ahora" (Before/Now) is shown; confirming with "+" only saves the new value if it falls within ±7 counts of the expected nominal (167/188/214/236) — this prevents accepting a clearly wrong resistor.

### Restoring factory defaults

Holding "+" at power-on and confirming "SI" erases the entire EEPROM (all voltage and selector calibrations) and resets it to factory defaults, then goes straight into CONFIG mode so you can recalibrate.

## Temperature / humidity sensor

- **ATtiny85**: only one model (DHT or DS18B20) can be chosen, **at compile time** (define or comment out `SENSOR_DHT`), due to code-size limits.
- **ATmega328P**: having more room, it auto-detects which of the two is connected, without recompiling.

That's also why the USB-type connector brings out VCC: to power that external sensor.

## Reprogramming connector (ATtiny85)

The board includes a connector to reprogram the ATtiny85 without desoldering it (firmware updates), or to repurpose it for something else. It follows the standard ISP programming pinout (e.g., with "Arduino as ISP"):

| ISP signal | ATtiny85 pin |
|---|---|
| RESET | pin 1 |
| VCC | pin 8 |
| SCK | pin 7 (PB2) |
| MISO | pin 6 (PB1) |
| MOSI | pin 5 (PB0) |
| GND | pin 4 |
