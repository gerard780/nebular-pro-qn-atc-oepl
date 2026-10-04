# Validation of the included binary

Firmware SHA-256:
`be9fedc864d74d96f31697a99b629d56814e83225cb04e7cd9c61ff8ec7e083c`.

## Existing bench evidence, 3–4 October 2026

| Check | Result |
| --- | --- |
| Host display tests | Two models × ten temperature bands × RAW1/RAW2; 80 paired refresh cases |
| Factory waveforms | All 40 reconstructed waveforms byte-exact |
| Additional host checks | Geometry, asymmetric pixel ordering, boot identity preservation, invalid input, unknown records, timer wrap and BUSY timeout |
| Rebuild | Reproduced the included firmware byte for byte |
| 154 installation | Guarded RP2040 wired update; changed-sector checks and matching full application readback |
| Legacy-record 266 installation | Guarded CH340 wired update; all 32 application sectors checked, 20 changed; retained settings/product/calibration verified |
| Automatic configuration | Type `0073` and 200 × 200 on 154; type `0072` and native 152 × 296 on 266 |
| BLE image delivery | Physical white/black/red/yellow display on both bench tags |
| OEPL image delivery | Transfers completed; physical images, orientation and all four colors verified on both bench tags |
| 266 RGB LED | Red, green, blue and off verified over BLE; all three colors also verified over OEPL |

The version-2 AP profiles dated 4 October expose LED controls, remove the
unsupported button option and omit unsupported NFC URL content. The firmware
binary did not change for this profile update.

## Package and flasher checks

`tools/verify.py` checks the binary SHA-256, `KNLT` signature, header length,
Telink-style CRC and application boundary. It checks both profiles' geometry,
palette, rotation, compression settings and current feature options.
`SHA256SUMS` covers every distributed file except itself.

Compatibility metadata records checks using functions extracted from the
upstream browser flasher: file selection, BLE format acceptance, and the UART
write loop run with simulated flash operations. Simulation validates the file
and address range; it does not exercise a physical programmer.

The subsequent [browser hardware test](BROWSER-TEST.md) passed on the
legacy-record 266: native Web Serial, CH340, Chromium 153.0.8010.52,
921600 baud, 1-second activation and the unmodified live ATC uploader. A marker
outside the firmware image but inside its final erased sector proved that
hardware flash was erased. The entire 128 KiB application matched afterward;
settings, factory product data, calibration and flash protection were preserved.
Automatic type `0072`, geometry and pins were verified after restarting.
A fresh RAW image also passed physical inspection for text, orientation,
four colors and rectangle outlines after correcting the test bitmap's layout.

## Still unverified

- Browser UART installation on the 154, newer-record 266 or from factory firmware.
- BLE/OTA firmware installation and bootloader behavior.
- Unified-v2 physical rendering on the newer factory-record 266; only the
  legacy-record 266 and captured 154 were physically tested with unified v2.
- Extreme-temperature operation and other hardware revisions.
- Physical NFC phone-tap wake. NFC URL writing is absent from the pinned base.
- Recovery through the browser workflow.

This package summarizes the existing bench evidence. Private flash dumps,
device identities, AP addresses and raw bench logs are excluded.
