# Handoff Prompt — Beagle Stuck-Print Investigation

Paste the block below into a fresh chat. Attach (or paste the contents of)
`BEAGLE_STUCK_PRINT_INVESTIGATION.md` from the same directory so the new
session has the full comparison table.

---

## Prompt to send

I'm debugging a stuck-print bug on a custom Marlin firmware fork. I need a
second opinion to confirm or rule out that the fork is the cause before I
start modifying its config.

**Symptom.** When printing through a Mintion Beagle camera (acting as a
USB-serial proxy between the slicer host PC and the printer's Marlin MCU),
the host eventually gets stuck retransmitting one line number forever (e.g.
`Resend: N87272` over and over) with no `ok` coming back from Marlin. The
print halts. The same gcode prints fine when the host talks directly to the
printer (no Beagle in line) or from the BTT TFT's SD slot (when the TFT
firmware is in a known-good state).

**Why I think it's firmware, not hardware.** Reproduced on **two different
printers** (Artillery Genius Pro and Genius Pro 1.5) running the same fork
("v10"), each with a **different Beagle unit**. Two hardware variables
swapped, one firmware constant — points at firmware.

**Existing mitigation.** A G-code post-processor injects `M154 S0` (disable
Marlin position auto-report) and `M155 S30` (slow temperature auto-report
from default 5 s to 30 s) immediately after `action:print_start`. This
reduces but does NOT fully eliminate the deadlock, so other async chatter
sources are still firing.

**What I need from you.** Read the attached `BEAGLE_STUCK_PRINT_INVESTIGATION.md`
(three-way comparison of my fork vs stock Artillery vs vanilla Marlin
bugfix-2.1.x across 32 serial/auto-report/host-communication settings).

Then:

1. **Validate or challenge the top-three suspect ranking.** The doc names
   `SERIAL_XON_XOFF`, the auto-report flood, and `ADVANCED_OK` as the
   primary candidates. Is that ranking defensible? Anything missing?

2. **Specifically scrutinise `SERIAL_XON_XOFF` over USB-CDC.** My claim is
   that the Marlin USB-CDC endpoint (STM32 HAL) emits XON/XOFF as in-band
   bytes that the Beagle proxy passes through opaquely, but the host's USB
   driver may not honour them as flow control. Is that physically accurate?
   What does Marlin's STM32 HAL actually do when `SERIAL_XON_XOFF` is
   enabled on a CDC endpoint? Read
   `marlins/my-fork/Marlin/src/HAL/STM32/HardwareSerial.h` and
   `marlins/my-fork/Marlin/src/core/serial.h` if needed.

3. **Quantify the async traffic.** Given the fork enables
   `AUTO_REPORT_TEMPERATURES` (default 5 s, overridden to 2 s at boot via
   `STARTUP_COMMANDS "M155 S2"`), `AUTO_REPORT_POSITION`,
   `AUTO_REPORT_SD_STATUS`, `BUSY_WHILE_HEATING`, `REPORT_FAN_CHANGE`, and
   the `HOST_ACTION_COMMANDS` family — estimate aggregate bytes-per-second
   of unsolicited Marlin → host traffic during a typical print body. A
   Beagle-class proxy has a limited internal pipe; if async chatter exceeds
   the pipe's drain rate even briefly, line-numbered command/ack handshake
   desyncs.

4. **Recommend the single safest first revert.** The investigation doc
   recommends disabling `SERIAL_XON_XOFF` first. Agree, or is there a more
   diagnostic single-variable change? I want one change at a time, with a
   testable A/B between v10 and v11.

5. **Anything I should also instrument** — e.g. enable
   `SERIAL_STATS_RX_BUFFER_OVERRUNS` /
   `SERIAL_STATS_RX_FRAMING_ERRORS` / `SERIAL_STATS_DROPPED_RX` (currently
   commented at `Configuration_adv.h:422-424` in my fork) so the next test
   build emits diagnostic counters when the deadlock occurs?

**Constraints.**
- I will rebuild the fork with PlatformIO target `Artillery_Ruby`. The
  build is currently `v10 = 64.0% flash`, `58.4% RAM` (STM32F401RCT6),
  so I have headroom for diagnostic features.
- The fork is published at https://github.com/persano/Marlin (branch
  `artillery-genius-pro`) but local edits happen in
  `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\`.
- Do NOT touch the TFT firmware — that's a separate known issue I'm
  tracking elsewhere. This conversation is strictly about the
  Marlin↔Beagle serial protocol.

**Deliverable.** A short report (~300 words) with: ranking of the top
suspects (your version), specific config edits to try first in priority
order, and any diagnostic changes worth bundling into the v11 test build.
File:line citations for every config edit recommendation. Do NOT actually
write code yet — I want to see the recommendation before applying it.

---

## Files to attach to the new chat

1. `BEAGLE_STUCK_PRINT_INVESTIGATION.md` — the three-way config comparison.

## Files the new chat will read on its own

(Tell it the local path, no need to paste contents)

- `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\Marlin\Configuration.h`
- `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\Marlin\Configuration_adv.h`
- `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\Marlin\src\HAL\STM32\HardwareSerial.h`
- `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\Marlin\src\core\serial.h`
- `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\releases\v10\MERGE_REPORT.md`
- `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\README.md` (feature comparison
  tables, lines 12–135)
