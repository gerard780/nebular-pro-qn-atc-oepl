# Flashing unified v2

## Status

Guarded wired installation, application readback, automatic panel configuration,
and subsequent BLE/OEPL image delivery were verified on a 154 and a legacy-record
266. The following ATC browser UART workflow is supported by source inspection
and file-format checks, but has not been run against hardware with this binary.
BLE/OTA firmware installation remains unverified.

## Before writing

1. Confirm the exact supported model and PCB in the [README](../README.md).
2. Capture a complete **512 KiB backup of your own tag**, using a suitable wired
   readback tool. Retain it and your recovery programmer. The browser workflow
   below does not create or verify that backup.
3. Run `python3 tools/verify.py --backup /path/to/your-backup.bin` from the
   repository root to check the package and match the preserved product record.
   A record match is a compatibility check, not proof of a reliable backup;
   independently verify your readback before programming.
4. Use the correct SWS/programming pads for that PCB and the programmer's
   validated wiring. The ATC UART tool uses SWS through a serial adapter, rather
   than the tag's ordinary UART RX/TX connection. Avoid powering the tag from
   both its battery and a programmer unless that setup is validated.

Do not erase the entire chip. The firmware needs the original product record at
`0x7e000`, and a full-chip backup also preserves settings and calibration data.

## Intended ATC browser UART workflow

1. Open the [ATC_BLE_OEPL uploader](https://atc1441.github.io/ATC_BLE_OEPL_Image_Upload.html)
   in a browser supporting Web Serial. Switch to **Advanced View** if needed,
   then select **UART Flasher**.
2. Connect the correctly wired programmer and open its COM port. Use the baud
   rate and activation timing validated for your adapter and tag.
3. Choose the local file
   `EXPERIMENTAL_ATC_Nebular_154Q_266Q_Unified.bin` in the **UART Flasher** section.
   The expected size is **129,588 bytes**. The **Select Firmware** control in the
   BLE section starts the separate, unverified OTA path; do not use it here.
4. Click **Write Firmware**. Avoid **Write Firmware & Type**, **Write Display
   Type**, and **Erase All Flash**. The server's **Load ATC_BLE_OEPL.bin** action
   loads the upstream firmware, rather than this patched local file.
5. After completion and reset, read screen/configuration information over BLE.
   A recognized tag should automatically report type `0073` / 200 × 200 for the
   154, or `0072` / native 152 × 296 for the 266. No preliminary ATC installation
   or manually selected display preset is part of this intended workflow.
6. Verify application readback with a suitable wired tool, then send an
   asymmetric test image with text and all four colors. The browser's completion
   message alone is not a readback check. Allow time for both refresh passes and
   inspect the settled display.
7. Install the matching [AP profile](../README.md#openepaperlink-ap-setup) if
   using OEPL radio image delivery.

If detection or display setup fails, keep the factory sector intact and use
your own same-tag backup for recovery. Do not force a supported record or apply
another tag's full flash dump.

## Why the file should work with the UART writer

The upstream uploader source inspected on 4 October 2026 checks `KNLT` at offset
`0x08` for UART file selection. The binary contains that signature, its length
field at `0x18` equals 129,588, and its final Telink-style CRC matches.

The inspected `uartFlashWrite` starts at address zero, erases 4 KiB sectors as
needed, and writes 256-byte blocks. For this file it erases 32 sectors:
`0x00000` through `0x1f000`, ending at **`0x20000` exclusive**. It does not reach
settings at `0x79000`, the product sector at `0x7e000`, or calibration at
`0x7f000`. ATC's own first boot may initialize or update its settings sector.

This establishes format and write-range compatibility with the inspected
source. It does not establish electrical/programmer reliability, hardware
browser installation, or OTA bootloader behavior. The source URL and inspected
file hash are recorded in [flasher compatibility metadata](flasher-compatibility.json).

## Recovery

Restore only a complete backup captured from the same physical tag using a
wired recovery tool appropriate for that hardware. Keep stable power throughout
installation and recovery. No factory backup is bundled in this repository.
