# TFT Firmware Files — GD32F305 Only

> [!WARNING]
> **These files are for the GD32F305 variant of the TFT28 only.** The Artillery Genius Pro may ship with either a GD32F305 or an STM32 screen. Flashing the wrong binary will brick your screen. Verify your chip before proceeding — see [the upgrade guide](../README.md#step-0--verify-your-screen-chip) for how to check.

## Files in this folder

| File | Purpose |
|------|---------|
| `MKSTFT28EVO.bin` | TFT firmware binary (GD32F305) |
| `config.ini` | Pre-configured settings for the Artillery Genius Pro |

## What you still need

This folder does **not** include a language file or a theme — you choose those yourself from the community repository:

**[kisslorand/BTT-TFT-FW — Copy to SD Card root directory to update](https://github.com/kisslorand/BTT-TFT-FW/tree/main/Copy%20to%20SD%20Card%20root%20directory%20to%20update)**

From there, download:

- **A language file** — named `language_XX.ini` (e.g. `language_en.ini` for English, `language_es.ini` for Spanish)
- **A theme folder** — named `TFT28/`, containing `bmp/` and `font/` subfolders

Place everything together in the SD card root:

```
SD root/
  MKSTFT28EVO.bin      ← from this folder
  config.ini           ← from this folder
  language_XX.ini      ← your choice from kisslorand/BTT-TFT-FW
  TFT28/               ← your choice from kisslorand/BTT-TFT-FW
    bmp/
    font/
```

See the full flashing instructions in the [upgrade guide](../README.md).
