# Operation details

👉 Versión en castellano [Funcionamiento_es.md](Funcionamiento_es.md) —
👉 Versão em português [Funcionamiento_br.md](Funcionamiento_br.md)

This document complements the [README](README_en.md): it explains how the device is used in practice — the selector, the accessory connector, the push buttons, and the calibration procedure — based on how the firmware actually implements them.

> The on-screen menu names shown in this document (in the walk-
> through examples) are the Spanish ones (default build,
> `IDIOMA_ES`), with the English meaning in parentheses. The
> firmware also builds in English (`#define IDIOMA_EN`) or
> Portuguese (`#define IDIOMA_BR`) near the top of the `.ino`.

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

### Selector table (factory defaults)

The selector pin is read against VCC as reference. These are the
nominal factory values (what `selValores[]` holds the first time
the device boots, before anything is calibrated):

| Resistor | Factory-default ADC reading | Function |
|---|---|---|
| Short to ground (0 Ω) | 151 | Oscilloscope **X** |
| 5.6 kΩ | 172 | Oscilloscope **Y** |
| 10 kΩ | 186 | Oscilloscope **Z** |
| 22 kΩ | 203 | Generator |
| 47 kΩ | 221 | Frequency counter |
| 150 kΩ | 241 | Temperature/humidity sensor |
| Open circuit (no resistor) | ≥250 | **No accessory connected** (waiting mode) |

### How the firmware detects which accessory is plugged in

Open circuit (≥250 counts) is checked first and always means "no
accessory", without comparing against anything else.

For everything else, the firmware does **not** use fixed windows
around a nominal value. Instead, it stores in EEPROM (`ee_r[0..5]`)
the ADC value measured the last time each of the 6 positions was
calibrated (factory-fresh, those 6 values are the ones in the table
above). When reading the selector, it scans the 6 calibrated
positions and picks whichever is **closest** to the current
reading. If the distance to the closest one is still more than 5
counts (`SEL_TOL`), the reading is treated as outside any known
position and reported as "Invalid selector".

In practice this means that if two calibrated positions ended up
too close to each other, whichever is objectively closer to the
reading wins — it's best to keep at least ~10-12 counts of
separation between each position's calibrated value so there's no
ambiguity.

## Power-on behavior

After the splash screen (name/version), the firmware reads the push buttons:

| At power-on | Result |
|---|---|
| No button held | Mode is determined by the selector (connected accessory) |
| **"+" (plus)** held | Shows "Reset" (Factory reset) with a "YES/NO" confirmation; if confirmed with "+", it erases the whole EEPROM back to factory defaults, and **then goes straight into CONFIG mode** |
| **"-" (minus)** held | Goes straight into CONFIG mode (calibration), without resetting anything |

## Using oscilloscope mode

Same navigation as CONFIG: hold a button (item changes every ~0.8 s) and release on the one you want to run:

- Holding **"+"**: `Autoset → <+1> → <+10> → <MAX> → <ZOOM> (Stretch) → Grid → <HOME> (Back) (then repeats).`
- Holding **"-"**: `Freeze → Free/Auto (Free/Triggered) → <-1> → <-10> → <MIN> → Vector/Dot → Slope → <HOME> (Back) (then repeats).`

Note: #define IDIOMA_EN / IDIOMA_BR / IDIOMA_ES

What each one does:

| On-screen | Meaning | Effect |
|---|---|---|
| `Autoset` | Autoscale | Toggles automatic time-scale adjustment based on the detected frequency. Enabling it forces triggered mode. |
| `<+1> / <+10>` | Up 1 / Up 10 | Manually increases the time scale (more time per division) by 1 or 10 valid steps. Turns off Autoset and stretch. |
| `<+1> / <+10>` | Down 1 / Down 10 | Same, but decreases the scale (less time per division, faster sweep). |
| `<MAX> / <MIN>` | Up max / Down min | Jumps straight to the slowest or fastest available scale. |
| `<ZOOM>` | Stretch | Toggles a ×4 zoom into the visible portion of the capture. |
| `Grid` | Grid | Shows or hides the reference lines (vertical every 20 px, horizontal at 0/25/50/75/100%). |
| `Vector/Dot` | Lines/Dots | Toggles between a continuous trace (lines) and just the sampled points. |
| `Slope` | Edge | Toggles the trigger between rising and falling edge. |
| `Free/Auto` | Free/Triggered | Toggles between free-running sweep (no wait) and triggered mode (waits for a crossing at the mid-level to sync the waveform). |
| `Freeze` | Capture | Freezes the screen (inverts video to show it's frozen) until any button is pressed; then resumes normally. |
| `<HOME>` | Back | Exits the menu without changing anything. |

All of these (except Freeze) are saved to EEPROM as soon as they're chosen, so they persist across power cycles. The time scale itself (which "+/- N" step it's on) is not saved: on restart it goes back to whatever the default/Autoescala leaves it at.

### Noise filtering in frequency detection

Before computing a frequency or locating the trigger point, the
firmware discards the capture if its peak-to-peak amplitude is below
`AMPLITUD_MINIMA_DETECCION` ADC counts (4 by default). This keeps a
bit of background noise, with nothing connected to the input, from
being read as a real high-frequency signal. When that happens, the
displayed frequency is "-----" and the sweep falls back to free-
running from the start of the capture, as if there were no signal.

### Status line (oscilloscope mode)

The bottom of the screen shows, from left to right:

- **Lupa (1/2/3)** ("magnification"): automatic magnification
  factor applied to the waveform when its amplitude is small
  relative to full scale.
- **A/M**: Autoset / Manual ("Auto-scale" / "Manual").
- **N/4**: Normal / X4 ("Normal" / "Stretched").
- **F / + / -**: trigger mode. 'F' = Free ("Free run"),
  '+' = rising-edge trigger, '-' = falling-edge trigger.
- **Time (number + u)**: actual time per division, in
  microseconds.
- **Frequency (number + H)**: detected frequency, one decimal
  place. Shows "-----" if below 0.5 Hz or no full cycle was
  detected.
- **Amplitude (number + %) or SAT**: peak amplitude as a
  percentage of full scale. Shows "SAT" if the signal exceeds
  full scale (saturated).
- **Range (X/Y/Z)**: selected input voltage range.

## Using generator mode

Same hold-and-release navigation:

- Holding **"+"**: `<+1> (Up 1) → <+10> (Up 10) → <+100> (Up 100) → <+1000> (Up 1000) → A 100 Hz → A 25 kHz → <HOME>` (Back) (then repeats).`
- Holding **"-"**: `<-1> (Down 1) → <-10> (Down 10) → <-100> (Down 100) → <-1000> (Down 1000) → A 1 kHz → A 10 kHz → <HOME>` (Back) (then repeats).`

`"+/- N"` (Up/Down N) adjusts the output frequency in steps of 1, 10, 100 or 1000 Hz; the `"A NN Hz"` (At NN Hz) options jump straight to a fixed frequency — these are already the same in both languages. The range runs from 1 Hz to 25 kHz (it clamps at those limits). After any change, the screen shows the requested frequency and the one actually achievable with the hardware (marked "=" if exact, or "#" if it's the closest approximation). `<Volver>` doesn't change anything, it just re-applies the current frequency.

## Using frequency-counter mode

The measured value is shown on screen in Hz, updating automatically every time the window completes.

## CONFIG mode (calibration)

There are only two physical buttons, used two different ways depending on context:

- **Navigating the menu**: hold a button down; every ~0.8 s the screen advances to the next item in a list; release when the desired item is shown, which runs it.
  - Holding **"+"**: `Cal 0 → Cal X → Cal Y → Cal Z → *(crystal-less builds only: `CAL 50Hz`)* → About → <HOME> (Back) (then repeats).`
  - Holding **"-"**: `"X 0/151"` → `"Y 5k6/172" → "Z 10k/186" → "G 22k/203" → "F 47k/221" → "S 150k/241" → <HOME> (Back) (then repeats).`
- **Confirm/abort** ("`<YES/NO>`" screen): "+" confirms (YES), "-" aborts (NO).

### Calibrating a voltage scale (Cal X / Cal Y / Cal Z)

1. Design the accessory's resistive divider so that, at the maximum voltage you want to measure, the divider's midpoint delivers a voltage slightly below the internal ADC reference used in oscilloscope mode:
   - **ATtiny85**: special 2.56 V reference (no external AREF capacitor needed, which frees up that pin); target no more than ~2.3 V — that's the minimum the manufacturer guarantees for this reference, given unit-to-unit manufacturing spread.
   - **ATmega328P**: 1.1 V reference (with an external AREF capacitor, since it has more spare pins); target no more than ~1 V (not the 2.3 V above, which is only for the ATtiny85) — that 1 V is the guaranteed minimum for this 1.1 V reference.
2. Apply that known maximum voltage to the input, with the matching accessory/selector already connected.
3. Enter CONFIG mode ("-" at power-on) and select Cal X, Cal Y, or Cal Z (hold "+" until it's shown, then release).
4. The screen shows "OLD" (Before, the stored value) and "NOW" (Now, the measured value). Confirm with "+" (YES) to save it as the new 100% for that scale, or "-" (NO) to discard it.
   - It's only saved if the reading falls within a reasonable range (neither saturated nor too low); otherwise it's rejected even if you confirm "YES".
5. Repeat for every scale in use. The divider doesn't need to be exact: calibration absorbs the actual tolerance of the resistors used.

**Example** for a 12 V accessory on ATtiny85 (targeting 2.3 V): required ratio ≈ 12/2.3 ≈ 5.2:1 — for instance R_top=42 kΩ and R_bottom=10 kΩ gives 12 V × 10/52 ≈ 2.31 V. The exact resistor values aren't critical, since step 4 calibrates against the actual applied voltage.

If you're measuring up to the reference voltage directly, no divider is needed: connect the signal straight to the input.

### Calibrating selector-resistor tolerance

If an accessory's selector doesn't land where it should (e.g. due
to the real resistor's manufacturing tolerance, or because you
swapped that resistor for a different value), it can be
recalibrated:

1. Connect the accessory with that selector resistor.
2. Enter CONFIG mode and, holding "-", select the matching option ("X 0/151" / "Y 5k6/172" / "Z 10k/186" / "G 22k/203" / "F 47k/221" / "S 150k/241").
3. "OLD"/"NOW" (Before/Now) is shown; confirming with "+" only saves
   the new value if it falls **strictly between 128 and 250 ADC
   counts** (this rejects clearly invalid readings, e.g. nearly
   shorted or nearly open-circuit). There's no fixed window around
   the expected nominal value: once saved, that reading becomes the
   new reference point for that position, and the firmware later
   detects it by proximity (see "How the firmware detects which
   accessory is plugged in" above).

Oscilloscope X doesn't appear in this list: since it uses a fixed short to ground by design, it has no manufacturing tolerance to calibrate.

### Calibrating the internal oscillator (crystal-less builds only)

The ATtiny85 can be built without an external crystal, using its
internal RC oscillator at 8 MHz (see [README_en.md](README_en.md),
"Crystal-less mode" section). The internal RC oscillator isn't
accurate out of the box, nor stable with temperature, so the
firmware offers calibration against a known reference frequency: the
mains line, at 50 Hz or 60 Hz depending on how it was built (`CAL50`
constant in the `.ino`: defined calibrates against 50 Hz, commented
out calibrates against 60 Hz).

This menu item (`CAL 50Hz` or `CAL 60Hz`, depending on the build)
**only appears** in crystal-less builds; a build with an external
crystal doesn't show it and doesn't need it.

**Before starting:**
- The device must be in oscilloscope mode with a known 50 or 60 Hz
  signal already connected to the input (e.g. the mains line
  frequency, taken through the accessory and resistive divider of
  one of the oscilloscope scales).

**Procedure:**
1. Enter CONFIG mode ("-" at power-on).
2. Hold "+" until you see "CAL 50Hz" (or "CAL 60Hz") and release.
3. Confirm with "+" (or abort with "-", which cancels without
   touching anything).
4. The device measures in 600 ms windows (a multiple of both 50 Hz
   and 60 Hz) and automatically scans every possible `OSCCAL` value
   (2 to 253), looking for the point where the measured frequency
   crosses the expected value (500 or 600, in tenths of a Hz,
   depending on `CAL50`).
5. If it finds a valid crossing, it saves the new `OSCCAL` to
   EEPROM and shows "DONE". If it doesn't find one (e.g. no real
   signal connected), it restores whatever `OSCCAL` it had before
   starting and shows "Abort", saving nothing.

The calibrated value stays in EEPROM and is only restored at power-
on, so it survives a power cycle. It's worth redoing this
calibration if the ambient temperature changes a lot, or if the
ATtiny85 gets reprogrammed (which can change its factory `OSCCAL`).

> This calibration is independent from the oscilloscope's time
> scales: `configurarEscala()` already has the relationship between
> scale steps and real microseconds tabulated for 8 MHz (crystal or
> no crystal). Calibrating `OSCCAL` doesn't change that table, only
> how fast the ATtiny85's internal clock actually runs.

### Restoring factory defaults

Holding "+" at power-on and confirming "YES" erases the entire EEPROM (all voltage and selector calibrations) and resets it to factory defaults, then goes straight into CONFIG mode so you can recalibrate.

## Temperature / humidity sensor

-  It auto-detects which of the two is connected (DHT/DS18B20).

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
