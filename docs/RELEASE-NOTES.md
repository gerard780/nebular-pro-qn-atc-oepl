# Unified v2 — experimental prerelease

One firmware file automatically configures the three recognized product records
for supported Hanshow Nebular Pro-154Q-N and Pro-266Q-N revisions. Physical BLE
and OEPL four-color image delivery passed on the 154 and legacy-record 266 bench
tags. Includes both version-2 AP profiles dated 4 October 2026.

ATC's unmodified browser UART flasher passed a real reflash on the legacy-record
266 with CH340 at **921600 baud** and **1-second activation**, including a
padding-marker erase test, full application readback and physical image checks.
The test bitmap's rectangle layout was corrected and verified on the display.

Browser installation on the 154 or from factory firmware, and BLE/OTA firmware
updates remain unverified. Preserve a complete same-tag backup and the factory
product sector. **RAW1/RAW2 only; keep compression disabled.**

Download the unified `.bin`, the profile for your model (`72.json` for the 266,
`73.json` for the 154), or the complete package ZIP. `SHA256SUMS.txt` on the
GitHub release verifies the flat release downloads; `SHA256SUMS` inside the ZIP
verifies repository files using their packaged paths.

See [flashing instructions](https://github.com/gerard780/nebular-pro-qn-atc-oepl/blob/main/docs/FLASHING.md), [browser test](https://github.com/gerard780/nebular-pro-qn-atc-oepl/blob/main/docs/BROWSER-TEST.md),
[validation limits](https://github.com/gerard780/nebular-pro-qn-atc-oepl/blob/main/docs/VALIDATION.md) and [component licensing](https://github.com/gerard780/nebular-pro-qn-atc-oepl/blob/main/docs/CREDITS.md).
