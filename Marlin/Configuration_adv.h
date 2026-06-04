/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * Artillery Genius Pro — Configuration_adv.h
 * Base: Marlin bugfix-2.1.x (HEX 02010300, dated 2026-05-01)
 *
 * Place this file in: Marlin-bugfix-2.1.x/Marlin/Configuration_adv.h
 * Do NOT modify any Marlin source files.
 */

#define CONFIGURATION_ADV_H_VERSION 02010300

//===========================================================================
//============================= Thermal Settings ============================
//===========================================================================

// @section temperature

/**
 * Bed temperature residency time
 * ADDED: per user specification (increased from default 10 to 15s)
 * REASON: more stable bed temperature before leveling begins
 */
#define TEMP_BED_RESIDENCY_TIME 15   // ADDED: was default 10s | REASON: more stable bed temp before probing
#define TEMP_BED_WINDOW          1   // (°C) Temperature proximity for the "ready" timer
#define TEMP_BED_HYSTERESIS      3   // (°C) Temperature proximity considered "close enough" to the target

//===========================================================================
//==================== Z Steppers & Tramming ================================
//===========================================================================

// @section motion

/**
 * Z Stepper Auto-Alignment — DISABLED
 * SOURCE: user specification | REASON: Genius Pro has a single Z driver for both Z steppers;
 * auto-alignment requires independent drivers and would misfire on this hardware.
 */
//#define Z_STEPPER_AUTO_ALIGN

/**
 * Assisted Tramming
 * ADDED: per user specification | REASON: helps manual bed leveling at the corners
 */
#define ASSISTED_TRAMMING
#if ENABLED(ASSISTED_TRAMMING)
  // Screw thread type: M3_CW=30, M3_CCW=31, M4_CW=40, M4_CCW=41, M5_CW=50, M5_CCW=51
  #define TRAMMING_SCREW_THREAD 30  // M3 clockwise (standard on Genius Pro)
  // X coords adjusted for PROBE_OFFSET_X=27.25: probe X=35 → nozzle X=7.75 (within bounds)
  #define TRAMMING_POINT_XY { { 35, 15 }, { 185, 15 }, { 185, 200 }, { 35, 200 } }
  #define TRAMMING_POINT_NAME_1 "Front-Left"
  #define TRAMMING_POINT_NAME_2 "Front-Right"
  #define TRAMMING_POINT_NAME_3 "Back-Right"
  #define TRAMMING_POINT_NAME_4 "Back-Left"

  #define RESTORE_LEVELING_AFTER_G35   // Restore leveling after tramming
  //#define REPORT_TRAMMING_MM          // Report Z deviation in mm instead of turns
  //#define ASSISTED_TRAMMING_WIZARD    // Add a Tramming Wizard to the LCD menu

  //#define ASSISTED_TRAMMING_WAIT_POSITION { X_CENTER, Y_CENTER, 30 }
#endif

//===========================================================================
//====================== Input Shaping =====================================
//===========================================================================

// @section motion

/**
 * Input Shaping (ZV filter)
 * ADDED: per user specification
 *
 * Frequencies measured on Sidewinder X2 via ADXL345 accelerometer.
 * The Genius Pro has a similar but not identical frame — re-measure with ADXL345
 * on your specific machine for best results. These are a calibrated starting point only.
 *
 * To re-measure: enable M593, connect ADXL345, run resonance test, update values, M500.
 */
#define INPUT_SHAPING_X   // ADDED: X-axis input shaping
#define INPUT_SHAPING_Y   // ADDED: Y-axis input shaping
//#define INPUT_SHAPING_Z // Not needed — Z is a leadscrew, not subject to resonance

#if ANY(INPUT_SHAPING_X, INPUT_SHAPING_Y, INPUT_SHAPING_Z)
  #if ENABLED(INPUT_SHAPING_X)
    #define SHAPING_FREQ_X  55.0  // ADDED: (Hz) measured on SW2; re-measure on Genius Pro for best results
    #define SHAPING_ZETA_X   0.1  // (linear) Damping factor; tune with SHAPING_MENU after measuring
  #endif
  #if ENABLED(INPUT_SHAPING_Y)
    #define SHAPING_FREQ_Y  48.6  // ADDED: (Hz) measured on SW2; re-measure on Genius Pro for best results
    #define SHAPING_ZETA_Y   0.1  // (linear) Damping factor; tune with SHAPING_MENU after measuring
  #endif
  //#if ENABLED(INPUT_SHAPING_Z)
  //  #define SHAPING_FREQ_Z  40.0
  //  #define SHAPING_ZETA_Z   0.1
  //#endif

  #define SHAPING_MENU  // ADDED: adds input shaping submenu to LCD for live tuning
#endif

//===========================================================================
//====================== Adaptive Step Smoothing ============================
//===========================================================================

// @section motion

/**
 * Adaptive Step Smoothing
 * ADDED: per user specification
 * NOTE: in bugfix-2.1.x this define lives in Configuration_adv.h, not Configuration.h
 */
#define ADAPTIVE_STEP_SMOOTHING

//===========================================================================
//========================= Power Loss Recovery =============================
//===========================================================================

// @section power-loss

/**
 * Power Loss Recovery
 * ADDED: per user specification
 *
 * WARNING: Test on a SACRIFICIAL PRINT before trusting this feature.
 * Some boards (including certain Ruby revisions) may have issues with the
 * PLR pin or SD card state after power restoration. Verify that:
 *   1. The printer correctly detects the power loss event
 *   2. The resume position is accurate (no Z offset drift)
 *   3. The nozzle does not drag across the print on resume
 * Do NOT rely on this feature for important prints until personally verified.
 */
#define POWER_LOSS_RECOVERY
#if ENABLED(POWER_LOSS_RECOVERY)
  #define PLR_ENABLED_DEFAULT   false // Power Loss Recovery enabled by default? (M413 S<0,1> to override)
  //#define POWER_LOSS_PIN        44  // Pin to detect power loss. Check pins_ARTILLERY_RUBY.h.
  //#define POWER_LOSS_STATE     HIGH // State of pin indicating power loss
  //#define POWER_LOSS_PULL           // Set pullup / pulldown as appropriate
  //#define POWER_LOSS_PURGE_LEN   20 // (mm) Length of filament to purge on resume
  //#define POWER_LOSS_RETRACT_LEN 10 // (mm) Length of filament to retract on fail. Requires backup power.

  #define POWER_LOSS_MIN_Z_CHANGE 0.05 // (mm) Minimum Z change before saving power-loss data
#endif

//===========================================================================
//========================= Babystepping ====================================
//===========================================================================

// @section motion

/**
 * Babystepping
 * ADDED: per user specification | REASON: fine-tune Z offset during a live print
 */
#define BABYSTEPPING
#if ENABLED(BABYSTEPPING)
  //#define EP_BABYSTEPPING                // M293/M294 babystepping with EMERGENCY_PARSER
  //#define BABYSTEP_WITHOUT_HOMING
  //#define BABYSTEP_ALWAYS_AVAILABLE      // Allow babystepping even when not printing (M290)
  #define BABYSTEP_ALWAYS_AVAILABLE        // ADDED: enables babystepping any time, not just during prints
  //#define DOUBLECLICK_FOR_Z_BABYSTEPPING // Double-click on the status screen to babystep Z
  //#define DOUBLECLICK_MAX_INTERVAL 1250  // (ms) Maximum interval between double-clicks
  //#define BABYSTEP_DISPLAY_TOTAL         // Display total babysteps since last G28
  //#define BABYSTEP_ZPROBE_OFFSET         // Combine M851 Z and babystepping
  //#define BABYSTEP_HOTEND_Z_OFFSET       // For multiple hotends, babystep relative Z offsets
  //#define BABYSTEP_GFX_OVERLAY           // Enable graphical overlay on z-offset editor
  #define BABYSTEP_MULTIPLICATOR_Z  1    // SOURCE: custom fw artillery genius pro (gpro-mp) | (steps or mm) one step per babystep trigger
#endif

//===========================================================================
//==================== Linear Advance ======================================
//===========================================================================

// @section extruder

/**
 * Linear Advance
 * ADDED: per user specification
 * K=0.13 is a starting value for direct drive. Print a K-factor tower starting
 * at 0.0 and increasing by 0.05 increments to find your optimal K value.
 * Then set with M900 K<value> and save with M500.
 *
 * NOTE: LIN_ADVANCE 1.5 K values differ from earlier versions — do not reuse old values.
 * NOTE: ALLOW_LOW_EJERK is deprecated in bugfix-2.1.x (compile error if used).
 *       Not needed when CLASSIC_JERK is disabled — JUNCTION_DEVIATION handles E jerk.
 * NOTE: EXPERIMENTAL_SCURVE is deprecated in bugfix-2.1.x (compile error if used).
 *       S_CURVE_ACCELERATION is the correct define (set in Configuration.h).
 */
#define LIN_ADVANCE
#if ENABLED(LIN_ADVANCE)
  #define ADVANCE_K 0.13  // ADDED: starting K value for Genius Pro direct drive; calibrate with K-factor tower
  //#define ADVANCE_K_EXTRA           // Add a second linear advance constant
  //#define LA_DEBUG                  // Print debug info to serial
  //#define EXPERIMENTAL_SCURVE       // DEPRECATED — removed per Changes.h line 632-633
  //#define SMOOTH_LIN_ADVANCE        // Remove limits on acceleration by gradual K reduction
#endif

//===========================================================================
//====================== Arc Support ========================================
//===========================================================================

// @section gcode

/**
 * Arc Support
 * NOTE: ARC_SUPPORT is in Configuration_adv.h in bugfix-2.1.x (not Configuration.h)
 * ADDED: per user specification
 */
#define ARC_SUPPORT
#if ENABLED(ARC_SUPPORT)
  #define MIN_ARC_SEGMENT_MM      0.1  // (mm) Minimum length of each arc segment
  #define MAX_ARC_SEGMENT_MM      1.0  // (mm) Maximum length of each arc segment
  #define MIN_CIRCLE_SEGMENTS    72    // Minimum number of segments in a complete circle
  #define N_ARC_CORRECTION       25    // Number of interpolated segments between corrections
  #define ARC_P_CIRCLES              // ADDED: enables the 'P' parameter to specify complete circles
  //#define SF_ARC_FIX               // Enable only if using 'ArcWelder' with older firmware
#endif

//===========================================================================
//====================== SD Card & Media ====================================
//===========================================================================

// @section media

/**
 * Long filename support — needed for BTT-TFT compatibility
 * ADDED: per user specification (BTT TFT support)
 */
#define LONG_FILENAME_HOST_SUPPORT  // ADDED: G-code access to long filenames from TFT
//#define LONG_FILENAME_WRITE_SUPPORT

// Auto-report SD status — required by TFT for M27 polling
#define AUTO_REPORT_SD_STATUS  // ADDED: BTT TFT support — TFT polls SD status with M27

/**
 * SD Card Connection — ONBOARD
 * ADDED: per user specification | REASON: Artillery Ruby board uses onboard SD reader
 * Options: LCD, ONBOARD, CUSTOM_CABLE
 * NOTE: SDCARD_CONNECTION is in Configuration_adv.h, not Configuration.h
 */
#define SDCARD_CONNECTION ONBOARD

//===========================================================================
//====================== Bed Leveling (Bilinear subdivision) ================
//===========================================================================

// @section bed leveling

/**
 * Bilinear Subdivision
 * NOTE: ABL_BILINEAR_SUBDIVISION has NO EFFECT when AUTO_BED_LEVELING_UBL is active.
 * Included here for completeness — if UBL is later disabled and BILINEAR re-enabled,
 * this setting will then take effect.
 * SOURCE: user specification (include for completeness)
 */
//#define ABL_BILINEAR_SUBDIVISION
#if ENABLED(ABL_BILINEAR_SUBDIVISION)
  #define BILINEAR_SUBDIVISIONS 3
#endif

//===========================================================================
//====================== Progress & Status ==================================
//===========================================================================

// @section interface

/**
 * Progress display
 * NOTE: LCD_SET_PROGRESS_MANUALLY was renamed to SET_PROGRESS_MANUALLY in bugfix-2.1.x.
 *       Using the old name causes a compile error.
 * RENAMED in Marlin bugfix-2.1.x: was LCD_SET_PROGRESS_MANUALLY, now SET_PROGRESS_MANUALLY
 */
#define SET_PROGRESS_MANUALLY   // RENAMED in Marlin bugfix-2.1.x: was LCD_SET_PROGRESS_MANUALLY
#if ENABLED(SET_PROGRESS_MANUALLY)
  #define SET_PROGRESS_PERCENT    // Add M73 G-code to set display progress percentage
  #define SET_REMAINING_TIME      // Add M73 G-code to set remaining print time
  //#define M73_REPORT            // REMOVED: was spamming TFT with notifications every minute during SD prints
  #if ALL(M73_REPORT, HAS_MEDIA)
    //#define M73_REPORT_SD_ONLY  // Report only when printing from SD card
  #endif
#endif

#if HAS_DISPLAY && ANY(HAS_MEDIA, SET_PROGRESS_MANUALLY)
  #define SHOW_REMAINING_TIME     // ADDED: display estimated remaining time in status bar
  #if ENABLED(SHOW_REMAINING_TIME)
    //#define ROTATE_PROGRESS_DISPLAY  // Cycle between progress bar and remaining time
  #endif
#endif

//===========================================================================
//====================== Pause & Filament ===================================
//===========================================================================

// @section extruder

/**
 * Advanced Pause Feature
 * ADDED: per user specification | REASON: enables filament change and park/resume
 * Requires NOZZLE_PARK_FEATURE in Configuration.h (confirmed enabled).
 */
#define ADVANCED_PAUSE_FEATURE
#if ENABLED(ADVANCED_PAUSE_FEATURE)
  #define PAUSE_PARK_RETRACT_FEEDRATE         60  // (mm/s) Initial retract feedrate
  #define PAUSE_PARK_RETRACT_LENGTH            2  // (mm) Initial retract
  #define FILAMENT_CHANGE_UNLOAD_FEEDRATE     10  // (mm/s) Unload filament feedrate
  #define FILAMENT_CHANGE_UNLOAD_ACCEL        25  // (mm/s²) Lower acceleration for unload
  #define FILAMENT_CHANGE_UNLOAD_LENGTH      100  // (mm) Unload filament length
  #define FILAMENT_CHANGE_SLOW_LOAD_FEEDRATE   6  // (mm/s) Slow load feedrate
  #define FILAMENT_CHANGE_SLOW_LOAD_LENGTH     0  // (mm) Slow load length (0 = disabled)
  #define FILAMENT_CHANGE_FAST_LOAD_FEEDRATE  10  // (mm/s) Fast load feedrate
  #define FILAMENT_CHANGE_FAST_LOAD_ACCEL     25  // (mm/s²) Lower acceleration for load
  #define FILAMENT_CHANGE_FAST_LOAD_LENGTH     0  // (mm) Fast load length (0 = disabled)
  #define ADVANCED_PAUSE_PURGE_FEEDRATE        3  // (mm/s) Purge feedrate after load
  #define ADVANCED_PAUSE_PURGE_LENGTH         50  // (mm) Purge length after load
  #define ADVANCED_PAUSE_RESUME_PRIME          0  // (mm) Extra prime on resume
  //#define ADVANCED_PAUSE_FANS_PAUSE          // Turn off fans when paused

  #define FILAMENT_UNLOAD_PURGE_RETRACT        13 // (mm) Retract before unload purge
  #define FILAMENT_UNLOAD_PURGE_DELAY        5000 // (ms) Delay for melting
  #define FILAMENT_UNLOAD_PURGE_LENGTH          8 // (mm) Purge before unload
  #define FILAMENT_UNLOAD_PURGE_FEEDRATE       25 // (mm/s) Purge feedrate

  #define PAUSE_PARK_NO_STEPPER_TIMEOUT          // Keep steppers enabled during pause
  #define PAUSE_PARK_NOZZLE_TIMEOUT           45 // (s) Nozzle timeout before turning off for safety
  #define FILAMENT_CHANGE_ALERT_BEEPS         10 // Number of alert beeps before timeout
  //#define FILAMENT_CHANGE_RESUME_ON_INSERT      // Resume automatically when filament detected

  #define PARK_HEAD_ON_PAUSE   // ADDED: park nozzle when M25 / M0 pauses the print

  #define FILAMENT_LOAD_UNLOAD_GCODES  // ADDED: adds M701 (load) / M702 (unload) G-codes
#endif

// @section gcode

/**
 * Cancel Objects (M486)
 * ADDED: per user specification | REASON: allows skipping failed objects mid-print
 * NOTE: in bugfix-2.1.x, CANCEL_OBJECTS is in Configuration_adv.h
 */
#define CANCEL_OBJECTS
#if ENABLED(CANCEL_OBJECTS)
  #define CANCEL_OBJECTS_REPORTING  // Emit the current object as a status message
#endif

/**
 * G-code Macros (M810-M819)
 * ADDED: per user specification | REASON: store frequently used G-code sequences in EEPROM
 * NOTE: in bugfix-2.1.x, GCODE_MACROS is in Configuration_adv.h
 */
#define GCODE_MACROS
#if ENABLED(GCODE_MACROS)
  #define GCODE_MACROS_SLOTS       5  // Up to 10 slots
  #define GCODE_MACROS_SLOT_SIZE  50  // Max characters per slot
  //#define GCODE_MACROS_IN_EEPROM  // Store macros in EEPROM
#endif

//===========================================================================
//====================== Buffers & Communication ============================
//===========================================================================

// @section serial

/**
 * Emergency Parser
 * ADDED: per user specification (BTT TFT support) | REASON: allows M108, M112, M410 even
 * when the planner buffer is full — critical for TFT pause/cancel commands to be received
 */
#define EMERGENCY_PARSER

/**
 * Serial float precision
 * ADDED: per user specification (BTT TFT support) | REASON: TFT parses position data
 */
#define SERIAL_FLOAT_PRECISION 4

/**
 * M575 - Change serial baud rate at runtime
 * ADDED: per user request | REASON: lets host/TFT switch baud (e.g. 115200 ↔ 250000)
 * without reflashing. Accepted bauds: 2400, 9600, 19200, 38400, 57600, 115200,
 * 250000, 500000, 1000000 (also 24/96/19/38/57/115/250/500/1000 shortcut codes).
 */
#define BAUD_RATE_GCODE

// @section buffers

/**
 * Buffer sizes
 * ADDED: per user specification | REASON: larger buffers prevent starvation on large G-code
 * files and improve TFT communication reliability
 */
#define MAX_CMD_SIZE 96         // SOURCE: custom fw artillery genius pro (gpro-mp) | max serial command length (bytes)
#define PROPORTIONAL_FONT_RATIO 1.0 // SOURCE: custom fw artillery genius pro (gpro-mp) | spacing ratio for proportional font (no LCD = 1.0)
#define BLOCK_BUFFER_SIZE 16  // SOURCE: custom fw artillery genius pro (gpro-mp) | (power of 2, 8–32) motion planning buffer
#define MIN_STEPS_PER_SEGMENT 6 // SOURCE: custom fw artillery genius pro (gpro-mp) | replaces deprecated 'dropsegments' (was 5, +1 per Changes.h)
#define BUFSIZE 32        // ADDED: was default 4 | REASON: prevent buffer starvation with TFT
#define TX_BUFFER_SIZE 128 // ADDED: was default 0 | REASON: improve serial TX throughput to TFT
#define RX_BUFFER_SIZE 1024 // ADDED: was default 128 | REASON: handle large TFT status payloads
#if RX_BUFFER_SIZE >= 1024
  #define SERIAL_XON_XOFF   // Software flow control — recommended with large RX buffer
#endif

#define NO_TIMEOUTS 1000   // ADDED: (ms) disable timeout errors during TFT communication pauses
//#define SERIAL_STATS_RX_BUFFER_OVERRUNS
//#define SERIAL_STATS_RX_FRAMING_ERRORS
//#define SERIAL_STATS_DROPPED_RX

#define ADVANCED_OK  // ADDED: extend OK response with line number, planner space, and queue info
#define SERIAL_OVERRUN_PROTECTION  // Throttle when serial buffer is full to prevent data loss
#define FASTER_GCODE_PARSER  // Use a smaller, faster G-code parser
#define PAREN_COMMENTS       // Support (comments in parentheses) — used by Simplify3D and some post-processors
#define GCODE_QUOTED_STRINGS // Support quoted string parameters, e.g. M117 "message"

// v11 diagnostics — both dormant in normal operation (zero steady-state
// chatter) so they do not contaminate the Beagle A/B; they only emit on
// explicit trigger or MCU fault.
#define DEBUG_FLAGS_GCODE    // Enable M111 runtime debug-mask handling. Marlin upstream
                             // emits a build warning when this is off ("Some hosts rely
                             // on it"). Output is gated by M111 Sn from the host — by
                             // default M111 is OFF, so nothing fires unless we ask for it.
#define POSTMORTEM_DEBUGGING // On HardFault/UsageFault/MemManage/BusFault, dump CPU
                             // registers + stack trace via MinSerial (HAL/STM32/
                             // MinSerial.cpp). Only fires on MCU fault — silent in
                             // normal operation. Tells us whether a future Beagle
                             // deadlock is a soft-crash that USE_WATCHDOG is masking.
                             // REQUIRES -flto disabled (see ini/stm32f4.ini comment).

//===========================================================================
//====================== Auto-Reporting =====================================
//===========================================================================

// @section reporting

/**
 * Auto-report features — mandatory for BTT TFT touchscreen operation
 * ADDED: per user specification
 */
#define AUTO_REPORT_TEMPERATURES  // Already default-enabled in latest Marlin; confirmed present
#define AUTO_REPORT_POSITION      // ADDED: auto-report position with M114 — used by TFT
#if ENABLED(AUTO_REPORT_POSITION)
  //#define AUTO_REPORT_REAL_POSITION  // Auto-report in real coordinates (after applying offsets)
#endif
#define BUSY_WHILE_HEATING  // Send busy status to host while waiting for temperature

// M115 firmware info + capabilities report
// CAPABILITIES_REPORT gates the entire M115.cpp — must be defined for M115 to exist.
// EXTENDED_CAPABILITIES_REPORT enables the Cap:... lines within M115.
// These were split in a later bugfix-2.1.x commit; older configs only had EXTENDED_.
#define CAPABILITIES_REPORT
#if ENABLED(CAPABILITIES_REPORT)
  #define EXTENDED_CAPABILITIES_REPORT
  #if ENABLED(EXTENDED_CAPABILITIES_REPORT)
    #define M115_GEOMETRY_REPORT  // include geometry info in M115 response
  #endif
#endif

#define PINS_DEBUGGING  // Enable M43 to report pin state and toggle pins
#define EDITABLE_STEPS_PER_UNIT  // Enable M92 to set steps-per-unit at runtime

// Auto-enable temperature reporting on boot so the TFT touchscreen gets live data
// without requiring M155 S2 from slicer start G-code.
// MarlinCore.cpp injects this into the command queue at the end of setup().
// v11 A/B: disabled — pre-arms temp auto-report before slicer start-gcode can run,
// fires async chatter through the Beagle USB-CDC proxy and is the strongest single
// suspect for the Resend-loop deadlock. TFT issues its own M155 S2 on connect via
// SERIAL_PORT_2, so live temps on the touchscreen are unaffected.
//#define STARTUP_COMMANDS "M155 S2"
#define USE_WATCHDOG  // Hardware watchdog — resets if firmware hangs for >8s
#define SLOWDOWN       // Slow down machine if look-ahead buffer is nearly full
#define SLOWDOWN_DIVISOR 2
#define MULTISTEPPING_LIMIT 16  // Match reference firmware (auto-default was 128)

// M114 detail
#define M114_DETAIL  // ADDED: enables M114 D for detailed position data including planner state

// Fan change reporting
#define REPORT_FAN_CHANGE  // ADDED: report fan speed to host when changed with M106

//===========================================================================
//====================== Host Action Commands ================================
//===========================================================================

// @section host

/**
 * Host Action Commands — mandatory for BTT TFT touchscreen operation
 * ADDED: per user specification | REASON: enables pause/cancel/resume handshake with TFT
 */
#define HOST_ACTION_COMMANDS
#if ENABLED(HOST_ACTION_COMMANDS)
  #define HOST_PROMPT_SUPPORT          // ADDED: allow host (TFT) to respond to Marlin prompts
  #if ENABLED(HOST_PROMPT_SUPPORT)
    //#define EMERGENCY_PARSER_FOR_PROMPT // Use EMERGENCY_PARSER for host prompts
  #endif
  #define HOST_STATUS_NOTIFICATIONS    // ADDED: send status messages to TFT for display
  //#define HOST_START_MENU_ITEM       // Add a menu item that does host start
  //#define HOST_SHUTDOWN_MENU_ITEM    // Add a menu item to trigger a host shutdown
#endif
#define HOST_KEEPALIVE_FEATURE  // Send busy/idle status to host so it knows Marlin is alive
#if ENABLED(HOST_KEEPALIVE_FEATURE)
  #define DEFAULT_KEEPALIVE_INTERVAL 2  // Seconds between busy status messages
#endif

//===========================================================================
//====================== SD / Long Filename (Additional) ====================
//===========================================================================

// @section media

// Scroll long filenames in the file menu
//#define SCROLL_LONG_FILENAMES

//===========================================================================
//====================== G-code Features ====================================
//===========================================================================

// @section gcode

// Minimum planner junction speed
//#define MINIMUM_STEPPER_POST_DIR_DELAY 0

// Fan control
#define FAN_KICKSTART_TIME   100   // (ms) Spin fans up to full speed for this long before reducing to target PWM
//#define FAN_SOFT_PWM
//#define SOFT_PWM_SCALE 0

//===========================================================================
//====================== Filament Width Sensor ==============================
//===========================================================================

//#define FILAMENT_WIDTH_SENSOR

//===========================================================================
//====================== Additional Motion Features =========================
//===========================================================================

// @section motion

// Motor stall detection (SENSORLESS_HOMING requires TMC2130/2160/2209/5130/5160)
// Not applicable with TMC2100 — see TMC2100 note in Configuration.h
//#define SENSORLESS_HOMING

//===========================================================================
//====================== Fixed-Time Motion (FT Motion) ======================
//===========================================================================

// @section motion

/**
 * Fixed-Time Motion (FT Motion) — BETA FEATURE
 * ADDED: per user specification
 *
 * FT Motion uses a fixed-time trajectory planner with its own built-in shaping.
 * Standard motion (INPUT_SHAPING_X/Y) remains compiled in alongside FT Motion.
 * Use M493 S1 to enable FT Motion, M493 S0 to return to standard motion.
 *
 * Shaping frequencies carry over from the INPUT_SHAPING values above.
 * Re-measure with ADXL345 on this specific machine and update DEFAULT_FREQ values.
 * Enable/disable at runtime: M493 S<0/1>
 * Adjust shaper: M493 A<axis> S<type> F<hz> Z<zeta>
 */
#define FT_MOTION
#if ENABLED(FT_MOTION)
  //#define FTM_IS_DEFAULT_MOTION               // Make FT Motion the power-on default (M493 not needed after boot)
  //#define NO_STANDARD_MOTION                  // Remove standard planner entirely (saves Flash/RAM; disable INPUT_SHAPING_X/Y first)

  // Shaper algorithms compiled into the binary — comment out unused ones to save Flash
  #define FTM_SHAPER_ZV
  #define FTM_SHAPER_ZVD
  //#define FTM_SHAPER_ZVDD
  //#define FTM_SHAPER_ZVDDD
  //#define FTM_SHAPER_EI
  //#define FTM_SHAPER_2HEI
  //#define FTM_SHAPER_3HEI
  #define FTM_SHAPER_MZV

  // X axis — initial values carried from INPUT_SHAPING_X above; re-measure on Genius Pro
  #define FTM_DEFAULT_SHAPER_X      ftMotionShaper_ZV  // (NONE, ZV, ZVD, ZVDD, ZVDDD, EI, 2HEI, 3HEI, MZV)
  #define FTM_SHAPING_DEFAULT_FREQ_X   55.0f           // (Hz) from SHAPING_FREQ_X
  #define FTM_SHAPING_ZETA_X            0.1f           // from SHAPING_ZETA_X

  // Y axis — initial values carried from INPUT_SHAPING_Y above; re-measure on Genius Pro
  #define FTM_DEFAULT_SHAPER_Y      ftMotionShaper_ZV  // (NONE, ZV, ZVD, ZVDD, ZVDDD, EI, 2HEI, 3HEI, MZV)
  #define FTM_SHAPING_DEFAULT_FREQ_Y   48.6f           // (Hz) from SHAPING_FREQ_Y
  #define FTM_SHAPING_ZETA_Y            0.1f           // from SHAPING_ZETA_Y

  // Z axis — leadscrew drive, no shaping needed
  #define FTM_DEFAULT_SHAPER_Z      ftMotionShaper_NONE
  #define FTM_SHAPING_DEFAULT_FREQ_Z   21.0f
  #define FTM_SHAPING_ZETA_Z            0.03f

  // Extruder axis
  #define FTM_DEFAULT_SHAPER_E      ftMotionShaper_NONE
  #define FTM_SHAPING_DEFAULT_FREQ_E   21.0f
  #define FTM_SHAPING_ZETA_E            0.03f

  // Trajectory profile
  #define FTM_POLYS                              // Include POLY5/POLY6 support (~3k Flash overhead)
  #if ENABLED(FTM_POLYS)
    #define FTM_TRAJECTORY_TYPE TRAPEZOIDAL      // TRAPEZOIDAL (continuous velocity), POLY5, or POLY6 (S-curve)
    #define FTM_POLY6_ACCELERATION_OVERSHOOT 1.875f // Max acceleration overshoot for POLY6 (1.25–1.875)
  #endif

  // Core timing
  #define FTM_BUFFER_SIZE  256   // (entries) Power of 2 required; 256 = 256 ms buffer at 1 kHz (was 128 — doubled for Beagle deadlock investigation, in case host-side stalls were starving the FT_MOTION planner)
  #define FTM_FS          1000   // (Hz) Trajectory generation rate
  #define FTM_MIN_SHAPE_FREQ  20 // (Hz) Minimum shaping frequency; lower values consume more RAM

#endif // FT_MOTION

//===========================================================================
//====================== Summary ============================================
//===========================================================================
//
// Features enabled in this file:
//   TEMP_BED_RESIDENCY_TIME 15
//   Z_STEPPER_AUTO_ALIGN    DISABLED
//   ASSISTED_TRAMMING
//   INPUT_SHAPING_X (SHAPING_FREQ_X 55.0 Hz)
//   INPUT_SHAPING_Y (SHAPING_FREQ_Y 48.6 Hz)
//   SHAPING_MENU
//   ADAPTIVE_STEP_SMOOTHING
//   POWER_LOSS_RECOVERY (with disclaimer)
//   BABYSTEPPING + BABYSTEP_ALWAYS_AVAILABLE
//   LIN_ADVANCE (ADVANCE_K 0.13)
//   ARC_SUPPORT + ARC_P_CIRCLES
//   LONG_FILENAME_HOST_SUPPORT
//   AUTO_REPORT_SD_STATUS
//   SDCARD_CONNECTION ONBOARD
//   ABL_BILINEAR_SUBDIVISION (included, no effect with UBL active)
//   SET_PROGRESS_MANUALLY (renamed from LCD_SET_PROGRESS_MANUALLY)
//   SHOW_REMAINING_TIME
//   M73_REPORT              DISABLED (was spamming TFT every minute)
//   ADVANCED_PAUSE_FEATURE + PARK_HEAD_ON_PAUSE + FILAMENT_LOAD_UNLOAD_GCODES
//   CANCEL_OBJECTS
//   GCODE_MACROS
//   PAREN_COMMENTS
//   GCODE_QUOTED_STRINGS
//   FAN_KICKSTART_TIME 100
//   EMERGENCY_PARSER
//   SERIAL_FLOAT_PRECISION 4
//   BUFSIZE 32
//   TX_BUFFER_SIZE 128
//   RX_BUFFER_SIZE 1024
//   NO_TIMEOUTS 1000
//   ADVANCED_OK
//   AUTO_REPORT_TEMPERATURES
//   AUTO_REPORT_POSITION
//   EXTENDED_CAPABILITIES_REPORT
//   M115_GEOMETRY_REPORT
//   M114_DETAIL
//   REPORT_FAN_CHANGE
//   HOST_ACTION_COMMANDS + HOST_PROMPT_SUPPORT + HOST_STATUS_NOTIFICATIONS
//   FT_MOTION (ZV default; ZV+ZVD+MZV compiled in; SHAPING_FREQ_X=55.0 Hz, Y=48.6 Hz)
//
//===========================================================================
