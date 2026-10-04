# Browser UART test — 4 October 2026

The unmodified live [ATC_BLE_OEPL uploader](https://atc1441.github.io/ATC_BLE_OEPL_Image_Upload.html)
successfully reflashed unified v2 on the connected **Nebular Pro-266Q-N with the
recognized legacy product record**.

| Item | Tested setup/result |
| --- | --- |
| Browser | Chromium 153.0.8010.52 on the bench Pi, with a virtual desktop |
| Serial transport | Native Web Serial through the existing CH340 |
| UART baud | 921600 |
| Activation | 1 second |
| Selected action | UART Flasher → local unified binary → Write Firmware |
| Browser write time | Approximately 33 seconds |
| Erased application sectors | 32, from `0x00000` through `0x1f000` |
| Readback | All 131,072 application bytes match the binary plus erased padding |
| Retained data | Settings, factory product record and calibration match pre-test copies |
| Flash protection | Same `0x382c` before and after |
| Automatic configuration | Type `0072`, native 152 × 296 geometry and expected panel pins |
| Post-flash display | Fresh raw BLE image; text, orientation, four colors and rectangle outlines verified through the webcam |

Firmware SHA-256:
`be9fedc864d74d96f31697a99b629d56814e83225cb04e7cd9c61ff8ec7e083c`.

## Proof that the browser reached hardware

The same unified binary was already installed, so a matching readback alone
could not distinguish a successful reflash from a no-op. A guarded wired tool
programmed a 16-byte marker at `0x1fff0`, outside the firmware image but inside
the final application sector. The marker was read back and verified before
the browser write. After browser completion it was erased to `ff`, and the
subsequent full application readback matched every expected byte. This confirms
an actual hardware flash erase and a correct resulting application.

The initial browser attempt at 460800 baud reported success and read back the
unchanged application/settings/factory data. Because it preceded the marker
test, it does not independently prove that a flash write occurred. Use the
marker-proven **921600 baud** setup for this tested CH340/tag combination.

## Scope

This was a reflash on a previously modified tag already running unified v2.
It does not validate first installation from stock firmware, browser flashing
on the 154, other hardware records or programmers, or BLE/OTA installation.
No full-chip erase or browser display-type write was used. The browser source
and its serial implementation were not modified for the test.

Full backups and device-specific readback files are retained privately and are
excluded from the public draft. The source fingerprint and compact hardware
results are in [flasher-compatibility.json](flasher-compatibility.json).

## Display check

The first test bitmap had incorrect layout coordinates: its rectangle outlines
extended below the color swatches and overlapped the footer at 296 × 152.
The test-image generator was corrected to place each outline inside its swatch,
and the corrected image was uploaded and physically verified. No firmware
change was needed for this layout correction.

The [corrected 296 × 152 test bitmap](browser-test-pattern.png) can be used for
the 266 image-delivery check. Send it uncompressed and inspect the settled
display for white, black, red and yellow, readable text, and complete outlines.
