# Marlin bugfix-2.1.x — Artillery Genius Pro

Custom Marlin firmware for the **Artillery Genius Pro** (STM32F401RCT6, BOARD_ARTILLERY_RUBY).
Hardware-verified configuration based on Marlin bugfix-2.1.x.

**→ [Download pre-built firmware (v7)](releases/v7/firmware-gpro-v7-0x08000000.bin)**
**→ [Flash instructions & build guide](releases/v7/README.md)**
**→ [Recommended: upgrade your TFT screen firmware](releases/tft/TFT_UPGRADE.md)**

---

## Feature Comparison

Full comparison against stock Marlin 2.1.x, the gpro-mp reference, and Marlin-for-artillery-genius-pro-bugfix-2.1.x.

### Hardware / Board

| Feature | Stock Marlin 2.1.x | gpro-mp | mfagp | Our v7 |
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

| G-code | Purpose | Stock | gpro-mp | mfagp | Our v7 |
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
| M593 | Input shaping config | no | no | yes | yes |
| M600 | Filament change | no | partial | yes | yes |
| M701/M702 | Load/Unload filament | no | no | no | yes |
| M73 | Print progress + remaining time | no | no | yes | yes |
| M810-M819 | G-code macros | no | no | **no** | yes |
| M876 | Host prompt response | no | no | yes | yes |
| M900 | Linear Advance K-factor | no | no | yes | yes |
| M48 | Probe repeatability test | no | no | yes | yes |

### Bed Leveling

| Feature | Stock | gpro-mp | mfagp | Our v7 |
|---------|-------|---------|-------|--------|
| Leveling method | bilinear | UBL | UBL | UBL |
| Probe repetitions | 1 | 3 + 1 extra | 3 + 1 extra | 3 + 1 extra |
| Restore leveling after G28 | no | yes | yes | yes |
| Segment-leveled moves | no | no | no | **yes** (5 mm) |
| Assisted tramming (G35) | no | yes | **no** | yes |

### Motion / Print Quality

| Feature | Stock | gpro-mp | mfagp | Our v7 |
|---------|-------|---------|-------|--------|
| S-curve acceleration | no | yes | yes | yes |
| Junction Deviation | no | yes | yes | yes |
| Linear Advance | no | no | yes (K unset) | **yes** (K=0.13) |
| Input Shaping X+Y | no | no | yes | yes |
| Input Shaping LCD menu | no | no | yes | yes |
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

| Feature | Stock | gpro-mp | mfagp | Our v7 |
|---------|-------|---------|-------|--------|
| Power loss recovery | no | yes | **no** | yes |
| Thermal protection hotend | yes | yes | yes | yes |
| Thermal protection bed | yes | yes | yes | yes |
| Hardware watchdog | no | no | yes | yes |
| Detect broken endstop | no | no | yes | yes |
| Cold extrusion prevention | yes | yes | yes | yes |

### Serial / TFT Communication

| Feature | Stock | gpro-mp | mfagp | Our v7 |
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

| Feature | Stock | gpro-mp | mfagp | Our v7 |
|---------|-------|---------|-------|--------|
| Long filename support | no | no | yes | yes |
| Auto-report SD status (M27) | no | yes | yes | yes |
| SD block retry on error | no | no | no | **yes** |
| Most-recent files first | no | no | yes | yes |
| SD abort G-code (G28XY) | no | no | yes | yes |
| Cancel objects (M486) | no | no | yes | yes |
| G-code macros (M810-M819) | no | no | **no** | yes |

---

## Firmware Download

| File | Description |
|------|-------------|
| [firmware-gpro-v7-0x08000000.bin](releases/v7/firmware-gpro-v7-0x08000000.bin) | Pre-built binary — flash at `0x08000000` |
| [releases/v7/README.md](releases/v7/README.md) | Flash instructions, build guide, post-flash calibration |
| [releases/v7/MERGE_REPORT.md](releases/v7/MERGE_REPORT.md) | Full audit log of every configuration decision |

## TFT Screen Firmware (Recommended)

The Artillery Genius Pro ships with an outdated TFT firmware. Upgrading it unlocks better compatibility with this Marlin build and fixes several touchscreen bugs.

**[TFT upgrade guide →](releases/tft/TFT_UPGRADE.md)**

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
