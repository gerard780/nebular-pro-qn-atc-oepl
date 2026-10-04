# Hanshow Nebular — unified ATC BLE / OpenEPaperLink firmware

One experimental firmware binary for the **1.54″ Nebular Pro-154Q-N** and
**2.66″ Nebular Pro-266Q-N** revisions listed below. It detects a supported
factory product record and selects the panel configuration automatically.
Both sizes have physically displayed white, black, red and yellow images sent
over Bluetooth and OpenEPaperLink (OEPL) radio using this exact unified v2 build.

This is an unofficial patch of Aaron Christophel / ATC1441's firmware.

## Download

- [Unified v2 firmware](firmware/EXPERIMENTAL_ATC_Nebular_154Q_266Q_Unified.bin)
  — **129,588 bytes**, the same file for both sizes.
- OEPL AP profiles: [2.66″ / 72.json](tagtypes/72.json) and
  [1.54″ / 73.json](tagtypes/73.json).
- [Flashing instructions](docs/FLASHING.md), [validation](docs/VALIDATION.md),
  [manifest](manifest.json) and [checksums](SHA256SUMS).

Download the actual `.bin` through GitHub's **Download raw file** action. A saved
GitHub HTML page is not firmware.

```text
Firmware SHA-256:
be9fedc864d74d96f31697a99b629d56814e83225cb04e7cd9c61ff8ec7e083c
```

## Supported hardware

| Model | Case revision | PCB | Native resolution | OEPL type (hex) |
| --- | --- | --- | --- | --- |
| Nebular Pro-154Q-N | C3HW2 | HSEL4Q_01_54M_39 | 200 × 200 | `0073` |
| Nebular Pro-266Q-N | E3LW2 | HSEL7_02_66M_30 | 152 × 296 | `0072` |

Model names and PCB markings alone do not establish compatibility. Firmware
compares all **92 bytes at flash address `0x7e000`** with three captured records:
the 154 record, the newer factory 266 record, and a legacy 266 record. The exact
accepted records are in [manifest.json](manifest.json). A different record
selects a disabled-display preset. Preserve the tag's factory sector; do not
copy another tag's record to bypass detection.

The 154 and legacy-record 266 have passed physical unified-v2 tests. The newer
factory-record 266 path passed software tests; its unified-v2 display behavior
has not been physically tested. Other sizes and revisions are unsupported.

## Can ATC_BLE_OEPL flash it?

**It should work with the wired UART Flasher in the
[ATC_BLE_OEPL uploader](https://atc1441.github.io/ATC_BLE_OEPL_Image_Upload.html).**
The file passes the inspected uploader's Telink signature and length checks,
and its **Write Firmware** path writes the application from address zero while
preserving upper factory flash. Browser-to-hardware flashing itself remains
**untested**. Hardware installation was verified with guarded wired programmers.
Read [the exact workflow and backup requirements](docs/FLASHING.md) first.

**BLE/OTA firmware installation remains unverified.** Successful Bluetooth image
uploads and passing the browser's firmware-file checks do not validate OTA.

## OpenEPaperLink AP setup

Install the matching JSON file on the **AP**, separately from tag firmware:

| Tag | AP file | AP image size |
| --- | --- | --- |
| 2.66″ | `/tagtypes/72.json` | 296 × 152, landscape |
| 1.54″ | `/tagtypes/73.json` | 200 × 200 |

Back up an existing profile before replacing it. **72 and 73 are provisional
experimental IDs**, so check for another profile using either ID.

From the repository directory, replace `http://oepl.local` with your AP address
and upload the file for your tag:

```sh
curl --fail --form 'data=@tagtypes/72.json;filename=/tagtypes/72.json;type=application/json' http://oepl.local/edit
curl --fail --form 'data=@tagtypes/73.json;filename=/tagtypes/73.json;type=application/json' http://oepl.local/edit
```

Read back the uploaded JSON, reload the AP page, and allow the tag to check in.
Both profiles keep image compression disabled. Use rotation 0 and a four-color
test image; confirm orientation, text and colors on the actual display after
transfer completes. The 266 profile handles the difference between landscape
AP images and the panel's native portrait geometry.

Direct Bluetooth image uploads need no AP profile. Use **Upload Image Raw** in
ATC's uploader, with second and third colors enabled for four-color images.

## Features and limits

- Automatic supported-record detection and panel setup.
- All 40 captured factory waveforms: two passes × ten temperature bands × two
  panel sizes. Software checks recover each original waveform byte exactly.
- RAW1 / RAW2 image delivery over BLE and OEPL; **no zlib/G5 image decoding**.
- RGB LED controls exposed by the profiles; physically verified on the 266.
- Physical temperature testing covers ambient conditions only. Embedded factory
  bands do not establish operating-temperature ratings.
- NFC wake is advertised, but phone-tap wake testing remains pending. OEPL
  **Set NFC URL is not implemented** by the pinned ATC base.
- No integrated ATC built-in clock/text renderer or custom-LUT support. AP-side
  rendered clock images use the ordinary image-transfer path.
- Neither tested tag advertises a wake button.

## Verify the download

```sh
python3 tools/verify.py
```

This checks package hashes, the firmware header/checksum, application bounds and
AP profile settings without accessing hardware. To also identify a tag from
your own complete 512 KiB flash backup:

```sh
python3 tools/verify.py --backup /path/to/your-own-tag-backup.bin
```

Keep that backup private. This repository contains the application binary and
AP profiles, with no full-chip snapshots or device calibration backups.

## Credits and licensing

Base firmware: **Aaron Christophel / ATC1441**. Panel recovery and hardware
testing: **Gerard780**, with AI-assisted implementation and analysis.
AP profiles derive from OpenEPaperLink contributor material.
See [credits and component licensing](docs/CREDITS.md). No repository-wide
license is assigned to the combined firmware binary.
