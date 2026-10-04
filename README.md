# Hanshow Nebular Pro-154Q-N / Pro-266Q-N — unified firmware

> **Need another Hanshow tag supported?** Please take a complete, verified
> readback of its **original factory firmware before flashing anything**, then
> send it to **[Gerard780](https://github.com/gerard780)** with the exact model,
> hardware/PCB revision and clear board/display photos. Two matching reads and
> the dump's SHA-256 help establish that it is usable. See the
> **[RP2040 programmer source, wiring diagram and readback guide](https://github.com/gerard780/hanshow-rp2040-programmer)**.
> [Open a support request](https://github.com/gerard780/nebular-pro-qn-atc-oepl/issues/new?title=Hanshow%20tag%20support%20request)
> to coordinate sending the files. The reader probes compatible legacy Telink
> hardware; other MCU families need an appropriate readback method. A dump is
> a starting point for investigation and does not guarantee support.

One experimental firmware binary for the **1.54″ Nebular Pro-154Q-N** and
**2.66″ Nebular Pro-266Q-N** revisions listed below. It detects a supported
factory product record and selects the panel configuration automatically.
Both sizes have physically displayed white, black, red and yellow images sent
over Bluetooth and OpenEPaperLink (OEPL) radio using this exact unified v2 build.

This is an unofficial patch of Aaron Christophel / ATC1441's firmware.

**For ATC1441 and firmware developers:** [display-driver source, all factory
waveforms and integration notes](atc-review/README.md) cover both sizes, including
the latest [1.54″ hardware and color findings](atc-review/154-FINDINGS.md).

## Research: orange and gray beyond the four-color image path

Separate bench diagnostics on **both tested panel sizes** displayed seven
visible palette states: white, black, red, yellow, **orange/coral, light gray
and dark gray**. The extra codes were already in the recovered factory palette;
the experiments used each panel's existing LUTs without changing waveform
timing or analog voltage settings.

![Extra-color diagnostics on the 2.66-inch and 1.54-inch panels](atc-review/images/both-panels-brighter.jpg)

The **unified v2 download still supports four-color image uploads**. OEPL already
has 3-bit and 4-bit image formats; this tag firmware needs receiver and decoder
support for them before arbitrary extra-color images can be uploaded. A further
16-code test on both sizes produced additional shades. A clearer 2.66 photograph shows
pink/salmon, mauve and khaki tones, but does not establish sixteen distinct
usable colors.

![Owner photograph of the 2.66-inch sixteen-code chart](atc-review/images/266-16-codes-cropped.png)

Read the [color findings, palette codes, photographs and validation limits](docs/LUT-COLORS.md).

## Download

- [Experimental unified v2 release](https://github.com/gerard780/nebular-pro-qn-atc-oepl/releases/tag/unified-v2)
  — firmware, both AP profiles, a complete package ZIP and release checksums.
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

**Yes: ATC's wired browser UART flasher was verified on the legacy-record 2.66″
tag.** The unmodified live
[ATC_BLE_OEPL uploader](https://atc1441.github.io/ATC_BLE_OEPL_Image_Upload.html)
passed using Chromium, CH340, **921600 baud**, 1-second activation and
**Write Firmware**. A marker in unused application padding proved a real flash
erase; full application readback matched, and settings/product/calibration were
unchanged. This was a reflash of an already installed unified v2 application.
Browser installation on the 154 or directly from factory firmware remains
untested. Read [the workflow and backup requirements](docs/FLASHING.md) and
[browser test details](docs/BROWSER-TEST.md) first.

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
