# Artillery Genius Pro — v9 Change Log

**Previous release:** v8 (`firmware-gpro-v8-0x08000000.bin`)  
**This release:** v9 (`firmware-gpro-v9-0x08000000.bin`)  
**Date:** 2026-05-02  

---

## Summary

v9 adds three features over v8: filament runout sensor support, fan kickstart, and G-code parser compatibility improvements (parenthesis comments + quoted strings). No motion, bed leveling, PID, serial, or FT Motion settings were changed. v9 is a drop-in upgrade from v8; the same post-flash procedure applies (`M502` + `M500`).

---

## Change 1: Filament Runout Sensor (FILAMENT_RUNOUT_SENSOR)

### What changed

`my-fork/Marlin/Configuration.h` — enabled `FILAMENT_RUNOUT_SENSOR` block, replacing the commented-out placeholder.

### Why

The Artillery Genius Pro has a filament runout sensor. The pin used is **PA0** — the original Z-MIN endstop connector, which is freed when BLTouch is installed (Z homing is handled by the BLTouch probe on PC2, not the physical endstop on PA0). This repurposing is confirmed by the community guide for the Artillery Sidewinder X2, which uses the same Artillery Ruby board.

### Configuration

| Define | Value | Notes |
|--------|-------|-------|
| `FILAMENT_RUNOUT_SENSOR` | (gate) | Enables the feature |
| `NUM_RUNOUT_SENSORS` | `1` | One sensor for one extruder |
| `FIL_RUNOUT_PIN` | `PA0` | Z-MIN connector (original Z endstop, repurposed) |
| `FIL_RUNOUT_STATE` | `LOW` | Pin is LOW when filament is absent (NC sensor + internal pullup) |
| `FIL_RUNOUT_PULLUP` | (gate) | Internal pullup on FIL_RUNOUT_PIN |
| `FILAMENT_RUNOUT_SCRIPT` | `"M600"` | Triggers Advanced Pause / filament change on runout |

`FILAMENT_RUNOUT_DISTANCE_MM` is left commented out (immediate pause on runout). Can be set at runtime with `M412 D<mm>` if a debounce distance is needed.

### Wiring note

Connect the filament sensor to the **Z endstop connector on the right side of the mainboard**, not the stock connector on the left. The right-side connector is PA0 — the original Z-MIN endstop pin, freed once BLTouch takes over Z homing. The stock Z endstop cable (left-side connector) should remain disconnected.

PA0 is still listed as `Z_MIN_ENDSTOP_HIT_STATE LOW` in the config. Since BLTouch handles Z homing via PC2, the Z_MIN endstop is never actively checked during homing — a filament runout (LOW on PA0) would also read as "Z_MIN triggered", but this is harmless because it only occurs during a paused print, not during homing.

---

## Change 2: Fan Kickstart (FAN_KICKSTART_TIME)

### What changed

`my-fork/Marlin/Configuration_adv.h` — added `FAN_KICKSTART_TIME 100`.

### Why

Without kickstart, fans commanded to a low PWM level from stopped may fail to spin up due to static friction. A 100 ms full-power burst at startup ensures reliable spin-up at any target speed.

---

## Change 3: G-code Parser Compatibility (PAREN_COMMENTS + GCODE_QUOTED_STRINGS)

### What changed

`my-fork/Marlin/Configuration_adv.h` — added `PAREN_COMMENTS` and `GCODE_QUOTED_STRINGS`.

### Why

- **PAREN_COMMENTS**: Simplify3D and some post-processors emit `(comment)` style inline comments. Without this define, Marlin does not skip parenthesized content and may misparse the command.
- **GCODE_QUOTED_STRINGS**: Enables quoted string parameters such as `M117 "my message"`. Some hosts and macros use this syntax.

---

## Build result

| Metric | v8 | v9 |
|--------|----|----|
| Flash | 69.0% (180,828 B) | 69.5% (182,216 B) |
| RAM | 58.4% (38,244 B) | 58.4% (38,260 B) |

---

## Files changed

| File | Change |
|------|--------|
| `Marlin/Configuration.h` | Enable FILAMENT_RUNOUT_SENSOR block (PA0, LOW, M600) |
| `Marlin/Configuration_adv.h` | Add FAN_KICKSTART_TIME 100, PAREN_COMMENTS, GCODE_QUOTED_STRINGS |
