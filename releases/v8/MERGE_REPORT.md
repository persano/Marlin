# Artillery Genius Pro — v8 Change Log

**Previous release:** v7 (`firmware-gpro-v7-0x08000000.bin`)  
**This release:** v8 (`firmware-gpro-v8-0x08000000.bin`)  
**Date:** 2026-05-01  
**Commit:** `71fb0913c8` — "Enable FT_MOTION (Fixed-Time Motion) with ZV shaping defaults"

---

## Summary

v8 adds a single feature over v7: **Fixed-Time Motion (FT Motion)**, enabled via `#define FT_MOTION` in `Configuration_adv.h`.

No hardware settings, bed leveling, PID values, serial configuration, or other features were changed. v8 is a drop-in upgrade from v7; the same post-flash procedure applies (`M502` + `M500`).

---

## Change: FT_MOTION enabled

### What changed

`my-fork/Marlin/Configuration_adv.h` — 69 lines inserted after the SENSORLESS_HOMING section.

### Why

FT Motion uses a fixed-time trajectory planner with its own built-in input shaping. Unlike the standard M593 input shaping (which compensates at the planning layer), FT Motion runs at a fixed 1 kHz generation rate and applies ZV/ZVD/MZV shaping as part of the motion profile. This can improve print quality at higher speeds where the standard shaper's step-timing approximations introduce residual ringing.

Standard motion (INPUT_SHAPING_X/Y, M593) is **not removed** — both are compiled in. Toggle at runtime:
- `M493 S1` — switch to FT Motion
- `M493 S0` — return to standard motion

### Configuration added

| Define | Value | Notes |
|--------|-------|-------|
| `FT_MOTION` | (gate) | Enables the feature |
| `FTM_SHAPER_ZV` | compiled | ZV shaper algorithm |
| `FTM_SHAPER_ZVD` | compiled | ZVD shaper algorithm |
| `FTM_SHAPER_MZV` | compiled | MZV shaper algorithm |
| `FTM_DEFAULT_SHAPER_X` | `ftMotionShaper_ZV` | ZV active on boot (if FT Motion enabled) |
| `FTM_SHAPING_DEFAULT_FREQ_X` | `55.0f` Hz | From `SHAPING_FREQ_X` (re-measure on Genius Pro) |
| `FTM_SHAPING_ZETA_X` | `0.1f` | Damping ratio X |
| `FTM_DEFAULT_SHAPER_Y` | `ftMotionShaper_ZV` | ZV active on boot (if FT Motion enabled) |
| `FTM_SHAPING_DEFAULT_FREQ_Y` | `48.6f` Hz | From `SHAPING_FREQ_Y` (re-measure on Genius Pro) |
| `FTM_SHAPING_ZETA_Y` | `0.1f` | Damping ratio Y |
| `FTM_DEFAULT_SHAPER_Z` | `ftMotionShaper_NONE` | Leadscrew — no shaping needed |
| `FTM_DEFAULT_SHAPER_E` | `ftMotionShaper_NONE` | Extruder — no shaping |
| `FTM_POLYS` | (gate) | Enables POLY5/POLY6 trajectory support |
| `FTM_TRAJECTORY_TYPE` | `TRAPEZOIDAL` | Default profile (continuous velocity) |
| `FTM_POLY6_ACCELERATION_OVERSHOOT` | `1.875f` | Required companion for FTM_POLYS |
| `FTM_BUFFER_SIZE` | `128` | 128 ms buffer at 1 kHz |
| `FTM_FS` | `1000` | Trajectory generation rate (Hz) |
| `FTM_MIN_SHAPE_FREQ` | `20` | Minimum supported shaping frequency (Hz) |

### Build result

| Metric | v7 | v8 |
|--------|----|----|
| Flash | ~67% | 69.0% (181 KB / 256 KB) |
| RAM | ~57% | 58.4% (38 KB / 64 KB) |
| Build time | — | 47.6 s (Artillery_Ruby env) |

Flash and RAM usage are well within limits. No features were removed to make room.

### Notes for calibration

The shaping frequencies `55.0 Hz` (X) and `48.6 Hz` (Y) were carried over from the `INPUT_SHAPING_X/Y` values, which were originally measured on a Sidewinder X2. These are a reasonable starting point but should be re-measured on your specific Genius Pro with an ADXL345 accelerometer:

```gcode
M493 S1          ; enable FT Motion
M493 A0 F55.0    ; set X shaping freq (update after measurement)
M493 A1 F48.6    ; set Y shaping freq (update after measurement)
M500             ; save to EEPROM
```

---

## Change: Temperature broadcast during host-controlled preheat

### What changed

`my-fork/Marlin/src/module/temperature.cpp` — 4 lines added across two wait loops.

### Why

When printing via a USB host (e.g. BeagleCam), the printer's M109/M190 blocking wait loop sends the 1-second temperature heartbeat only to the requesting serial port (USB). The TFT's serial port (UART) received nothing during preheat, so the TFT screen showed stale temperatures while heating up.

The `auto_reporter.tick()` path (M155) would normally broadcast to all ports every 2 seconds, but most host tools send `M155 S0` on connect, disabling it globally.

### Fix

Added `PORT_REDIRECT(SerialMask::All)` + `PORT_RESTORE()` around the `print_heater_states()` block in both `wait_for_hotend()` and `wait_for_bed()`. This broadcasts the existing 1-second temperature update to all serial ports unconditionally during any blocking preheat — no config required, and unaffected by whether the host disables M155.

---

## Files changed

| File | Change |
|------|--------|
| `Marlin/Configuration_adv.h` | +69 lines (FT_MOTION block) |
| `Marlin/src/module/temperature.cpp` | +4 lines (broadcast preheat temps to all ports) |
