# Artillery Genius Pro — v12 Change Log

**Previous release:** v11 (`firmware-gpro-v11-0x08000000.bin`)
**This release:** v12 (`firmware-gpro-v12-0x08000000.bin`)
**Date:** 2026-06-04

---

## Summary

v12 is an incremental diagnostic build over v11 in the same Mintion Beagle USB-serial stuck-print investigation. It changes one runtime variable (doubles the FT_MOTION ring buffer) and brings the fork current with 5 additional upstream commits. All v11 changes (commented `STARTUP_COMMANDS "M155 S2"`, `DEBUG_FLAGS_GCODE`, `POSTMORTEM_DEBUGGING`, no `-flto`, upstream-default GCC 9.2.1) are retained.

Net effect on the host serial wire: identical to v11. The FT_MOTION buffer enlargement is internal — it makes the planner's per-axis trajectory queue more tolerant of brief host-side stalls, which is the hypothesis being probed here.

---

## Change 1: double `FTM_BUFFER_SIZE` from 128 to 256

### What changed

`Marlin/Configuration_adv.h:623` — `#define FTM_BUFFER_SIZE 256` (was `128`).

### Why

When `FT_MOTION` is enabled the stepper ISR consumes from a ring buffer of `FTM_BUFFER_SIZE` `stepper_plan_t` entries running at `FTM_FS = 1000 Hz` — so 128 entries was exactly 128 ms of step lookahead. If the host-to-firmware command pipeline stalls for longer than that (Beagle proxy buffering hiccup, slicer chunked send, USB-CDC backpressure on the printer side), the planner runs dry, motion underflows, and the deadlock symptoms surface as the host waits for `ok`/M114 that never come because the buffer-drain interrupt path is also stuck.

Doubling to 256 ms of lookahead gives the planner twice the cushion against transient stalls. The value must be a power of two ≥ 4 (`SanityCheck.h:4711`). Upstream's default is also 128 — the bump is fork-specific, motivated by this investigation.

### Cost

| Metric | v11 | v12 | Delta |
|---|---|---|---|
| RAM | 58.7% (38,500 B) | 62.3% (40,804 B) | +2,304 B (+3.6 pp) |
| Flash | 74.1% (194,120 B) | 74.1% (194,136 B) | +16 B (~0 pp) |

The 2304-byte RAM cost is exactly 128 × `sizeof(stepper_plan_t)` (≈18 B each). ~24 KB RAM headroom remains. User has previously confirmed "do not care about bin size" — RAM cost is comparably small relative to the device's 64 KB total.

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

| Metric | v11 | v12 | Delta |
|---|---|---|---|
| Flash | 74.1% (194,120 B) | 74.1% (194,136 B) | +16 B |
| RAM | 58.7% (38,500 B) | 62.3% (40,804 B) | +2,304 B |

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
| `Marlin/Configuration_adv.h` | `FTM_BUFFER_SIZE` 128 → 256 |
| `releases/v12/` | New — `firmware-gpro-v12-0x08000000.bin`, `README.md`, this file |
| 5 upstream commits | Cron bumps, DWIN UI, resonance-generator fix, NEOPIXEL sanity check |
