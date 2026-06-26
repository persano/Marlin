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

## Change 4: enable Thermal Runaway heating-ramp watch (`WATCH_TEMP_PERIOD` + bed)

### What changed

`Marlin/Configuration_adv.h` — new block in Thermal Settings:

```cpp
#define WATCH_TEMP_PERIOD          40   // (s) Heater ramp-up watch window
#define WATCH_TEMP_INCREASE         2   // (°C) Min hotend rise required in that window
#define WATCH_BED_TEMP_PERIOD      60   // (s) Bed ramp-up watch window
#define WATCH_BED_TEMP_INCREASE     2   // (°C) Min bed rise required in that window
```

### Why

This is the **second occurrence of the same slim-config safety regression** that produced the auto-fan miss in Change 3. The fork's slimmed `Configuration_adv.h` had no `WATCH_TEMP_*` defines at all. Marlin's `WATCH_HOTENDS` flag is gated by `Conditionals-5-post.h:2732`:

```c
#if ENABLED(THERMAL_PROTECTION_HOTENDS) && WATCH_TEMP_PERIOD > 0
  #define WATCH_HOTENDS 1
#endif
```

When `WATCH_TEMP_PERIOD` is undefined the preprocessor evaluates it as `0`, so `0 > 0` is false and `WATCH_HOTENDS` is silently NOT defined — *despite* `THERMAL_PROTECTION_HOTENDS` being on. The result: the heating-ramp watch (the protection that catches a dead heater during heat-up) was completely off, even though `M115` reported thermal protection as enabled.

Steady-state thermal runaway protection (the `THERMAL_PROTECTION_PERIOD` / `_HYSTERESIS` mechanism defined in `Configuration.h`) was unaffected and is still active. But the ramp-up watch, which catches an open thermistor, broken heater cartridge, or stuck MOSFET *before* the printer claims temperature was reached, was off.

Same root cause as the auto-fan miss: slim config dropped a load-bearing default and the Conditionals macro silently disables the feature.

### Cost

Linker added the `HeaterWatch<>` template instantiation and surrounding guards to the binary — verifies the feature is now actually compiled in, not just no longer a syntax error.

| Metric | pre-fix v12 | post-fix v12 | Delta |
|---|---|---|---|
| Flash | 74.1% (194,288 B) | 74.4% (195,064 B) | +776 B |
| RAM | 69.3% (45,416 B) | 69.3% (45,432 B) | +16 B |

The +776 B Flash is the new code path that was previously dead-stripped (because `WATCH_HOTENDS` was 0). That's the auditable evidence the fix took effect.

---

## Change 10: revert PR #26952 — restore continuous M155 reports during heat-up

### What changed

`Marlin/src/module/temperature.cpp:4550` — commented out the line `if (marlin.is_heating()) return;` inside `Temperature::AutoReportTemp::report()`. M155 auto-temperature reports now fire on their interval regardless of `M109`/`M190` wait state.

### Why

Upstream PR [MarlinFirmware/Marlin#26952](https://github.com/MarlinFirmware/Marlin/pull/26952) ("Fix Duplicated Temp Report" by InsanityAutomation) added `if (wait_for_heatup) return;` to suppress M155 lines while a wait-for-temperature loop was active, with the stated goal of deduplicating the temp output during `M109`/`M190` (which already emit their own "Heating..." temp updates). Later refactoring wrapped the global behind the helper class — the current code is `if (marlin.is_heating()) return;` where `is_heating()` is `return wait_for_heatup;` (`MarlinCore.h:89`).

ellensp identified this as a regression in [#27645](https://github.com/MarlinFirmware/Marlin/issues/27645), with the same symptom pattern reported in [#25157](https://github.com/MarlinFirmware/Marlin/issues/25157) across LPC176X (WEREWOLF1973, IndaloGo), GD32F303 (dsovsyannikov), and ATMEGA2560 (efsa91) hardware: TFT-style passive hosts stopped working after the upgrade from Marlin 2.1.1 to 2.1.2. Root cause: the wait loop uses `wait_for_heatup` as a *success indicator* and can exit with the flag still true; if the host then issues `M108` mid-wait or the wait path ends via a non-normal route, M155 stays silent indefinitely. A TFT or Beagle-style proxy parsing the temp stream for liveness sees nothing and concludes the printer is unresponsive.

The Beagle is exactly this class of host (it parses Marlin's stream to sniff `//action:` directives for time-lapse triggers). Same mechanism that #24347 / Change 9 addressed for `ADVANCED_OK`: a serial-aware host expects a steady stream of state, Marlin goes uncharacteristically silent in an edge case, host can't resync.

This is a source-level revert, not a Configuration toggle. The patch carries a comment explaining the rationale so future upstream syncs leave it intact — re-apply if a merge ever restores the line.

### Side effect

During `M109`/`M190` heat-up the host sees both:
- the wait loop's own busy "Heating..." temp echo (Marlin emits one on each wait poll)
- the M155 timer's regular `T:...` line

Duplicate temp output during heat-up only — cosmetic regression of what PR #26952 was trying to fix. Hosts that parse the temp stream byte-by-byte handle duplicates fine; humans reading the serial console see two `T:` lines per second instead of one for the duration of heat-up.

### Cost

| Metric | post-ADVANCED_OK-revert | post-#26952-revert | Delta |
|---|---|---|---|
| Flash | 74.4% (195,008 B) | 74.4% (195,000 B) | −8 B |
| RAM | 69.3% (45,432 B) | 69.3% (45,432 B) | 0 B |

The −8 B Flash is the `is_heating()` load + test + early-return branch being dead-stripped. The function now unconditionally calls `print_heater_states`.

### Verification protocol (A/B vs the prior v12 push)

The patched temperature.cpp line is the only variable changed against the prior v12 push (which already has Change 9's `ADVANCED_OK` revert). If a previously-Beagle-deadlocking gcode now prints cleanly, and the prior v12 with `ADVANCED_OK` off did NOT fix it:

- **Strong signal:** the deadlock involves a heat-up-window silence component. The Beagle was losing track of the printer during `M109`/`M190` because M155 went mute, and recovery never happened cleanly. Keep this patch in all future builds.
- **No change vs prior v12:** PR #26952's behavior was not the cause. Source revert can stay (continuous temp output is harmless and a defensive choice) or be reverted — your call.

If the prior v12 already fixed it via `ADVANCED_OK` off, this Change is redundant for the Beagle bug but still defensible as a fragility-reduction patch (it removes a known-buggy edge case).

### Source-level patch — maintenance note

Because this lives in `Marlin/src/module/temperature.cpp` rather than `Configuration_adv.h`, it requires per-merge attention:

1. On every `git merge upstream/bugfix-2.1.x`, check if `temperature.cpp:4550` was touched.
2. If upstream rewrote the surrounding lines, re-apply the comment-out and rationale block.
3. The line will not conflict cleanly with `--ours` resolution because the patched form is divergent — verify manually with `git diff upstream/bugfix-2.1.x -- Marlin/src/module/temperature.cpp`.

---

## Change 9: disable `ADVANCED_OK` — second protocol-content revert (Beagle deadlock)

### What changed

`Marlin/Configuration_adv.h:503` — commented out `#define ADVANCED_OK`.

### Why

Issue [MarlinFirmware/Marlin#24347](https://github.com/MarlinFirmware/Marlin/issues/24347) reports the same Artillery Sidewinder X2 hardware family (same Ruby board) deadlocking when `ADVANCED_OK` is on and a serial streamer / proxy is in the path. Resolution from Marlin contributor InsanityAutomation:

> "That's an issue with the MKS TFT being used for the USB/SD host. That is also a serial streamer, like octoprint, and doesn't support advanced OK."

`ADVANCED_OK` replaces Marlin's plain `ok` response with `ok N<n> P<p> B<b>` (line number + planner-buffer count + serial-buffer count). Any serial proxy that parses the response and doesn't understand the new format breaks the line-numbered handshake.

The Beagle is exactly that class of host — it parses Marlin's stream to sniff `//action:` directives for time-lapse triggers. If the Beagle's parser only knows about plain `ok` and sees `ok N5 P10 B3` instead, the handshake desyncs. Same mechanism behind both #24347's "stuck at 0%" (silent stall) and our `Resend: N<n>` infinite loop (re-ack storm).

The original v10 → v11 investigation ranked `ADVANCED_OK` as the #2 protocol-content suspect after `STARTUP_COMMANDS` (reverted in v11). #24347 is the missing ground-truth evidence promoting it to v12's next-flip.

### Side effect

Hosts that REQUIRE `ADVANCED_OK` to manage Marlin's command buffer (BufferBuddy, some advanced OctoPrint plugins) will no longer work. User does not run these — no functional loss.

### Cost

| Metric | prior v12 push | post-revert v12 | Delta |
|---|---|---|---|
| Flash | 74.5% (195,176 B) | 74.4% (195,008 B) | **−168 B** |
| RAM | 69.3% (45,432 B) | 69.3% (45,432 B) | 0 B |

The −168 B Flash is the `ADVANCED_OK` formatting/state code being dead-stripped. Auditable evidence the revert took effect at the binary level.

### Verification protocol (A/B vs the prior v12 push)

The flash on this v12 is the only variable changed against the prior v12 push. If a previously-Beagle-deadlocking gcode now prints cleanly:

- **Strong signal:** the deadlock has a serial-protocol-mismatch component. The Beagle was misreading `ok N<n> P<p> B<b>`. Keep `ADVANCED_OK` off in all builds intended to print through the Beagle (or any passive serial proxy).
- **No change:** ADVANCED_OK is not the cause. Move to the next suspect — `HOST_ACTION_COMMANDS` block, `AUTO_REPORT_POSITION`, `AUTO_REPORT_SD_STATUS`, `REPORT_FAN_CHANGE`, `RX_BUFFER_SIZE 1024`.

---

## Change 8: sync 16 more upstream commits (round 3 within v12, through 2026-06-26)

Latest upstream sync after the FAN_MIN_PWM commit. 16 commits through `5b0e1a6111`:

| SHA | Subject | Relevant to Genius Pro? |
|---|---|---|
| `5b0e1a6111` | [cron] Bump distribution date (2026-06-26) | infra |
| `a940c26490` | 🐛 Fix ESP32 build with WiFi (#28469) | no — no ESP32 |
| `12875be7a9` | [cron] Bump distribution date (2026-06-25) | infra |
| `045f078df3` | ✅ Refine parallel builds | infra |
| `f6e00bfa99` | [cron] Bump distribution date (2026-06-24) | infra |
| `1f255d16ec` | 🚸 'M421 I J' bounds-checking (#28468) | UBL command — tiny robustness fix |
| `3fe641e081` | 🚸 Preserve workspace with G28 (#28465) | homing-behavior fix |
| `c3974eadf2` | [cron] Bump distribution date (2026-06-23) | infra |
| `2f0b5f04a7` | 🚸 Fix menu value Y pos (#28464) | LCD UI only |
| `0bc89dd620` | 🧑‍💻 Matching config.py files | build infra |
| `aedbcb5063` | [cron] Bump distribution date (2026-06-22) | infra |
| `da81bba7c8` | 🔧 Sanity Check Resonance Test for standard Shaping | added a sanity check; our INPUT_SHAPING config passes (build OK) |
| `d79dbb0dfc` | 🚸 Disable FT_MOTION for M48 (#28314) | **FT_MOTION-related fix** — temporarily disables FT_MOTION during M48 probe-repeatability test to avoid interference; we use both, so this is welcome |
| `9048d826a4` | [cron] Bump distribution date (2026-06-19) | infra |
| `7f2ef9d04d` | ✅ Ignore schema.json file | gitignore + removed file |
| `46376028e5` | [cron] Bump distribution date (2026-06-16) | infra |
| `e255770605` | 🔧 Unified input shaping Resonance Test (#28282) | **refactor**: resonance-test code moved from `Marlin/src/module/ft_motion/` to `Marlin/src/feature/resonance/`. Behavior preserved. |

**None** touch HAL/STM32, usb_serial, the host serial path, thermal protection, or the auto-fan/watch-temp blocks I added. The protocol-content A/B against v11 remains readable.

Configuration_adv.h conflict again resolved with `--ours` (same fork-vs-upstream diff pattern).

Build delta vs prior v12 push: Flash 74.4% → 74.5% (+72 B from upstream code changes), RAM unchanged at 69.3% (45,432 B).

---

## Change 5: sync 5 more upstream commits (round 2 within v12)

Round-2 upstream sync after the initial v12 push:

| SHA | Subject |
|---|---|
| `8c79ed3892` | [cron] Bump distribution date (2026-06-08) |
| `f5ff51b674` | 🔧 Enable HOST_ACTION_COMMANDS by default (#28442) — no-op for us, we already had it on |
| `92a5c34d35` | [cron] Bump distribution date (2026-06-06) |
| `d3e1cbd554` | 🔧 Enforce monotonic bed description (#28310) — bed-leveling helper |
| `428ceb8172` | 🩹 Better guard of keypad_buttons = 0 (#28417) — only fires with keypad UI, irrelevant here |

None touch HAL/STM32, usb_serial, thermal, or motion. Merge required `--ours` resolution on `Configuration_adv.h` again (same fork-vs-upstream diff pattern as prior rounds); upstream-only deltas in the resolved file were the usual inert/commented-out boilerplate.

---

## Change 7: enable `FAN_MIN_PWM 50` — part-cooling-fan stall mitigation

### What changed

`Marlin/Configuration_adv.h` — added in the fan block:

```cpp
#define FAN_MIN_PWM  50   // (0-255) Minimum PWM sent to non-zero fan speeds
```

### Why

Below ~20 % PWM (50/255) the small worn part-cooling fans common on the Artillery hotend assembly stall instead of spinning slowly. Slicer commands like `M106 S40` become silently 0 % cooling rather than "low" cooling, producing stringing and blob defects that read as a slicer bug.

With `FAN_MIN_PWM = 50` the firmware remaps the requested M106 range so the minimum non-zero PWM sent to the FET is 50/255. `M106 S0` still means fan off — `CALC_FAN_SPEED` preserves the off case explicitly (`Conditionals-5-post.h:3000`).

### Precedent

Surveyed five sibling Marlin forks in this project's `marlins/` tree:

| Fork | `FAN_MIN_PWM` |
|---|---|
| stock Genius Pro | commented |
| stock Genius Pro all-metal | commented |
| gpro-mp (custom Genius Pro) | commented |
| mfagp (custom Genius Pro) | commented |
| **Sidewinder X2 (`Marlin-lts-2.1.2-swx2`)** | **enabled, value 50** |

The X2 fork was tuned by the community for the same fan stall pattern. Same hotend fan family on the Genius Pro, so the carry-over is justified. The same survey covered `SD_ABORT_ON_ENDSTOP_HIT` and `HOMING_BACKOFF_POST_MM` — neither is enabled in any sibling fork, so the v12 audit's skip decisions stand for those two.

### Cost

| Metric | pre-fix v12 | post-fix v12 | Delta |
|---|---|---|---|
| Flash | 74.4% (195,064 B) | 74.4% (195,104 B) | +40 B |
| RAM | 69.3% (45,432 B) | 69.3% (45,432 B) | 0 B |

The +40 B Flash is `CALC_FAN_SPEED` switching from its early-out form to the `map(f, 1, 255, FAN_MIN_PWM, FAN_MAX_PWM)` form per the `Conditionals-5-post.h:3000-3003` branch — auditable evidence the remap path is now compiled in.

---

## Change 6: fix stale internal docstring on filament-runout sensor

`Marlin/Configuration.h:634` block-comment used to say `FIL_RUNOUT_STATE LOW: pin is LOW when filament is absent` while the actual define on line 641 sets `FIL_RUNOUT_STATE HIGH`. The actual setting is correct given the user's documented wiring (switch closes to GND when filament present, pullup pulls HIGH when absent). Only the stale docstring was wrong — no behavior change, but a cross-AI audit flagged it as a likely contradiction and the comment was sweeping confusion across future reviews. Updated to match.

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
