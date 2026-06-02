# Marlin bugfix-2.1.x for Artillery Genius Pro — v11

Custom Marlin firmware for the **Artillery Genius Pro** (STM32F401RCT6, BOARD_ARTILLERY_RUBY) based on Marlin bugfix-2.1.x.

> **v11 is a diagnostic build**, not a feature release. It targets the Mintion Beagle USB-serial proxy stuck-print bug (`Resend: N<n>` infinite-loop deadlock when printing through the Beagle camera). See [MERGE_REPORT.md](MERGE_REPORT.md) for the full reasoning and `BEAGLE_STUCK_PRINT_INVESTIGATION.md` at the repo root for the upstream three-way config comparison.

---

## Flash the pre-built binary

If you just want to update your printer, use the pre-built binary — no compilation required.

**File:** `firmware-gpro-v11-0x08000000.bin`
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
2. Load `firmware-gpro-v11-0x08000000.bin` at address `0x08000000`.
3. Click "Start Programming".
4. After programming completes, click "Disconnect" and power-cycle the printer.

> **Important:** always run `M502` + `M500` after flashing. Old EEPROM data from a previous firmware version can cause erratic behavior.

---

## What's new in v11

v11 is a **debug build** for an active investigation, not a stable feature release. Three categories of change vs v10:

### 1. Protocol-content revert — primary Beagle fix candidate

**`STARTUP_COMMANDS "M155 S2"` disabled.** Marlin no longer pre-arms 2-second temperature auto-reporting in its command queue at boot. The BTT TFT touchscreen still issues its own `M155 S2` over the dedicated UART (`SERIAL_PORT_2`) when it connects, so live temps on the touchscreen are unaffected. The change targets a desync window where async chatter from a pre-armed `M155` floods the Beagle's internal buffer before the host's `M155 S30` mitigation can take effect.

### 2. Diagnostics enabled (dormant by default — zero steady-state output)

**`POSTMORTEM_DEBUGGING`.** On any STM32 fault (`HardFault`, `UsageFault`, `MemManage`, `BusFault`), the firmware dumps CPU registers and stack trace via USB-CDC. Silent in normal operation. Decode the dumped PC value against `firmware.elf` with `arm-none-eabi-addr2line` to identify the faulting instruction.

**`DEBUG_FLAGS_GCODE`.** Enables `M111` runtime debug-mask. Output is gated by `M111 Sn` from the host — default is off. Pre-arm `M111 S1` in slicer start G-code to echo every received command during a diagnostic print run.

### 3. Build pipeline reverted to Marlin upstream defaults

| | v10 | v11 |
|---|---|---|
| Toolchain | GCC 10.3.1 (pinned) | GCC 9.2.1 (upstream default) |
| Link-time optimization | `-flto` enabled | disabled |
| Flash | 64.0% (167,868 B) | 74.1% (194,120 B) |
| RAM | 58.4% (38,284 B) | 58.7% (38,500 B) |

`-flto` had to come off because it's incompatible with `POSTMORTEM_DEBUGGING` (LTO strips the fault-handler's naked-asm symbol). Reverting the toolchain too is a deliberate A/B variable — it isolates whether v10's newer-GCC + LTO codegen contributes to the Beagle deadlock. ~66 KB flash headroom remains.

### 4. 31 commits synced from upstream MarlinFirmware/Marlin bugfix-2.1.x

Notable substantive upstream fixes:
- Prevent unwanted downward move after motor-off (#28444)
- Reduce Segmented Leveled Moves judder (#28433)
- Skip unnecessary stepper ENA pin conflict check (#28432)
- Fix Resonance Testing with TMC2208
- Optional `PART_COOLING_FAN*_PIN` remap (commented-out, off-by-default)

None of the 31 commits touch HAL/STM32, usb_serial, the host-action emitters, the auto-report timers, or the gcode queue — the protocol-content A/B against v10 remains readable.

---

## What's in this firmware (carried over from v10)

All v10 features are retained. See [v10 README](../v10/README.md) for the full description. Key features:

- **M575** — runtime baud-rate change (v10)
- **Filament Runout Sensor** — pauses print and triggers M600 on runout (PA0 / Z-MIN connector, rewiring required — see v10 README)
- **Fan kickstart** — 100 ms full-power burst on fan startup
- **G-code parser compatibility** — `PAREN_COMMENTS`, `GCODE_QUOTED_STRINGS`
- **Temperature reporting to TFT during host-controlled preheat**
- **FT Motion** (M493) — ZV/ZVD/MZV input shaping
- **BLTouch** with correct dual-pin wiring for the Ruby board
- **Unified Bed Leveling (UBL)** with Hilbert-curve scan and G26 mesh test
- **Linear Advance** (K = 0.13 default for direct drive)
- **Full BTT TFT touchscreen support**
- **Power loss recovery**
- **Hardware watchdog**

---

## How to use the v11 diagnostics

### Test the Beagle deadlock fix

1. Flash one printer with v11 via the steps above.
2. Leave a second printer on v10 as a control.
3. Run the previously-deadlocking gcode through the Beagle on both.
4. Outcome interpretation:
   - **v11 clean, v10 deadlocks:** root cause confirmed in the v10 → v11 deltas. Most-likely subset: the `STARTUP_COMMANDS "M155 S2"` revert.
   - **Both clean:** the deadlock is intermittent and inconclusive; needs more runs.
   - **v11 still deadlocks:** see [MERGE_REPORT.md](MERGE_REPORT.md) for the next-suspect ranking.

### Capture a faulting MCU

If v11 reboots unexpectedly during a print, check the USB-CDC capture in your slicer/host console for output like:

```
*** HARDFAULT ***
R0 :0x00000000  R1 :0x20003F40  R2 :0xDEADBEEF  R3 :0x00000001
R12:0x00000000  LR :0x080045A1  PC :0x08012F88  PSR:0x21000000
CFSR:0x00020000 HFSR:0x40000000
Stack: 0x20003F40: 0x080045A1 0x08012F88 ...
```

Decode the PC value against the build artifact:

```bash
arm-none-eabi-addr2line -e .pio/build/Artillery_Ruby/firmware.elf 0x08012F88
```

This pinpoints the faulting source line. If the dump itself doesn't reach your host (the Beagle is in the path and may eat it), reconnect direct USB to the printer after the deadlock and read the post-fault state from the next boot.

### Pre-arm full command echo for a diagnostic run

Add to slicer start G-code:
```
M111 S1
```
Marlin will echo every received command back to the host with `echo:` prefix. Output is verbose — only enable for diagnostic runs, then disable with `M111 S0`.

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

Same as v10:

1. `M502` — reset to firmware defaults
2. `M500` — save to EEPROM
3. Home all axes: `G28`
4. Run UBL mesh: `G29 P1` then `G29 P3` then `G29 S1` (save mesh to slot 1)
5. Enable leveling: `M420 S1`
6. Save: `M500`

---

## Reverting to v10

If v11 introduces unexpected behavior, flash the v10 binary from `releases/v10/firmware-gpro-v10-0x08000000.bin` using the same procedure. Configuration values are persisted in EEPROM and are forward/backward compatible between v10 and v11.

---

## Files in this release

| File | Description |
|---|---|
| `firmware-gpro-v11-0x08000000.bin` | Pre-built binary, flash at 0x08000000 |
| `MERGE_REPORT.md` | Detailed v10 → v11 change log with rationale |
| `README.md` | This file |

---

## License

Marlin firmware is licensed under [GPL v3](https://www.gnu.org/licenses/gpl-3.0.html). Configuration files and documentation in this branch follow the same license.
