# Marlin bugfix-2.1.x — Artillery Genius Pro

Custom Marlin firmware for the **Artillery Genius Pro** (STM32F401RCT6, BOARD_ARTILLERY_RUBY).
Hardware-verified configuration based on Marlin bugfix-2.1.x.

**→ [Download pre-built firmware (v9)](releases/v9/firmware-gpro-v9-0x08000000.bin)**
**→ [Flash instructions & build guide](releases/v9/README.md)**
**→ [Recommended: upgrade your TFT screen firmware](releases/tft/README.md)**

---

## Feature Comparison

Full comparison against stock Marlin 2.1.x, the gpro-mp reference, and Marlin-for-artillery-genius-pro-bugfix-2.1.x.

### Hardware / Board

| Feature | Stock Marlin 2.1.x | gpro-mp | mfagp | Our v9 |
|---------|-------------------|---------|-------|--------|
| Board | generic | `BOARD_ARTILLERY_RUBY` | `BOARD_ARTILLERY_RUBY` | `BOARD_ARTILLERY_RUBY` |
| Dual serial (USB + TFT UART) | 1 port | 2 ports | 2 ports | 2 ports |
| TFT baud rate | — | 250000 | 115200 | 250000 |
| BLTouch | off | on | on | on |
| BLTouch dual-pin wiring | — | yes (Z_MIN_PROBE=PC2, Z_MIN=PA0) | yes | yes — authoritative |
| Probe hit state | — | HIGH | HIGH | HIGH |
| Z endstop hit state | — | LOW (NC) | LOW (NC) | LOW (NC) |
| Calibrated steps/unit | generic | {80.121, 80.121, 402, 449.5} | generic defaults | same as gpro-mp |
| Calibrated PID hotend | generic | hardware-tuned | generic defaults | hardware-tuned |
| Calibrated PID bed | generic | hardware-tuned | generic defaults | hardware-tuned |
| Controller fan auto-management | no | no | yes | no |
| NeoPixel RGB LED | no | no | yes | no |

### G-code Commands

| G-code | Purpose | Stock | gpro-mp | mfagp | Our v9 |
|--------|---------|-------|---------|-------|--------|
| M92 | Set steps-per-unit at runtime | no | **no** | yes | yes |
| M113 | Host keepalive interval | no | no | yes | yes |
| M114 D | Detailed position | no | no | yes | yes |
| M115 | Firmware capabilities report | partial | yes | yes | yes |
| M43 | Pin debug / toggle | no | yes | **no** | yes |
| M154 | Auto-report position | no | no | **no** | yes |
| M155 auto on boot | Temps pushed to TFT on boot | no | no | no | **yes** |
| M211 | Software endstops toggle | yes | **no** | yes | yes |
| M290 | Babystepping | no | no | yes | yes |
| M486 | Cancel specific objects | no | no | yes | yes |
| M493 | FT Motion config (shaper, freq, zeta) | no | no | no | **yes** |
| M593 | Input shaping config | no | no | yes | yes |
| M600 | Filament change | no | partial | yes | yes |
| M701/M702 | Load/Unload filament | no | no | no | yes |
| M73 | Print progress + remaining time | no | no | yes | no |
| M810-M819 | G-code macros | no | no | **no** | yes |
| M876 | Host prompt response | no | no | yes | yes |
| M900 | Linear Advance K-factor | no | no | yes | yes |
| M48 | Probe repeatability test | no | no | yes | yes |

### Bed Leveling

| Feature | Stock | gpro-mp | mfagp | Our v9 |
|---------|-------|---------|-------|--------|
| Leveling method | bilinear | UBL | UBL | UBL |
| Probe repetitions | 1 | 3 + 1 extra | 3 + 1 extra | 3 + 1 extra |
| Restore leveling after G28 | no | yes | yes | yes |
| Segment-leveled moves | no | no | no | **yes** (5 mm) |
| Assisted tramming (G35) | no | yes | **no** | yes |

### Motion / Print Quality

| Feature | Stock | gpro-mp | mfagp | Our v9 |
|---------|-------|---------|-------|--------|
| S-curve acceleration | no | yes | yes | yes |
| Junction Deviation | no | yes | yes | yes |
| Linear Advance | no | no | yes (K unset) | **yes** (K=0.13) |
| Input Shaping X+Y | no | no | yes | yes |
| Input Shaping LCD menu | no | no | yes | yes |
| FT Motion (M493) | no | no | no | **yes** |
| FT Motion shapers compiled | — | — | — | ZV, ZVD, MZV |
| FT Motion default shaper | — | — | — | ZV (X=55 Hz, Y=48.6 Hz) |
| Adaptive step smoothing | no | no | yes | yes |
| Babystepping (always available) | no | no | yes | yes |
| Arc support (G2/G3) | no | no | yes | yes |
| AUTOTEMP | no | no | yes | yes |
| SLOWDOWN (buffer protection) | no | no | yes | yes |
| QUICK_HOME (simultaneous X+Y) | no | no | yes | yes |
| VALIDATE_HOMING_ENDSTOPS | no | no | yes | yes |
| Software endstops (M211) | yes | **no** | yes | yes |
| Stepper idle timeout | no | no | yes (120 s) | yes (120 s) |
| MULTISTEPPING_LIMIT | 128 | 16 | 16 | 16 |

### Safety

| Feature | Stock | gpro-mp | mfagp | Our v9 |
|---------|-------|---------|-------|--------|
| Power loss recovery | no | yes | **no** | yes |
| Thermal protection hotend | yes | yes | yes | yes |
| Thermal protection bed | yes | yes | yes | yes |
| Hardware watchdog | no | no | yes | yes |
| Detect broken endstop | no | no | yes | yes |
| Cold extrusion prevention | yes | yes | yes | yes |

### Serial / TFT Communication

| Feature | Stock | gpro-mp | mfagp | Our v9 |
|---------|-------|---------|-------|--------|
| BUFSIZE (command queue) | 4 | 32 | 32 | 32 |
| TX_BUFFER_SIZE | 0 | 128 | 128 | 128 |
| RX_BUFFER_SIZE | 128 | 1024 | 1024 | 1024 |
| XON/XOFF flow control | no | yes | **no** | yes |
| Serial overrun protection | no | no | yes | yes |
| Faster G-code parser | no | no | yes | yes |
| ADVANCED_OK | no | yes | yes | yes |
| Emergency parser | no | no | yes | yes |
| Host action commands | no | no | yes | yes |
| Host prompt support | no | no | yes | yes |
| Host keepalive (M113) | no | no | yes | yes |
| Busy-while-heating | no | no | yes | yes |
| Auto-report temperatures | yes | yes | yes | yes |
| Auto-report position (M154) | no | no | **no** | yes |
| M73 progress reporting | no | no | yes | yes |
| Startup auto-temp push | no | no | **no** | **yes** |
| Capabilities report (M115) | partial | yes | yes | yes |
| Extended capabilities report | no | no | yes | yes |

### SD Card / Storage

| Feature | Stock | gpro-mp | mfagp | Our v9 |
|---------|-------|---------|-------|--------|
| Long filename support | no | no | yes | yes |
| Auto-report SD status (M27) | no | yes | yes | yes |
| SD block retry on error | no | no | no | **yes** |
| Most-recent files first | no | no | yes | yes |
| SD abort G-code (G28XY) | no | no | yes | yes |
| Cancel objects (M486) | no | no | yes | yes |
| G-code macros (M810-M819) | no | no | **no** | yes |

---

## Filament Runout Sensor — Wiring Required

> **This requires a wiring change.** Without it the filament runout sensor will not work, even though it is enabled in the firmware.

The Genius Pro filament sensor must be connected to the **Z endstop connector on the right side of the mainboard**, not to the stock connector on the left.

**Why:** the right-side connector maps to pin **PA0** (the original Z-MIN endstop input). Once BLTouch is installed, Z homing is handled by the BLTouch probe on PC2, so PA0 is freed and repurposed here. The left-side Z endstop connector is a different pin and is unused in this build.

**What to do:**

1. Locate the two Z endstop connectors on the Artillery Ruby mainboard — one on the left side, one on the right side.
2. Move the filament sensor cable to the **right-side connector**.
3. Leave the stock Z endstop cable (left-side connector) disconnected.

When filament runs out, the printer pauses and triggers `M600` (filament change). The sensor uses an internal pullup: LOW = no filament, HIGH = filament present. The runout distance and enable/disable can be controlled at runtime with `M412`.

---

## Firmware Download

| File | Description |
|------|-------------|
| [firmware-gpro-v9-0x08000000.bin](releases/v9/firmware-gpro-v9-0x08000000.bin) | Pre-built binary — flash at `0x08000000` |
| [releases/v9/README.md](releases/v9/README.md) | Flash instructions, build guide, post-flash calibration |
| [releases/v9/MERGE_REPORT.md](releases/v9/MERGE_REPORT.md) | Full audit log of every configuration decision |

---

## How to Flash

Flashing uses **STM32CubeProgrammer** over USB — download it free from [st.com/stm32cubeprog](https://www.st.com/en/development-tools/stm32cubeprog.html).

### Quick steps

1. **Install STM32CubeProgrammer** (Windows / macOS / Linux).
2. **Enter DFU mode:** power off the printer, hold the **BOOT** button on the mainboard, connect USB to your PC, release BOOT.
3. In STM32CubeProgrammer, select **USB** connection, refresh, and click **Connect**.
4. Open the **Erasing & Programming** panel, set file to `firmware-gpro-v9-0x08000000.bin`, start address `0x08000000`, and click **Start Programming**.
5. Disconnect, unplug USB, power cycle the printer.
6. Send `M502` then `M500` to reset EEPROM to firmware defaults.

> **Important:** `M502` + `M500` after every flash — old EEPROM data from a previous version can cause erratic behavior.

### After flashing

Re-run bed leveling after any firmware update:

```
G28        ; home all axes
G29 P1     ; UBL automated mesh
G29 P3     ; fill any unmeasured points
G29 S1     ; save mesh to slot 1
M420 S1    ; enable leveling
M500       ; persist to EEPROM
```

See [releases/v9/README.md](releases/v9/README.md) for the full step-by-step flash guide and post-flash calibration sequence.

## TFT Screen Firmware (Recommended)

The Artillery Genius Pro ships with an outdated TFT firmware. Upgrading it unlocks better compatibility with this Marlin build and fixes several touchscreen bugs.

> **Before flashing:** the Artillery Genius Pro may ship with either a **GD32F305** or an **STM32** TFT28 variant. The files provided are for the **GD32F305 only**. Verify your chip in the TFT About screen before proceeding — the upgrade guide explains how.

**[TFT upgrade guide →](releases/tft/README.md)**

---

## Build from Source

```bash
git clone https://github.com/persano/Marlin.git
cd Marlin
git checkout artillery-genius-pro
python -m platformio run -e Artillery_Ruby
# output: .pio/build/Artillery_Ruby/firmware.bin
```

Requires [PlatformIO](https://platformio.org/) and Python 3.

---

## Hardware

| Component | Part |
|-----------|------|
| MCU | STM32F401RCT6 @ 84 MHz |
| Board | Artillery Ruby |
| Extruder | Direct drive |
| Probe | BLTouch |
| Display | BTT TFT28 (GD32F305 variant) |

---

## License

Marlin firmware is licensed under [GPL v3](https://www.gnu.org/licenses/gpl-3.0.html). Configuration files in this branch follow the same license.

---

## How It Looks

With both the Marlin v9 firmware and the latest TFT firmware running, the info screen confirms the full setup — firmware version, capabilities, and screen chip all in one place.

![TFT info screen with v9 Marlin and latest TFT firmware](releases/tft/tft-info-screen.jpeg)
