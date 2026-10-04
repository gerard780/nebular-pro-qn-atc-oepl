# Latest Pro-154Q-N findings

Captured specimen: C3HW2, PCB `HSEL4Q_01_54M_39`, native 200 × 200.
Updated 4 October 2026.

## What changes from the older 266-only driver

The 154 shares the recovered two-component panel protocol and pin map with
the captured 266. It requires its own geometry, initialization settings and
temperature-specific waveform records. Reusing the 266 LUT after only changing
dimensions is not supported by the captured data.

| Setting | Pro-266Q-N | Pro-154Q-N |
| --- | --- | --- |
| Native dimensions | 152 × 296 | 200 × 200 |
| `0x4D`, `0xAE`, `0xB6`, `0xBA` preamble | `78`, `0F`, `0F`, `2A` | Omitted |
| Command `0x06` booster bytes | `0F 0A 2F 25 22 2E 21` | `0F 0A 2F 25 22 2B 21` |
| Command `0xF0` | `5F` | `7D` |
| Source-plane bytes | 5,624 | 5,000 |
| Packed bytes per refresh component | 11,248 | 10,000 |
| Waveform source | Twenty 266 records | Twenty 154 records |

Both panels have ten captured temperature bands with identical boundaries and
the same seven-entry factory palette. Their waveform/analog records differ
within the same temperature band. [waveform-comparison.json](waveform-comparison.json)
records the byte differences and hashes for all twenty matched pairs;
[waveforms.json](waveforms.json) identifies the included fixture bytes.
Factory code similarity is evidence for a shared protocol, not an established
commercial controller part number or permission to exchange the waveforms.

## Unified-v2 checks on the 154

The corrected unified v2 application was installed with the RP2040 programmer,
changed sectors were verified, and full application readback matched.
Automatic configuration selected provisional type `0073`, 200 × 200 geometry
and the expected pins. Fresh BLE and OEPL images physically displayed white,
black, red and yellow with correct orientation. The same application file also
passed physical display tests on the legacy-record 266.

The host fixture covers both geometries, all ten bands, both raw formats and
independent waveform bytes. Physical display testing covers ambient conditions;
capturing low/high-temperature waveform records is not physical testing in
those environments. Browser UART installation on the 154 and BLE/OTA firmware
installation remain unverified.

## Seven factory palette states

Separate diagnostic applications exposed the existing factory palette without
changing waveform timings or analog voltage settings. On the 154, the settled
chart at MCU/EPD 29°C used band 7 (`25 < MCU T ≤ 30°C`). The owner confirmed
orange/coral by eye; the chart also showed both gray states.

| State | Factory code | Upper component | Lower component |
| --- | --- | --- | --- |
| White | `5` | `1` | `1` |
| Black | `4` | `1` | `0` |
| Red | `13` | `3` | `1` |
| Yellow | `9` | `2` | `1` |
| Orange/coral | `1` | `0` | `1` |
| Light gray | `6` | `1` | `2` |
| Dark gray | `7` | `1` | `3` |

![Corrected seven-state diagnostic on the 154](images/154-seven-colors.jpg)

The photograph was taken about 48 seconds after transfer completed. The white
reference is repeated at bottom right. An earlier chart incorrectly treated
native coordinates as displayed coordinates; the uploader rotation combined
with native row reversal transposed them. That first image was excluded from
the conclusions. The corrected chart aligns the labels and swatches.

The legacy-record 266 separately displayed the seven states using its own
band-6 waveforms at MCU 24°C / EPD 26°C. Its diagnostic installation recovered
from a lost SWS connection and completed without bulk readback at the owner's
request. Boot, image transfer and the settled photograph passed; a final full
application hash was not measured for that diagnostic.

![266 seven-state chart beside the 154 sixteen-code chart](images/both-panels-brighter.jpg)

Left: 266 seven-state chart. Right: 154 sixteen-code chart. The brighter image
uses increased camera exposure/gain with unchanged contrast/saturation and no
digital color correction. These are visual observations under uncalibrated
lighting, not measured gamut or guaranteed matches to factory RGB labels.

## Sixteen combinations on both panels

The 154 also received all sixteen four-bit combinations in two chart transfers
at a measured MCU temperature of 28°C:

```text
 5   4  13   9
 1   6   7  15
 0   2   3   8
10  11  12  14
```

The legacy-record 266 subsequently displayed the same matrix in two transfers
using its own unmodified band-6 waveform pair at MCU temperature 24°C. The
first transfer completed at 20:57:29 UTC (EPD 25°C); the repeat completed at
20:59:22 UTC (EPD 24°C). Settled webcam photographs were taken about 49 seconds
later, and both charts looked broadly consistent.

![Cropped owner photograph of the repeated 266 sixteen-code chart](images/266-16-codes-cropped.png)

The owner supplied this clearer photograph of the repeated chart, whose display
timestamp is 20:59:18 UTC. Orange is strong at code 1; codes 2 and 14 appear
pink/salmon, code 15 mauve, code 10 khaki, and code 11 another neutral gray.
Some dark combinations and warm shades still look similar. These are visual
descriptions, not calibrated color measurements. Camera settings and actual
photograph time were not supplied.

This is a rectangular crop of the [original owner photograph](images/266-16-codes-original.jpg).
No resizing, perspective correction or color adjustment was applied to the
published crop. [Photo provenance](images/266-16-codes-provenance.json) records
the original hash and crop rectangle.

The 266 sixteen-code diagnostic programmed all seven changed application
sectors, committed sector zero last, restored flash protection and booted.
Bulk readback was omitted at the owner's request. Successful BLE configuration,
both image transfers and physical photographs establish the chart result;
they do not establish a measured final application hash for that diagnostic.

Both panels produced extra visible tones. This does **not** establish sixteen
distinct, repeatable usable colors. These out-of-palette codes are not enabled
by the released `panel.c`; the diagnostic widened its accepted codes to 0–15.
The supported factory codes in the released core remain `1, 4, 5, 6, 7, 9, 13`.

The published unified v2 adapter maps incoming RAW1/RAW2 data to only four
codes (`5, 4, 13, 9`). Diagnostic firmware recolors fixed chart cells; it is
not an arbitrary seven-color image decoder. General extra-color support needs
an agreed image encoding, AP/uploader changes, decoding and repeatability,
ghosting/transition and temperature testing. Adding three palette entries to
an AP profile alone cannot encode the extra states.

These results support native integration of the two recovered panel variants
and further palette research. They do not change the released binary or expand
its input formats, hardware scope or physical temperature coverage.
