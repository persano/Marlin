# Marlin bugfix-2.1.x for Artillery Genius Pro — v12

Custom Marlin firmware for the **Artillery Genius Pro** (STM32F401RCT6, BOARD_ARTILLERY_RUBY) based on Marlin bugfix-2.1.x.

> **v12 is an incremental diagnostic build** over v11 in the ongoing Mintion Beagle USB-serial proxy stuck-print investigation. It quadruples the FT_MOTION planner ring buffer (`FTM_BUFFER_SIZE` 128 → 512) on top of v11's protocol-content reverts, and syncs 5 additional upstream commits. See [MERGE_REPORT.md](MERGE_REPORT.md) for the full reasoning.

---

## Flash the pre-built binary

If you just want to update your printer, use the pre-built binary — no compilation required.

**File:** `firmware-gpro-v12-0x08000000.bin`
**Flash address:** `0x08000000`
**Tool required:** [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) (free, from ST)

### How to flash

#### 1. Install STM32CubeProgrammer

Download and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) from ST. Available for Windows, macOS, and Linux.

#### 2. Enter DFU mode via M997

1. Connect a USB cable from the mainboard to your PC.
2. Send `M997` from any serial terminal (Pronterface, OctoPrint, slicer console). The board reboots into DFU mode.
3. STM32CubeProgrammer should detect the device as `STM32 BOOTLOADER` over USB.

#### 3. Flash

1. In STM32CubeProgrammer: connect via USB; select the device.
2. Load `firmware-gpro-v12-0x08000000.bin` at address `0x08000000`.
3. Click "Start Programming".
4. After programming completes, click "Disconnect" and power-cycle the printer.

> **Important:** always run `M502` + `M500` after flashing. Old EEPROM data from a previous firmware version can cause erratic behavior.

---

## What's new in v12

### 0. Hotend Extruder Auto-Fan enabled — **safety fix** (regression vs stock Artillery)

`Marlin/Configuration_adv.h` — new block:

```cpp
#define E0_AUTO_FAN_PIN              PC7   // FAN1 on Ruby = hotend heatsink fan
#define EXTRUDER_AUTO_FAN_TEMPERATURE 50
#define EXTRUDER_AUTO_FAN_SPEED      255
```

The fork's slimmed `Configuration_adv.h` had never carried the auto-fan block, so the hotend heatsink fan on PC7 was unmanaged — it would only run if the user manually sent `M106 P1 S255`. Without it, heat creeps up the heatbreak during a print and clogs the cold end. With this fix the firmware drives PC7 at full speed whenever the hotend reads ≥ 50 °C and turns it off below — no host commands required. Treat FAN1 as fully automatic from now on.

### 1. `FTM_BUFFER_SIZE` quadrupled — primary Beagle hypothesis being probed

`Marlin/Configuration_adv.h:623` — `FTM_BUFFER_SIZE` raised from `128` to `512`.

When FT_MOTION is enabled the stepper ISR consumes from a ring buffer of `stepper_plan_t` entries at `FTM_FS = 1000 Hz`. So 128 entries was 128 ms of step lookahead — short enough that a brief host-pipeline stall (Beagle proxy internal buffering, USB-CDC backpressure, slicer chunked send) could drain it and cause motion underflow. With 512 entries the planner has **512 ms of cushion** — any single host-side stall shorter than half a second can no longer underflow it.

This is the new variable being tested in the deadlock investigation. All v11 reverts remain in place.

### 2. 5 commits synced from upstream MarlinFirmware/Marlin bugfix-2.1.x

| SHA | Subject |
|---|---|
| `465fcd055a` | [cron] Bump distribution date (2026-06-04) |
| `32940d77d6` | 🚸 Rotate Progress for DWIN MarlinUI (#28449) — no DWIN on Ruby |
| `4d57f80618` | [cron] Bump distribution date (2026-06-03) |
| `43ab8f4fe7` | 🩹 Fix Z spike on Y/X resonance test start (#28452) |
| `000963994e` | 🔧 Fix NEOPIXEL_BKGD_INDEX_FIRST sanity check (#28453) |

None touch HAL/STM32, usb_serial, the host-action emitters, the auto-report timers, or the gcode queue — the protocol-content A/B against v11 remains readable.

### Build result

| | v11 | v12 |
|---|---|---|
| Flash | 74.1% (194,120 B) | 74.1% (194,288 B) |
| RAM | 58.7% (38,500 B) | 69.3% (45,416 B) |

The +6,916 B RAM cost is dominated by `(512 − 128) × sizeof(stepper_plan_t)` = 384 × 18 B for the FT_MOTION ring; the auto-fan handler adds only a couple of bytes. Flash +168 B vs v11. ~19.6 KB RAM headroom remains.

---

## What's carried over from v11

All v11 changes are retained:

- `STARTUP_COMMANDS "M155 S2"` disabled (primary Beagle protocol-content revert)
- `POSTMORTEM_DEBUGGING` enabled (CPU register/stack dump on STM32 fault)
- `DEBUG_FLAGS_GCODE` enabled (`M111` runtime debug mask, off by default)
- `-flto` removed from `[env:Artillery_Ruby]`
- Toolchain reverted to upstream-default GCC 9.2.1

See [v11 README](../v11/README.md) for details.

## What's in this firmware (carried over from v10)

All v10 features are retained:

- **M575** — runtime baud-rate change
- **Filament Runout Sensor** — pauses print and triggers M600 on runout (PA0 / Z-MIN connector, rewiring required — see v10 README)
- **Fan kickstart** — 100 ms full-power burst on fan startup
- **G-code parser compatibility** — `PAREN_COMMENTS`, `GCODE_QUOTED_STRINGS`
- **Temperature reporting to TFT during host-controlled preheat**
- **FT Motion** (M493) — ZV/ZVD/MZV input shaping with **512-entry planner buffer** (v12)
- **BLTouch** with correct dual-pin wiring for the Ruby board
- **Unified Bed Leveling (UBL)** with Hilbert-curve scan and G26 mesh test
- **Linear Advance** (K = 0.13 default for direct drive)
- **Full BTT TFT touchscreen support**
- **Power loss recovery**
- **Hardware watchdog**

---

## How to interpret a v12 test run

### Test the Beagle deadlock fix

1. Flash one printer with v12 via the steps above.
2. Run the previously-deadlocking gcode through the Beagle.
3. Outcome interpretation:
   - **v12 clean, v11 deadlocks:** the FT_MOTION buffer cushion is the cure — the root cause has a planner-underflow component, host pipeline stalls were running the 128 ms buffer dry. Investigation focus shifts away from chatty async messages and toward what is throttling command delivery (Beagle internal buffering, USB-CDC backpressure, slicer chunked-send timing).
   - **Both v12 and v11 clean:** v11's `STARTUP_COMMANDS` revert was already the cure; v12 piles on the buffer bump but the difference is invisible without an A/B against the deadlocking baseline.
   - **v12 still deadlocks:** the buffer cushion was insufficient. Either the host-pipeline stall is longer than 512 ms (unlikely for a transient buffering hiccup), or the root cause is unrelated to planner underflow. Revisit MERGE_REPORT.md ranking.

### Capture a faulting MCU

Same as v11 — see the v11 README for the full procedure.

### Pre-arm full command echo for a diagnostic run

Same as v11 — `M111 S1` in slicer start G-code.

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
|---|---|---|
| Bootloader | `0x08000000` | 16 KB |
| EEPROM emulation | `0x08004000` | 16 KB |
| Firmware | `0x08008000` | 224 KB |

The binary produced by PlatformIO starts at `0x08000000` and includes the bootloader header.

---

## Post-flash calibration

Same as v10/v11:

1. `M502` — reset to firmware defaults
2. `M500` — save to EEPROM
3. Home all axes: `G28`
4. Run UBL mesh: `G29 P1` then `G29 P3` then `G29 S1` (save mesh to slot 1)
5. Enable leveling: `M420 S1`
6. Save: `M500`

---

## Reverting to v11 or v10

If v12 introduces unexpected behavior, flash the previous binary from `releases/v11/firmware-gpro-v11-0x08000000.bin` or `releases/v10/firmware-gpro-v10-0x08000000.bin` using the same procedure. Configuration values are persisted in EEPROM and are forward/backward compatible between v10, v11, and v12.

---

## Files in this release

| File | Description |
|---|---|
| `firmware-gpro-v12-0x08000000.bin` | Pre-built binary, flash at 0x08000000 |
| `MERGE_REPORT.md` | Detailed v11 → v12 change log with rationale |
| `README.md` | This file |

---

## License

Marlin firmware is licensed under [GPL v3](https://www.gnu.org/licenses/gpl-3.0.html). Configuration files and documentation in this branch follow the same license.
