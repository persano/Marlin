# Beagle Stuck-Print Investigation — Three-Way Configuration Comparison

## Context

**Symptom.** When printing through a Mintion Beagle camera unit acting as a USB-serial proxy between the slicer/host PC and the printer's Marlin MCU, the host gets stuck in an infinite resend loop on a single line number (e.g. repeatedly emitting `Resend: N87272` and never receiving an `ok` from Marlin). The print halts. Direct host-to-printer USB (no Beagle in line) is reported to work normally. The post-processor currently injects `M154 S0` (disable position auto-report) and `M155 S30` (slow temperature auto-report to 30 s) as a mitigation, which reduces but has not fully eliminated the symptom.

**Hardware setup.** Artillery Genius Pro and Genius Pro 1.5 boards (Marlin running on the Ruby/Mars motherboard; STM32-class MCU). BTT TFT-class touchscreen on the secondary serial port. Mintion Beagle camera inserted in-line on the USB serial path between the host PC and the Marlin USB-CDC endpoint.

**What is known so far.**
- The bug has been reproduced on **two different printers** running the same custom fork firmware ("v10") with **two different Beagle units**. This strongly suggests a firmware-side problem, not a hardware fault.
- The Beagle is a transparent serial proxy with limited internal buffering. Any firmware that floods the upstream serial channel with un-prompted async chatter (auto-reports, host-action lines, keepalive busy lines, etc.) can overrun the Beagle's pipe, which then desynchronises the line-number checksum protocol and causes the host to resend forever while Marlin keeps emitting unrelated chatter that the host treats as junk.
- Post-processor `M154 S0` + `M155 S30` injection helps but is not a complete fix, implying there are still other async sources running.

**Purpose of this document.** Compare the three Marlin baselines (the user's fork, the stock Artillery firmware, and pristine vanilla Marlin bugfix-2.1.x) on every config setting that can plausibly affect the serial-proxy host-to-Marlin half-duplex protocol. The goal is to identify which fork-specific divergences are the most likely deadlock root cause, so that this can be handed to a follow-up debugging pass.

> **Caveat on the "Vanilla Marlin" column.** The Configuration files inside `marlins/latest marlin release/Marlin-bugfix-2.1.x/Marlin/` are **not pristine** — they have been pre-staged with the user's fork values (they are effectively a copy of the v10 fork). The "Vanilla Marlin" column below therefore reflects the **upstream pristine defaults** as documented in the unmodified Marlin bugfix-2.1.x `Configuration_adv.h` shipped by MarlinFirmware, plus the platform HAL fallback values used when a setting is left undefined (e.g. `RX_BUFFER_SIZE`/`TX_BUFFER_SIZE` fall back to `128`/`32` for STM32 via `Marlin/src/HAL/STM32/HardwareSerial.h` and `Marlin/src/inc/Conditionals-4-adv.h`).

---

## Comparison

### Serial / Buffer Layer

| Setting | Stock Artillery | Vanilla Marlin | My fork (v10) | Risk |
|---|---|---|---|---|
| `BAUDRATE` | `250000` | `250000` | `250000` | — |
| `BAUDRATE_2` | `115200` (defined, second port) | not set (defaults to `BAUDRATE`) | not set (defaults to `BAUDRATE`) | LOW |
| `MAX_CMD_SIZE` | `96` | `96` | `96` | — |
| `BUFSIZE` | `4` | `4` | `32` | **HIGH** |
| `RX_BUFFER_SIZE` | `128` | `128` (HAL fallback) | `1024` | **HIGH** |
| `TX_BUFFER_SIZE` | `32` | `32` (HAL fallback) | `128` | MED |
| `SERIAL_XON_XOFF` | `off` (commented) | `off` (default) | **`on`** (auto-enabled via `#if RX_BUFFER_SIZE >= 1024`) | **HIGH** |
| `SERIAL_OVERRUN_PROTECTION` | `on` | `on` (default) | `on` | — |
| `EMERGENCY_PARSER` | `on` | `on` (default) | `on` | — |
| `FASTER_GCODE_PARSER` | `on` | `off` (default) | `on` | LOW |
| `NO_TIMEOUTS` | `off` (commented) | `off` (default, hint `1000`) | **`1000`** (ms) | MED |
| `ADVANCED_OK` | `off` (commented) | `off` (default) | **`on`** | **HIGH** |
| `MEATPACK_ON_SERIAL_PORT_1` | `off` | `off` | not set / `off` | — |
| `MEATPACK_ON_SERIAL_PORT_2` | `off` | `off` | not set / `off` | — |
| `BINARY_FILE_TRANSFER` | `off` (commented) | `off` (default) | not enabled | — |

### Auto-Reporting / Async Chatter

| Setting | Stock Artillery | Vanilla Marlin | My fork (v10) | Risk |
|---|---|---|---|---|
| `AUTO_REPORT_TEMPERATURES` | `on` | `on` (default) | `on` | — |
| `AUTO_REPORT_POSITION` | `off` (commented) | `off` (default) | **`on`** | **HIGH** |
| `AUTO_REPORT_SD_STATUS` | `off` (commented) | `off` (default) | **`on`** | **HIGH** |
| `DEFAULT_AUTO_REPORT_INTERVAL` / `AUTO_REPORT_TEMP_INTERVAL` | not defined (per-feature defaults apply) | not defined (per-feature defaults apply) | not defined (gated only by runtime `M155 S2` via `STARTUP_COMMANDS`) | MED |
| `STARTUP_COMMANDS` | `off` (commented, hint `"M17 Z"`) | `off` (default) | **`"M155 S2"`** | **HIGH** |
| `CAPABILITIES_REPORT` | implicit (gated only by `EXTENDED_*` in older code) | implicit | `on` (explicit) | — |
| `EXTENDED_CAPABILITIES_REPORT` | `on` | `on` (default) | `on` | — |
| `M115_GEOMETRY_REPORT` | `off` (commented) | `off` (default) | `on` | LOW |
| `M114_DETAIL` | `off` (commented) | `off` (default) | **`on`** | LOW |
| `M114_REALTIME` | `off` (commented) | `off` (default) | not set / `off` | — |
| `M114_LEGACY` | `off` (commented) | `off` (default) | not set / `off` | — |
| `REPORT_FAN_CHANGE` | `off` (commented) | `off` (default) | **`on`** | MED |

### Host Communication / Action Commands

| Setting | Stock Artillery | Vanilla Marlin | My fork (v10) | Risk |
|---|---|---|---|---|
| `HOST_ACTION_COMMANDS` | `off` (commented) | `off` (default) | **`on`** | MED |
| `HOST_PROMPT_SUPPORT` | `off` (commented) | `off` (default) | **`on`** | MED |
| `HOST_STATUS_NOTIFICATIONS` | not present | `on` (default when `HOST_ACTION_COMMANDS`) | **`on`** | MED |
| `HOST_KEEPALIVE_FEATURE` | `on` (in `Configuration.h`) | `on` (default in `Configuration.h`) | `on` | — |
| `DEFAULT_KEEPALIVE_INTERVAL` | `2` (s) | `2` (s) | `2` (s) | — |
| `BUSY_WHILE_HEATING` | `on` (in `Configuration.h`) | `on` (default) | `on` (in `Configuration_adv.h`) | — |

### SD / Printing

| Setting | Stock Artillery | Vanilla Marlin | My fork (v10) | Risk |
|---|---|---|---|---|
| `LONG_FILENAME_HOST_SUPPORT` | `off` (commented) | `off` (default) | **`on`** | LOW |
| `AUTO_REPORT_SD_STATUS` | `off` (commented) | `off` (default) | **`on`** | **HIGH** (duplicate, see above) |
| `SDCARD_CONNECTION` | not in user config (pin-file default) | not in user config (pin-file default) | `ONBOARD` (explicit) | — |

---

### File-line citations

**My fork (v10)** — `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\my-fork\Marlin\`
- `Configuration.h:102` — `BAUDRATE 250000`
- `Configuration_adv.h:248` — `LONG_FILENAME_HOST_SUPPORT`
- `Configuration_adv.h:252` — `AUTO_REPORT_SD_STATUS`
- `Configuration_adv.h:260` — `SDCARD_CONNECTION ONBOARD`
- `Configuration_adv.h:387` — `EMERGENCY_PARSER`
- `Configuration_adv.h:410` — `MAX_CMD_SIZE 96`
- `Configuration_adv.h:414` — `BUFSIZE 32`
- `Configuration_adv.h:415` — `TX_BUFFER_SIZE 128`
- `Configuration_adv.h:416` — `RX_BUFFER_SIZE 1024`
- `Configuration_adv.h:417–419` — `#if RX_BUFFER_SIZE >= 1024 → SERIAL_XON_XOFF`
- `Configuration_adv.h:421` — `NO_TIMEOUTS 1000`
- `Configuration_adv.h:426` — `ADVANCED_OK`
- `Configuration_adv.h:427` — `SERIAL_OVERRUN_PROTECTION`
- `Configuration_adv.h:428` — `FASTER_GCODE_PARSER`
- `Configuration_adv.h:442` — `AUTO_REPORT_TEMPERATURES`
- `Configuration_adv.h:443` — `AUTO_REPORT_POSITION`
- `Configuration_adv.h:447` — `BUSY_WHILE_HEATING`
- `Configuration_adv.h:453–457` — `CAPABILITIES_REPORT`, `EXTENDED_CAPABILITIES_REPORT`, `M115_GEOMETRY_REPORT`
- `Configuration_adv.h:467` — `STARTUP_COMMANDS "M155 S2"`
- `Configuration_adv.h:474` — `M114_DETAIL`
- `Configuration_adv.h:477` — `REPORT_FAN_CHANGE`
- `Configuration_adv.h:489–495` — `HOST_ACTION_COMMANDS` block (`HOST_PROMPT_SUPPORT`, `HOST_STATUS_NOTIFICATIONS`)
- `Configuration_adv.h:499–501` — `HOST_KEEPALIVE_FEATURE`, `DEFAULT_KEEPALIVE_INTERVAL 2`

**Stock Artillery** — `d:\Documentos\Marlin Firmware for Artillery Genius Pro\marlins\old stock fw - artillery genius pro\genius-pro-firmware-all-metal-main\Marlin\`
- `Configuration.h:118` — `BAUDRATE 250000`
- `Configuration.h:127` — `BAUDRATE_2 115200`
- `Configuration.h:1828` — `HOST_KEEPALIVE_FEATURE`
- `Configuration.h:1829` — `DEFAULT_KEEPALIVE_INTERVAL 2`
- `Configuration.h:1830` — `BUSY_WHILE_HEATING`
- `Configuration_adv.h:1427` — `//#define LONG_FILENAME_HOST_SUPPORT` (off)
- `Configuration_adv.h:1452` — `//#define AUTO_REPORT_SD_STATUS` (off)
- `Configuration_adv.h:1511` — `//#define BINARY_FILE_TRANSFER` (off)
- `Configuration_adv.h:2136` — `MAX_CMD_SIZE 96`
- `Configuration_adv.h:2137` — `BUFSIZE 4`
- `Configuration_adv.h:2146` — `TX_BUFFER_SIZE 32`
- `Configuration_adv.h:2152` — `RX_BUFFER_SIZE 128`
- `Configuration_adv.h:2157` — `//#define SERIAL_XON_XOFF` (off)
- `Configuration_adv.h:2184` — `EMERGENCY_PARSER`
- `Configuration_adv.h:2210` — `//#define NO_TIMEOUTS 1000` (off)
- `Configuration_adv.h:2213` — `//#define ADVANCED_OK` (off)
- `Configuration_adv.h:2217` — `SERIAL_OVERRUN_PROTECTION`
- `Configuration_adv.h:3562` — `AUTO_REPORT_TEMPERATURES`
- `Configuration_adv.h:3567` — `//#define AUTO_REPORT_POSITION` (off)
- `Configuration_adv.h:3572` — `EXTENDED_CAPABILITIES_REPORT`
- `Configuration_adv.h:3574` — `//#define M115_GEOMETRY_REPORT` (off)
- `Configuration_adv.h:3622–3624` — `//#define M114_DETAIL/_REALTIME/_LEGACY` (all off)
- `Configuration_adv.h:3626` — `//#define REPORT_FAN_CHANGE` (off)
- `Configuration_adv.h:3640` — `FASTER_GCODE_PARSER`
- `Configuration_adv.h:3647–3648` — MEATPACK port 1/2 off
- `Configuration_adv.h:3674` — `//#define STARTUP_COMMANDS "M17 Z"` (off)
- `Configuration_adv.h:3798–3800` — `HOST_ACTION_COMMANDS` and `HOST_PROMPT_SUPPORT` both off

**Vanilla Marlin bugfix-2.1.x** — pristine upstream defaults. The on-disk copy in `marlins/latest marlin release/Marlin-bugfix-2.1.x/Marlin/` has been pre-staged with the user's fork values and is **not** a clean baseline. The values in the Vanilla column above are the documented upstream defaults from MarlinFirmware/Marlin (bugfix-2.1.x), with HAL fallbacks from:
- `marlins/my-fork/Marlin/src/HAL/STM32/HardwareSerial.h:31,36` — `RX_BUFFER_SIZE 128`, `TX_BUFFER_SIZE 64`
- `marlins/my-fork/Marlin/src/HAL/DUE/MarlinSerial.h:57,60` — `RX_BUFFER_SIZE 128`, `TX_BUFFER_SIZE 32`
- `marlins/my-fork/Marlin/src/inc/Conditionals-4-adv.h:1186,1191` — generic fallback `RX_BUFFER_SIZE 128`, `TX_BUFFER_SIZE 32`

---

## What this means

**Top three suspects for the deadlock root cause:**

1. **`SERIAL_XON_XOFF` on (fork only).** Software flow control (XON/XOFF, i.e. ASCII `0x11`/`0x13`) is a known footgun over USB-CDC and through any transparent serial proxy. The Beagle is a USB-CDC pass-through with no awareness of XON/XOFF; if Marlin emits an `XOFF` byte because its 1024-byte RX is approaching full, the host may not honour it (CDC is supposed to be fully buffered) and the Beagle may also pass or eat those control bytes unpredictably. Worse, those control bytes are stripped or mangled by some USB stacks, and any G-code line containing a literal `0x11`/`0x13` is treated as flow control and corrupted. The fork auto-enables this purely because RX is 1024, which is exactly the wrong combination for a USB-CDC + proxy topology. **This is the single most likely root cause.**

2. **Auto-report flood: `AUTO_REPORT_POSITION` + `AUTO_REPORT_SD_STATUS` + `STARTUP_COMMANDS "M155 S2"` + `REPORT_FAN_CHANGE` + `HOST_ACTION_COMMANDS`/`HOST_STATUS_NOTIFICATIONS`/`HOST_PROMPT_SUPPORT`.** Both stock Artillery and vanilla Marlin disable all of these. The fork enables every one of them, and on boot it pre-arms M155 S2 (temperature report every 2 s) without the host having asked for it. The aggregate effect is that Marlin emits async chatter on multiple cadences (temp every 2 s, position every poll, SD every poll, fan-change events, host-action events, keepalive busy lines every 2 s while heating) — all of which transit the Beagle's limited proxy buffer alongside the line-numbered command/`ok` stream the host is trying to synchronise on. This is exactly the failure mode the post-processor's `M154 S0`/`M155 S30` mitigation targets — and the fact that mitigation only partially works tells you the other async sources (M27 SD status, fan-change, host-action notifications) are still firing.

3. **`ADVANCED_OK` on (fork only).** This changes the `ok` reply to include line number, planner space, and queue info (e.g. `ok N123 P15 B3`). Most hosts handle it, but some serial proxies and older host stacks key off the exact byte length or trailing-byte position of a bare `ok`. If the Beagle parses `ok` lines (for its own state machine, e.g. to know when a print is progressing) and is naive about the extended form, the protocol-level handshake can desync.

**Settings that diverge from BOTH baselines but are probably fine:**
- `BUFSIZE 32` and `TX_BUFFER_SIZE 128`. Larger TX/queue is benign — it can only reduce, not increase, host-side resend pressure.
- `RX_BUFFER_SIZE 1024` is fine **by itself** — the danger is purely that it triggers the `SERIAL_XON_XOFF` auto-enable. If XON/XOFF is disabled, 1024 RX is strictly an improvement.
- `NO_TIMEOUTS 1000`, `FASTER_GCODE_PARSER`, `M114_DETAIL`, `M115_GEOMETRY_REPORT`, `LONG_FILENAME_HOST_SUPPORT` — none of these emit async traffic; they only change responses to explicit queries.

**Recommended next experiment (single-variable test).** Disable `SERIAL_XON_XOFF` first (either by raising the gate above 1024 or by explicitly `#undef SERIAL_XON_XOFF` after the auto-enable). If the deadlock persists, disable `AUTO_REPORT_SD_STATUS` and `REPORT_FAN_CHANGE` next. If it still persists, disable `HOST_ACTION_COMMANDS` block entirely. Re-enable one at a time once a clean run is achieved.
