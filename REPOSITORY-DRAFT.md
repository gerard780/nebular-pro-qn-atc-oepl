# GitHub repository draft

Suggested repository: **`gerard780/hanshow-nebular-unified`**

Description: Experimental unified ATC BLE / OpenEPaperLink firmware for supported
Hanshow Nebular Pro-154Q-N and Pro-266Q-N tags.

Intended visibility: **public**. This directory is a local draft; no remote
repository, public upload or release has been created.

Suggested topics: `hanshow`, `nebular`, `epaper`, `openepaperlink`, `telink`,
`tlsr8359`, `bluetooth`, `firmware`.

Suggested first release: **`unified-v2`**, titled **Unified v2 — 1.54″ and 2.66″
Nebular Pro (experimental)**, marked as a **prerelease**.

Release body:

> One firmware file automatically configures the three recognized product
> records for supported Nebular Pro-154Q-N and Pro-266Q-N revisions. Physical
> BLE and OEPL four-color image delivery passed on the 154 and legacy-record
> 266 bench tags. Includes both version-2 AP profiles. ATC browser UART file and
> write-range checks passed; physical browser installation and BLE/OTA firmware
> updates remain unverified. Preserve a complete same-tag backup and factory
> product sector. RAW1/RAW2 only; keep compression disabled.

Suggested release assets: the unified `.bin`, `72.json`, `73.json`, and the
complete repository ZIP containing instructions and checksums.

The existing credits record unresolved redistribution permissions for the ATC
base/manufacturer waveform material; retain that status until resolved. The AP
profile license has its own scope. Review [CREDITS.md](docs/CREDITS.md) before
public publication. No whole-chip captures or per-device calibration backups
are included.
