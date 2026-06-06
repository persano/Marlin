# Artillery Genius Pro — v12 Change Log

**Previous release:** v11 (`firmware-gpro-v11-0x08000000.bin`)
**This release:** v12 (`firmware-gpro-v12-0x08000000.bin`)
**Date:** 2026-06-04

---

## Summary

v12 is an incremental diagnostic build over v11 in the same Mintion Beagle USB-serial stuck-print investigation. It changes one runtime variable (doubles the FT_MOTION ring buffer) and brings the fork current with 5 additional upstream commits. All v11 changes (commented `STARTUP_COMMANDS "M155 S2"`, `DEBUG_FLAGS_GCODE`, `POSTMORTEM_DEBUGGING`, no `-flto`, upstream-default GCC 9.2.1) are retained.

Net effect on the host serial wire: identical to v11. The FT_MOTION buffer enlargement is internal — it makes the planner's per-axis trajectory queue more tolerant of brief host-side stalls, which is the hypothesis being probed here.

---

## Change 1: quadruple `FTM_BUFFER_SIZE` from 128 to 512

### What changed

`Marlin/Configuration_adv.h:623` — `#define FTM_BUFFER_SIZE 512` (was `128` upstream-default).

### Why

When `FT_MOTION` is enabled the stepper ISR consumes from a ring buffer of `FTM_BUFFER_SIZE` `stepper_plan_t` entries running at `FTM_FS = 1000 Hz` — so 128 entries was exactly 128 ms of step lookahead. If the host-to-firmware command pipeline stalls for longer than that (Beagle proxy buffering hiccup, slicer chunked send, USB-CDC backpressure on the printer side), the planner runs dry, motion underflows, and the deadlock symptoms surface as the host waits for `ok`/M114 that never come because the buffer-drain interrupt path is also stuck.

Raising to 512 entries gives the planner **512 ms** of cushion — quadrupling upstream's default. Any single host-side stall shorter than half a second can no longer underflow the planner. The value must be a power of two ≥ 4 (`SanityCheck.h:4711`). Upstream's default is 128 — the bump is fork-specific, motivated by this investigation.

(History note: this value first went 128 → 256 within the v12 cycle, then 256 → 512 immediately after on a "may as well be safe" decision. The build numbers below reflect the final 512 value.)

### Cost

| Metric | v11 | v12 (FTM_BUFFER_SIZE=512) | Delta |
|---|---|---|---|
| RAM | 58.7% (38,500 B) | 69.3% (45,412 B) | +6,912 B (+10.5 pp) |
| Flash | 74.1% (194,120 B) | 74.1% (194,144 B) | +24 B (~0 pp) |

The 6,912-byte RAM cost is exactly `(512 - 128) × sizeof(stepper_plan_t)` = 384 × 18 B. ~19.6 KB RAM headroom remains. User has previously confirmed "do not care about bin size" — the RAM cost is acceptable relative to the device's 64 KB total.

---

## Change 2: sync with upstream MarlinFirmware/Marlin bugfix-2.1.x (5 commits)

### What changed

Single merge commit brought the fork current with `upstream/bugfix-2.1.x` through `465fcd055a` (2026-06-04 distribution date).

### Upstream commits pulled in

| SHA | Subject | Relevant to Genius Pro? |
|---|---|---|
| `465fcd055a` | [cron] Bump distribution date (2026-06-04) | infra only |
| `32940d77d6` | 🚸 Rotate Progress for DWIN MarlinUI (#28449) | no — Ruby has no DWIN |
| `4d57f80618` | [cron] Bump distribution date (2026-06-03) | infra only |
| `43ab8f4fe7` | 🩹 Fix Z spike on Y/X resonance test start (#28452) | tangentially — resonance generator fix |
| `000963994e` | 🔧 Fix NEOPIXEL_BKGD_INDEX_FIRST sanity check (#28453) | no — feature gated off |

The resonance-test fix touches `Marlin/src/module/ft_motion/resonance_generator.cpp`. It is a tiny patch and only affects the entry phase of an M593/M958 resonance sweep, not normal printing.

### Merge result

Clean — no conflicts this round. (v11's two merge rounds had hit `Configuration.h`/`Configuration_adv.h` because git's diff over the fork's heavy customizations produced spurious conflict regions; the 5-commit set this round was small enough not to trip that.)

**None** of the 5 commits touch `HAL/STM32/`, `usb_serial.cpp`, `serial_hook.h`, the host-action emitters, the auto-report timers, or the gcode queue. The protocol-content A/B against v11 remains readable.

---

## Build result

Filled in from the actual v12 (512) build — see [README.md](README.md) for the table.

Flash delta is dominated by the cron distribution-date string update (a few bytes) and the resonance-generator patch. The FTM_BUFFER_SIZE bump itself is RAM-only — the ring-buffer indexing uses `FTM_BUFFER_MASK = FTM_BUFFER_SIZE - 1u` which the compiler folds into a single AND instruction either way.

---

## A/B interpretation if v12 prints cleanly through the Beagle

Compared to v11 the only runtime variable is the doubled FT_MOTION planner cushion. If v12 makes the deadlock disappear:

- **Strong signal:** the deadlock has a planner-underflow component — the host pipeline is briefly stalling and the cure is more lookahead, not fewer chatty async messages. This would point investigation away from `M155`/auto-reports and toward whatever is throttling command delivery (Beagle internal buffering, USB-CDC flow control, slicer chunked-send timing).
- **Less strong:** the bump masks the deadlock symptom but the root cause is elsewhere — possible if the deadlock was multi-stage and the new cushion is just buying enough time for some other recovery path to fire.

Compared to v10 the variables stacked are: `STARTUP_COMMANDS` revert (v11) + `FTM_BUFFER_SIZE` bump (v12). v12 vs v11 isolates the buffer bump cleanly.

---

## Files changed (since v11)

| File | Change |
|---|---|
| `Marlin/Configuration_adv.h` | `FTM_BUFFER_SIZE` 128 → 512; **new auto-fan block** (`E0_AUTO_FAN_PIN = PC7`) |
| `releases/v12/` | New — `firmware-gpro-v12-0x08000000.bin`, `README.md`, this file |
| 5 upstream commits | Cron bumps, DWIN UI, resonance-generator fix, NEOPIXEL sanity check |

---

## Change 3: enable Extruder Auto-Fan on `E0_AUTO_FAN_PIN = PC7` (FAN1)

### What changed

`Marlin/Configuration_adv.h` — new block in Thermal Settings:

```cpp
#define E0_AUTO_FAN_PIN              PC7   // FAN1 on Ruby = hotend heatsink fan
#define EXTRUDER_AUTO_FAN_TEMPERATURE 50
#define EXTRUDER_AUTO_FAN_SPEED      255
```

### Why

This was a **silent omission** in the fork's slimmed Configuration_adv.h that caused real hardware damage: the hotend heatsink fan on `FAN1_PIN = PC7` had no auto-management, so heat crept up the heatbreak during printing and clogged the cold end. The user had to manually issue `M106 P1 S255` every session to drive the fan.

With the new block, the firmware unconditionally drives PC7 at full speed whenever the hotend reads ≥ 50 °C and turns it off below that — the standard Marlin extruder-auto-fan behavior that stock Artillery firmware ships with. No host commands required.

Note: M106 P1 will still nominally address PC7, but the auto-fan handler runs every ~2.5 s on temperature updates and will override any manual setting — treat FAN1 as fully automatic now.

### Cost

| Metric | v11 | v12 (final) | Delta |
|---|---|---|---|
| Flash | 74.1% (194,120 B) | 74.1% (194,288 B) | +168 B |
| RAM | 58.7% (38,500 B) | 69.3% (45,416 B) | +6,916 B |

Flash delta is +144 B vs the FTM_BUFFER_SIZE=512 pre-fix build, all attributable to the auto-fan handler code path. RAM delta is unchanged from the 512 build (auto-fan state is a couple of bytes that the linker fits into existing alignment slack).
