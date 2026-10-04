# ATC1441 developer handoff — unified Nebular Pro Q-N driver

Developer reference for native firmware integration, updated 4 October 2026.

This directory contains the **display-driver and binary-adapter source behind
the released unified v2 application** for supported **Nebular Pro-154Q-N** and
**Pro-266Q-N** records. It updates the earlier 266-only V5 handoff with the
200 × 200 driver path, exact product-record detection, both panels' temperature
waveforms and the latest physical color observations, including the subsequent
sixteen-code chart on the 266.

![Latest cropped owner photo of the 266 sixteen-code diagnostic](images/266-16-codes-cropped.png)

*Separate diagnostic, photographed by the owner. All sixteen input combinations
were displayed, but sixteen distinct repeatable colors are not established.
The released firmware still supplies four colors. See the [findings and photo
provenance](154-FINDINGS.md#sixteen-combinations-on-both-panels).*

The intended upstream change is a distinct panel variant with two supported
configurations. Similar names or PCB markings do not establish compatibility.
The controller's exact commercial identity remains unconfirmed. The recovered
command sequence and tested product records are the basis for this driver.

## Start here

| File | Purpose |
| --- | --- |
| [panel.c](panel.c), [panel.h](panel.h) | Portable two-component panel driver, model-specific initialization, packing and BUSY state machine. |
| [adapter.c](adapter.c) | Released four-color RAW1/RAW2 decoder, boot detection, native row mapping and ATC completion integration. Absolute addresses are specific to the pinned compiled base. |
| [waves.inc](waves.inc) | All 40 embedded waveform records: two panels × ten temperature bands × two components; compressed with a 128-byte history window. |
| [products.inc](products.inc) | Three accepted 92-byte product records and their mapping to the two driver configurations. |
| [test_adapter.c](test_adapter.c), [host_io.h](host_io.h) | Host fixture for image conversion, both geometries, boot handling, bounds, timers and refresh completion. |
| [waveforms.json](waveforms.json), [fixtures/](fixtures/) | Hashes and independent extracted 546-byte factory records; the driver streams the first 535 bytes. |
| [154 findings](154-FINDINGS.md) | Hardware differences, waveform comparison and seven-state/16-code diagnostic results. |
| [PROVENANCE.json](PROVENANCE.json) | Source hashes and the exact released binary/base identities. |

The driver, adapter and generated includes are copied byte-for-byte from the
unified v2 source. The host test only changes its printed count to **40 paired
refreshes / 80 component refreshes**; the original test called these “80 paired
refreshes.” Its checks are unchanged. This handoff supplies the display source,
not the source of the complete ATC BLE/OEPL application or a complete firmware
rebuild package. No full-chip backups or per-device calibration files are included.

## Supported configurations

| Property | Pro-266Q-N | Pro-154Q-N |
| --- | --- | --- |
| Captured case / PCB | E3LW2 / `HSEL7_02_66M_30` | C3HW2 / `HSEL4Q_01_54M_39` |
| Driver model argument | `0` | `1` |
| Native geometry | 152 × 296 | 200 × 200 |
| Source one-bit plane | 5,624 bytes | 5,000 bytes |
| Packed bytes per component | 11,248 | 10,000 |
| OEPL AP image geometry | 296 × 152 landscape | 200 × 200 |
| Provisional hardware type (hex) | `0072` | `0073` |
| Patched base preset | `17` | `38` |

The three product records represent the captured factory 266, captured 154
and legacy 266 bench tag. `detect_model()` compares all 92 bytes at `0x7e000`.
Both 266 records select model 0; the 154 selects model 1. Unknown records select
the disabled-display preset. Preserve the factory sector and never copy a
different tag's record to force recognition. These are provisional type/preset
assignments in our patched base, not upstream allocations.

Both configurations use RESET PD4, DC PB7, BUSY PA1 (high means ready), CS PD2,
CLK PD7, MOSI PB6 and active-low ENABLE PB5.

## Integrating with native ATC source

1. Add a separate supported panel variant and retain the model-specific
   initialization from `panel.c`. The 154 omits the 266 preamble, uses `0x2B`
   instead of `0x2E` in the sixth booster byte and uses `0x7D` instead of `0x5F`
   for command `0xF0`. Its geometry and waveform bytes differ too.
2. Supply every `nebular_io` callback using your GPIO, SPI, timer and image
   functions. Zero-initialize `nebular_panel`, call
   `nebular_start(panel, io, context, model)`, and poll `nebular_poll()` through
   both components until DONE or ERROR. Keep the image and waveforms available
   for the entire operation and report ERROR as failure.
3. Select the panel's own waveform pair from signed MCU temperature at image
   start. Factory band upper bounds are `3, 6, 9, 12, 15, 20, 25, 30, 35, 127°C`
   and are inclusive. The adapter rejects `-128` as a sentinel. The EPD reading
   is not the temperature source used by this adapter. Captured band boundaries
   do not establish the panel's operating-temperature rating.
4. Provide `wave(context, component, index)` for the 535-byte record. The core
   reads analog bytes 0–6 during setup, then requests indices 0–534 in order
   for command `0x20`. Our decoder retains the first seven bytes separately
   and reconstructs the remaining 528 using the embedded history-window data.
   A source integration can use ordinary uncompressed arrays instead.
5. Port RAW conversion and orientation into your own image structures.
   `source_byte()` reverses native rows while preserving pixels within each
   row for the captured ATC upload pipeline. Avoid applying that transform
   twice. Confirm with asymmetric text/checkerboards, especially on the square
   154 where geometry alone cannot reveal a rotated image.
6. Allocate ordinary driver state and use native interfaces. The adapter's
   hard-coded function addresses, RAM addresses, 21-byte image header and
   reuse of ATC's 300-byte custom-LUT buffer belong to this binary patch.
   They are reference details, not a drop-in source integration.
7. Keep refresh scheduling active across both complete components. The adapter
   resets ATC's outer timer at the component transition; native code needs its
   own suitable deadline and error propagation. Allocate upstream hardware
   types separately from our provisional AP profiles.

BUSY waits are polled. Reset delays and complete image streams are synchronous;
adapt them to the scheduler/watchdog requirements of native firmware.

## Image and refresh representation

RAW1 (`0x20`) supplies one MSB-first bit plane; RAW2 (`0x21`) supplies two
consecutive planes. The released decoder maps
`plane0_bit | (plane1_bit << 1)` to white `5`, black `4`, red `13`, yellow `9`.
Other compressed input formats are rejected.

Each four-bit factory code is split across two complete refresh components:

```c
component_0 = (factory_code >> 2) & 3;
component_1 = factory_code & 3;
```

Four component pixels are packed per byte, starting in bits 7–6. Each component
has its own power/reset, initialization, waveform load, image stream, refresh
and power-down. The first component alone is not a finished image. The driver
uses command `0x20` for the waveform and `0x10` for pixels. Exact sequences and
timeouts are in `panel.c`.

The panel layer accepts seven factory codes, including orange/coral `1`, light
gray `6` and dark gray `7`. **The released image decoder still supplies only
four colors.** Separate diagnostics visibly demonstrated the additional states
on both bench panels. Arbitrary seven-color uploads require an extended image
encoding and matching AP/uploader/firmware support. See [154 findings](154-FINDINGS.md).

## Run the host checks

From the repository root, with a C99 host compiler installed:

```sh
cc -std=c99 -Wall -Wextra -Werror -O2 -I atc-review \
  atc-review/test_adapter.c atc-review/panel.c -o /tmp/nebular-review-test
/tmp/nebular-review-test atc-review/fixtures/266 atc-review/fixtures/154
python3 tools/verify.py
```

Do not compile the fixture with `-DNDEBUG`: its checks use C assertions.
The fixture runs 40 paired refreshes (80 components), checking all 40 embedded
waveforms against independently extracted records and both RAW formats. It
also checks initialization/geometry, every pixel of asymmetric images, boot
configuration and identity-field preservation, input limits, unknown records,
timer wrap and a BUSY timeout. These are host checks, not physical temperature
or OTA validation.

## Physical evidence and remaining limits

Unified v2 passed wired installation/readback, BLE four-color display and OEPL
four-color display on the captured 154 and legacy-record 266. The newer
factory-record 266 has software coverage but no physical unified-v2 test.
ATC's unmodified browser UART flasher was verified for a reflash on the legacy
266 at 921600 baud; it was not tested on the 154 or directly from factory
firmware. See [binary validation](../docs/VALIDATION.md) and
[browser test](../docs/BROWSER-TEST.md).

The latest 154 diagnostics used its own band-7 pair at 29°C and displayed seven
factory palette states. Its 16-code chart at 28°C produced additional tones.
The 266 subsequently displayed all sixteen combinations in two transfers at
MCU 24°C using its own band-6 pair. The owner photo shows orange, pink/salmon,
mauve, khaki and gray shades, without establishing sixteen distinct repeatable
colors. The separate 266 diagnostics omitted bulk application readback.
Neither diagnostic changes the released binary's validation or adds general
seven-color image support.

Extreme-temperature operation, other revisions, BLE/OTA installation,
integrated text/clock rendering and custom-LUT integration remain unvalidated
or unsupported by this adapter. NFC URL writing is absent from the pinned base.

## Provenance and licensing

Reference unified firmware SHA-256:
`be9fedc864d74d96f31697a99b629d56814e83225cb04e7cd9c61ff8ec7e083c`
(129,588 bytes). Absolute adapter addresses refer only to the 116,756-byte
ATC base with SHA-256
`26dc327d40d7db62c3867da8a9ae0a5092728404ca9942eae402720b5d72337c`.

Base firmware: Aaron Christophel / ATC1441. Panel recovery and testing:
Gerard780, with AI-assisted implementation and analysis. Manufacturer-extracted
waveforms retain the unresolved redistribution status described in
[component licensing](../docs/CREDITS.md). No blanket license is assigned to
this handoff; the AP profile license does not apply to these source/data files.
This is an unofficial integration proposal, with no upstream endorsement implied.
