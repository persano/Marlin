# Marlin bugfix-2.1.x for Artillery Genius Pro — v10

Custom Marlin firmware for the **Artillery Genius Pro** (STM32F401RCT6, BOARD_ARTILLERY_RUBY) based on Marlin bugfix-2.1.x.

Built and tested on a physical Genius Pro unit. All configurations are hardware-verified.

---

## Flash the pre-built binary

If you just want to update your printer, use the pre-built binary — no compilation required.

**File:** `firmware-gpro-v10-0x08000000.bin`  
**Flash address:** `0x08000000`  
**Tool required:** [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) (free, from ST)

### How to flash

#### 1. Install STM32CubeProgrammer

Download and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) from ST. Available for Windows, macOS, and Linux.

#### 2. Enter DFU mode via M997

1. Connect a USB cable from the mainboard to your PC.
2. Power on the printer.
3. Send **`M997`** from the TFT screen terminal or any serial software (e.g. [Pronterface](https://www.pronterface.com/), OctoPrint) at 250000 baud.

The board will reboot into DFU mode and appear as a USB DFU device on your PC.

#### 3. Flash with STM32CubeProgrammer

1. Open STM32CubeProgrammer.
2. In the connection panel (top right), select **USB** from the dropdown.
3. Click the refresh button — the DFU device should appear (e.g. `USB1`).
4. Click **Connect**.
5. In the left sidebar, click the **Erasing & Programming** icon (arrow pointing down into a chip).
6. Under **File path**, browse to `firmware-gpro-v10-0x08000000.bin`.
7. Set **Start address** to `0x08000000`.
8. Check **Verify programming** (recommended).
9. Click **Start Programming**.
10. Wait for the "File download complete" confirmation.

#### 4. Finish

1. Click **Disconnect** in STM32CubeProgrammer.
2. Unplug the USB cable.
3. Power cycle the printer (off, then on).
4. Send `M502` then `M500` to reset EEPROM to firmware defaults and save them.

> **Important:** always run `M502` + `M500` after flashing. Old EEPROM data from a previous firmware version can cause erratic behavior.

---

## What's new in v10

v10 brings build-pipeline upgrades over v9 plus one runtime addition.

**GCC 10.3.1 toolchain**

Upgraded from GCC 9.2.1 to GCC 10.3.1, which brings improved C++17 support, better optimizer heuristics, and four years of compiler bug fixes. Builds cleanly with zero warnings.

**Link-Time Optimization (`-flto`)**

LTO allows the compiler to optimize across all compilation units at link time, eliminating dead code and inlining across module boundaries. Combined with the GCC 10.3.1 upgrade, this reduces flash usage by ~14 KB (7.9%) compared to v9.

**M575 — runtime baud-rate change**

`BAUD_RATE_GCODE` is now enabled. Send `M575 B<baud>` (or `M575 P<port> B<baud>`) to switch the serial baud rate without reflashing. Accepted values: 2400, 9600, 19200, 38400, 57600, 115200, 250000, 500000, 1000000. Note: the change is **not** persisted — the next boot reverts to the compiled-in default (250000). Your sender must reconnect at the new baud immediately or comms will go silent.

| Metric | v9 | v10 |
|--------|----|-----|
| Flash | 69.5% (182,216 B) | 64.0% (167,692 B) |
| RAM | 58.4% (38,260 B) | 58.4% (38,284 B) |

---

## What's in this firmware

**Filament Runout Sensor** *(added in v9)*

The Genius Pro filament sensor is enabled. When filament runs out, the printer pauses and triggers `M600` (filament change).

**Rewiring required:** the sensor is connected to the TFT board by default — Marlin cannot read it there. Unplug it from the TFT board and connect it to the **white connector on the left side of the printer, below the Z axis at the front** — this is the **PA0 (Z-MIN) pin** on the mainboard.

For a visual guide on how to route the cable, follow this video from the timestamp: [YouTube — cable routing guide (9:26)](https://www.youtube.com/watch?v=WqoeYWdL-Hc&t=566s)

The sensor is a switch: filament present = pin LOW; no filament = pin HIGH (runout triggered). Enable/disable and runout distance can be set at runtime with `M412`.

**Fan kickstart (`FAN_KICKSTART_TIME 100`)** *(added in v9)*

Fans receive a 100 ms full-power burst when starting from stopped, ensuring reliable spin-up at any target PWM value.

**G-code parser compatibility** *(added in v9)*

- `PAREN_COMMENTS` — ignores `(inline comments in parentheses)` used by Simplify3D and some post-processors
- `GCODE_QUOTED_STRINGS` — enables quoted string parameters such as `M117 "my message"`

**Temperature reporting to TFT during host-controlled preheat** *(added in the v8 patch series)*

When printing via USB host (BeagleCam, OctoPrint, etc.), temperature updates during M109/M190 preheat are broadcast to all serial ports — the TFT display no longer shows frozen temperatures during preheat.

---

## What's different from stock Marlin

See [FEATURE_COMPARISON.md](../v9/FEATURE_COMPARISON.md) for a full side-by-side table against stock Marlin, the gpro-mp reference, and the Marlin-for-artillery-genius-pro reference. (Feature set is identical to v9.)

**Summary of key additions over stock:**

- **FT Motion** (M493) — fixed-time trajectory planner with ZV/ZVD/MZV shaping; toggle at runtime alongside Input Shaping
- **Filament runout sensor** — pauses print and triggers M600 on runout (PA0 / Z-MIN connector)
- **BLTouch** with correct dual-pin wiring for the Ruby board (Z_MIN_PROBE = PC2, Z_MIN = PA0)
- **Unified Bed Leveling (UBL)** with 3-point probing, Hilbert curve scan, and G26 mesh test
- **Input Shaping** (X + Y) with live tuning menu — reduces ringing/ghosting
- **Linear Advance** (K = 0.13 starting point for direct drive) — reduces bulging at corners
- **Full BTT TFT touchscreen support** — host action commands, prompts, status notifications, auto-report temperatures, auto-report position, M73 progress bar
- **Power loss recovery** — resume after power failure
- **M92** — set steps-per-unit at runtime
- **M211** — toggle software endstops at runtime
- **M575** — change serial baud rate at runtime *(added in v10)*
- **M600** — filament change mid-print
- **M486** — cancel individual objects mid-print
- **M43** — pin debug and toggle (diagnostic tool)
- **Arc support** (G2/G3) — smooth curves without extra host processing
- **Hardware watchdog** — resets printer if firmware hangs
- **Assisted tramming** (G35) — guided manual bed screw leveling
- **Segment-leveled moves** — UBL mesh compensation applied every 5 mm on long moves
- Hardware-tuned PID values for the Genius Pro hotend and bed

---

## Build from source

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- Python 3.x

### Steps

```bash
git clone https://github.com/persano/Marlin.git
cd Marlin
git checkout artillery-genius-pro
python -m platformio run -e Artillery_Ruby
```

The compiled binary will be at `.pio/build/Artillery_Ruby/firmware.bin`.

### Flash address

This board uses the standard STM32F401 layout:

| Region | Address | Size |
|--------|---------|------|
| Bootloader | `0x08000000` | 16 KB |
| EEPROM emulation | `0x08004000` | 16 KB |
| Firmware | `0x08008000` | 224 KB |

The binary produced by PlatformIO starts at `0x08000000` and includes the bootloader header.

---

## Post-flash calibration

1. `M502` — reset to firmware defaults
2. `M500` — save to EEPROM
3. Home all axes: `G28`
4. Run UBL mesh: `G29 P1` then `G29 P3` then `G29 S1` (save mesh to slot 1)
5. Enable leveling: `M420 S1`
6. Save: `M500`
7. Calibrate Linear Advance K-factor using the [Marlin K-factor calibration pattern](https://marlinfw.org/tools/lin_advance/k-factor.html)
8. Calibrate Input Shaping frequency with `M593` (standard motion) or `M493` (FT Motion)
9. Optionally enable FT Motion: `M493 S1` then `M500` to persist the choice

---

## Hardware

| Component | Part |
|-----------|------|
| MCU | STM32F401RCT6 @ 84 MHz |
| Board | Artillery Ruby |
| Extruder | Direct drive |
| Probe | BLTouch |
| Display | BTT TFT35 (or compatible) |

---

## Files in this release

| File | Description |
|------|-------------|
| `firmware-gpro-v10-0x08000000.bin` | Pre-built binary, flash at 0x08000000 |
| `MERGE_REPORT.md` | Change log from v9 to v10 |
| `README.md` | This file |

---

## License

Marlin firmware is licensed under [GPL v3](https://www.gnu.org/licenses/gpl-3.0.html). Configuration files and documentation in this branch follow the same license.
