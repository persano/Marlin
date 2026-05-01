# Bootloader Update Files — TFT28 GD32F305

## Files in this folder

| File / Folder | Required | Purpose |
|---------------|----------|---------|
| `MKSTFT28EVO.bin` | Yes | Bootloader v3.0.5 binary |
| `mks_config.txt` | Yes | Tells the old bootloader what update to apply |
| `mks_font/` | **Yes — must be present even if empty** | Required by the old bootloader to trigger the update process |
| `mks_pic/` | **Yes — must be present even if empty** | Required by the old bootloader to trigger the update process |

## Important — copy the empty folders too

The old MKS/Artillery bootloader checks for the presence of the `mks_font` and `mks_pic` folders on the SD card as part of its update detection logic. **If these folders are missing, the bootloader will not start the update.**

When you copy the contents of this folder to your SD card, make sure you copy the two empty folders as well:

```
SD root/
  MKSTFT28EVO.bin      ← bootloader binary
  mks_config.txt       ← update config
  mks_font/            ← must exist, can be empty
  mks_pic/             ← must exist, can be empty
```

> **Note:** the `.gitkeep` files inside `mks_font/` and `mks_pic/` are only here so Git tracks the folders. You do **not** need to copy `.gitkeep` to your SD card — only the folders themselves need to be present.
