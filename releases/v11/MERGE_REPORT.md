# Artillery Genius Pro — v11 Change Log

**Previous release:** v10 (`firmware-gpro-v10-0x08000000.bin`)
**This release:** v11 (`firmware-gpro-v11-0x08000000.bin`)
**Date:** 2026-06-02

---

## Summary

v11 is a **diagnostic build** targeted at the Mintion Beagle USB-serial proxy stuck-print bug, not a feature release. It pairs a single protocol-content revert (disabling `STARTUP_COMMANDS "M155 S2"`) with two dormant runtime diagnostics, rolls back v10's two build-pipeline optimizations (newer GCC + LTO) to the upstream-default toolchain, and brings the fork current with 31 upstream commits.

Net effect on the host serial wire during normal operation: identical to v10 *except* that Marlin no longer pre-arms `M155 S2` in the command queue at boot. Everything else added in this release is silent until a fault occurs or `M111 Sn` is explicitly sent.

---

## Change 1: disable boot-time temperature auto-report pre-arm

### What changed

`Marlin/Configuration_adv.h:471` — commented out `#define STARTUP_COMMANDS "M155 S2"`.

### Why

The Beagle is a transparent USB-CDC pass-through with limited internal buffering. When Marlin pre-arms `M155 S2` (temperature auto-report every 2 s) via `STARTUP_COMMANDS` before the slicer's start G-code has a chance to inject the post-processor's `M155 S30` mitigation, async chatter floods the proxy before the host has set up the command/`ok` line-numbered handshake. This is the strongest single suspect for the observed `Resend: N<n>` infinite-loop deadlock — reproduced on two different printers (Artillery Genius Pro and Genius Pro 1.5) with two different Beagle units, indicating a firmware-side root cause rather than hardware.

The BTT TFT touchscreen issues its own `M155 S2` over `SERIAL_PORT_2` when it connects, so live temps on the touchscreen remain unaffected.

---

## Change 2: enable `DEBUG_FLAGS_GCODE`

### What changed

`Marlin/Configuration_adv.h:433` — added `#define DEBUG_FLAGS_GCODE`.

### Why

Enables `M111` runtime debug-mask handling. Upstream Marlin emits a build warning when this is off (`"DEBUG_FLAGS_GCODE is recommended if you have space. Some hosts rely on it."`). Output is gated by `M111 Sn` from the host — default is `S0` (off), so emits **zero steady-state chatter**. Lets the user pre-arm `M111 S1` (ECHO every received command) in slicer start G-code for a diagnostic print run without re-flashing.

---

## Change 3: enable `POSTMORTEM_DEBUGGING`

### What changed

`Marlin/Configuration_adv.h:439` — added `#define POSTMORTEM_DEBUGGING`.

### Why

On any `HardFault`/`UsageFault`/`MemManage`/`BusFault`, the firmware dumps CPU registers and a stack trace via `MinSerial` (`HAL/STM32/MinSerial.cpp`). Only fires on MCU fault — silent in normal operation. If the Beagle deadlock turns out to be a soft-crash that `USE_WATCHDOG` is silently rebooting through, this is what surfaces it.

Decode the PC value from the dump against `firmware.elf`:

```bash
arm-none-eabi-addr2line -e .pio/build/Artillery_Ruby/firmware.elf 0x080xxxxx
```

---

## Change 4: remove `-flto` from `[env:Artillery_Ruby]`

### What changed

`ini/stm32f4.ini` — removed `-flto` from `build_flags`; added `build_unflags = -flto` as a belt-and-braces guard against inheritance from `common_stm32`.

### Why

`POSTMORTEM_DEBUGGING` is incompatible with link-time optimization. LTO strips the naked-asm `CommonHandler_ASM` symbol in `HAL/STM32/MinSerial.cpp:142` because it is referenced only from another inline-asm block, producing:

```
ld: MinSerial.cpp:142: undefined reference to `CommonHandler_ASM'
```

Reverting `-flto` is mandatory to enable the crash-dump diagnostic; the size cost is accepted because the v10 flash budget had ~36% headroom.

---

## Change 5: revert pinned toolchain to upstream default

### What changed

`ini/stm32f4.ini` — removed `platform_packages = toolchain-gccarmnoneeabi@1.100301.220327` (GCC 10.3.1) from `[env:Artillery_Ruby]`. The Marlin-upstream default toolchain (`framework-arduinoststm32` → GCC 9.2.1, per the commented hint in `ini/stm32-common.ini:14`) now applies.

### Why

v10 introduced GCC 10.3.1 in tandem with `-flto`. With LTO already reverted (Change 4), reverting the toolchain too moves v11 closer to the build configuration Marlin's CI actually validates against, removing newer-GCC codegen as an unknown in the Beagle deadlock investigation. The change is reversible — re-pinning GCC 10.3.1 is a one-line edit in `[env:Artillery_Ruby]`.

Surprising side-effect: GCC 9.2.1 without LTO produces ~2.6 KB **less** flash than GCC 10.3.1 without LTO for this codebase.

---

## Change 6: sync with upstream MarlinFirmware/Marlin bugfix-2.1.x (31 commits)

### What changed

Two merge commits brought the fork current with `upstream/bugfix-2.1.x`:

- `c0ffa19a32` — round 1 (15 upstream commits, through `5b46ba9923` dated 2026-05-15)
- `49ad56a08f` — round 2 (16 upstream commits, through `df59b87ced` dated 2026-05-30)

### Substantive upstream fixes pulled in

- Prevent unwanted downward move after motor-off (#28444) — motion safety
- Reduce Segmented Leveled Moves judder (#28433)
- Skip unnecessary stepper ENA pin conflict check (#28432)
- Ignore filament motion when out (#28429)
- Fix BLTouch Reset w/ FTDI EVE Touch UI (irrelevant on this hardware)
- Fix build of TFT_LVGL_UI + PROBE_OFFSET_WIZARD (irrelevant; build-only)
- Fix Resonance Testing with TMC2208
- PART_COOLING_FAN pins (#28356) — new optional commented-out feature
- Fix E-only position reporting (irrelevant; not an E-only config)
- HAS_PIN_27_BOARD → USE_PIN_27_BOARD rename
- Additional patches for E-only build (irrelevant)
- 14 cron distribution-date bumps
- Misc. G-code comment / param tidy
- Build infra: parallel configuration scripts, clang-cache .gitignore, Makefile format-pins
- Fix Invaders quitting (LCD easter-egg game)
- Fix Mesh SCAD F5 preview label orientation (CAD asset only)

**None** of the 31 commits touch `HAL/STM32/`, `usb_serial.cpp`, `serial_hook.h`, the host-action emitters, the auto-report timers, or the gcode queue — the protocol-content A/B against v10 remains readable.

### Configuration conflicts during merge

`Marlin/Configuration.h` and `Marlin/Configuration_adv.h` are heavily customized in this fork — git's diff algorithm produced multi-hundred-line conflict blocks despite the actual upstream deltas being tiny. Both rounds were resolved by keeping fork HEAD; the upstream-only changes lost in each case were:

- Round 1: 1 comment-typo fix in a `FILAMENT_MOTION_SENSOR` block the fork already deleted (Configuration.h); 19 commented-out `PART_COOLING_FAN*_PIN` defines (Configuration_adv.h, zero runtime effect)
- Round 2: 7 commented-out `USE_PIN_27_BOARD` defines (Configuration.h, Creality-specific)

All zero-runtime-effect; nothing functional lost.

---

## Build result

| Metric | v10 (GCC 10.3.1 + LTO) | v11 (GCC 9.2.1, no LTO, diagnostics on) | Delta vs v10 |
|---|---|---|---|
| Flash | 64.0% (167,868 B) | 74.1% (194,120 B) | +26,252 B (+10.1 pp) |
| RAM | 58.4% (38,284 B) | 58.7% (38,500 B) | +216 B (+0.3 pp) |

Flash delta breakdown: ~28 KB from removing LTO (recovered ~2.6 KB by reverting the toolchain), ~few hundred bytes from the fault-handler code path. ~66 KB flash headroom remains.

---

## A/B test protocol

v11 is the v10 baseline with multiple variables changed at once (deliberately — to avoid sequential flash cycles). If v11 prints cleanly through the Beagle on a previously-deadlocking gcode, the bug lives in one of:

1. `STARTUP_COMMANDS "M155 S2"` (highest prior probability)
2. Upstream-vs-fork divergence somewhere in the 31 merged commits (lowest prior — none touch the host/USB path)
3. `-flto` + GCC 10.3.1 codegen differences

If v11 still deadlocks, the remaining protocol-content suspects to revert one at a time are: `HOST_ACTION_COMMANDS` block, `ADVANCED_OK`, `AUTO_REPORT_POSITION`, `AUTO_REPORT_SD_STATUS`, `REPORT_FAN_CHANGE`, `RX_BUFFER_SIZE 1024`.

Confirmed not at fault on this STM32 USB-CDC HAL: `SERIAL_XON_XOFF`. Although `Configuration_adv.h:418` enables it via the `RX_BUFFER_SIZE >= 1024` auto-gate and `M115` advertises the capability, there is **zero** XON/XOFF byte-emission code anywhere in `HAL/STM32/` — the feature is implemented only for AVR (`HAL/AVR/MarlinSerial.h:226`) and DUE (`HAL/DUE/MarlinSerial.h`). On STM32 USB-CDC it is dead code.

---

## Files changed (since v10)

| File | Change |
|---|---|
| `Marlin/Configuration_adv.h` | Comment out `STARTUP_COMMANDS "M155 S2"`; add `DEBUG_FLAGS_GCODE` and `POSTMORTEM_DEBUGGING` |
| `ini/stm32f4.ini` | Remove `-flto`, add `build_unflags = -flto`, remove `platform_packages` toolchain pin |
| `releases/v11/` | New — `firmware-gpro-v11-0x08000000.bin`, `README.md`, this file |
| 31 upstream commits | Cross-tree changes in motion/leveling/TMC/UI; none touching the host serial path |
