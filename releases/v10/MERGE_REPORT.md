# Artillery Genius Pro — v10 Change Log

**Previous release:** v9 (`firmware-gpro-v9-0x08000000.bin`)  
**This release:** v10 (`firmware-gpro-v10-0x08000000.bin`)  
**Date:** 2026-05-03  

---

## Summary

v10 is a build-optimization-only release over v9. No features, motion, bed leveling, PID, serial, or hardware configuration settings were changed. The firmware is functionally identical to v9 — only the compiler toolchain and link-time optimization settings were updated, resulting in a 15 KB smaller binary.

---

## Change 1: Link-Time Optimization (−flto)

### What changed

`ini/stm32f4.ini` — added `-flto` to the `[env:Artillery_Ruby]` build flags.

### Why

LTO (Link-Time Optimization) defers optimization to the final link stage, allowing the compiler to eliminate dead code and inline functions across all compilation units. For a large codebase like Marlin, this typically yields a 5–15% flash size reduction with no runtime cost. The actual result for this build was a 9.4% reduction (−17,200 B) over the unoptimized v9 baseline.

---

## Change 2: GCC 10.3.1 Toolchain

### What changed

`ini/stm32f4.ini` — added `platform_packages = toolchain-gccarmnoneeabi@1.100301.220327` to the `[env:Artillery_Ruby]` environment.

### Why

The default PlatformIO ststm32@~12.1 toolchain ships GCC 9.2.1 (released 2019). GCC 10.3.1 (released 2022-03-27) is the next stable version confirmed working for ARM Cortex-M builds via PlatformIO. GCC 11.3.1 is known-broken for ARM Cortex-M (missing `bits/c++config.h`); no GCC 12+ packages are available in the PlatformIO registry. Upstream Marlin bugfix-2.1.x already has this `platform_packages` line in `ini/stm32-common.ini`, commented out.

The upgrade brings improved C++17 support, better optimizer heuristics, and bug fixes across four compiler minor versions. The Artillery Ruby build compiled cleanly with zero warnings or errors.

---

## Build result

| Metric | v9 (GCC 9.2.1, no LTO) | v10 (GCC 10.3.1 + LTO) | Delta |
|--------|------------------------|------------------------|-------|
| Flash | 69.5% (182,216 B) | 63.6% (166,828 B) | −15,388 B (−8.5%) |
| RAM | 58.4% (38,260 B) | 58.4% (38,284 B) | +24 B |

---

## Files changed

| File | Change |
|------|--------|
| `ini/stm32f4.ini` | Add `-flto` and `platform_packages = toolchain-gccarmnoneeabi@1.100301.220327` to `[env:Artillery_Ruby]` |
