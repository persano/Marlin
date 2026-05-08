# Artillery Genius Pro — v10 Change Log

**Previous release:** v9 (`firmware-gpro-v9-0x08000000.bin`)
**This release:** v10 (`firmware-gpro-v10-0x08000000.bin`)
**Date:** 2026-05-08

---

## Summary

v10 brings two build-pipeline upgrades over v9 (GCC 10.3.1 toolchain and link-time optimization) plus one runtime addition (M575 — runtime baud-rate change). Net flash usage is ~14 KB smaller than v9.

---

## Change 1: Link-Time Optimization (−flto)

### What changed

`ini/stm32f4.ini` — added `-flto` to the `[env:Artillery_Ruby]` build flags.

### Why

LTO (Link-Time Optimization) defers optimization to the final link stage, allowing the compiler to eliminate dead code and inline functions across all compilation units. For a large codebase like Marlin, this typically yields a 5–15% flash size reduction with no runtime cost.

---

## Change 2: GCC 10.3.1 Toolchain

### What changed

`ini/stm32f4.ini` — added `platform_packages = toolchain-gccarmnoneeabi@1.100301.220327` to the `[env:Artillery_Ruby]` environment.

### Why

The default PlatformIO ststm32@~12.1 toolchain ships GCC 9.2.1 (released 2019). GCC 10.3.1 (released 2022-03-27) is the next stable version confirmed working for ARM Cortex-M builds via PlatformIO. GCC 11.3.1 is known-broken for ARM Cortex-M (missing `bits/c++config.h`); no GCC 12+ packages are available in the PlatformIO registry. Upstream Marlin bugfix-2.1.x already has this `platform_packages` line in `ini/stm32-common.ini`, commented out.

The upgrade brings improved C++17 support, better optimizer heuristics, and bug fixes across four compiler minor versions. The Artillery Ruby build compiled cleanly with zero warnings or errors.

---

## Change 3: M575 — runtime baud-rate change

### What changed

`Marlin/Configuration_adv.h` — added `#define BAUD_RATE_GCODE` in the serial section.

### Why

Enables the M575 G-code so the host (or TFT) can switch the serial baud rate at runtime without reflashing. Useful when bringing up a different host stack or matching a TFT's preferred baud (e.g. 115200 vs the firmware default 250000).

The change is not persisted to EEPROM — on every boot the firmware reverts to the compiled-in `BAUDRATE` from `Configuration.h`.

Cost: +864 B flash (the M575 command handler).

---

## Build result

| Metric | v9 (GCC 9.2.1, no LTO) | v10 (GCC 10.3.1 + LTO + M575) | Delta vs v9 |
|--------|------------------------|-------------------------------|-------------|
| Flash | 69.5% (182,216 B) | 64.0% (167,692 B) | −14,524 B (−7.9%) |
| RAM | 58.4% (38,260 B) | 58.4% (38,284 B) | +24 B |

---

## Files changed

| File | Change |
|------|--------|
| `ini/stm32f4.ini` | Add `-flto` and `platform_packages = toolchain-gccarmnoneeabi@1.100301.220327` to `[env:Artillery_Ruby]` |
| `Marlin/Configuration_adv.h` | Add `#define BAUD_RATE_GCODE` in `@section serial` |
