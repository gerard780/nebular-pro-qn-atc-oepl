# Credits and component licensing

## ATC base firmware

Aaron Christophel / **ATC1441** provides the ATC_BLE_OEPL base firmware and
[browser uploader / flasher](https://atc1441.github.io/ATC_BLE_OEPL_Image_Upload.html).
Upstream material is in
[atc1441/atc1441.github.io](https://github.com/atc1441/atc1441.github.io).

The pinned base used for this build is 116,756 bytes with SHA-256
`26dc327d40d7db62c3867da8a9ae0a5092728404ca9942eae402720b5d72337c`.
The unified patch adds exact-record detection, model-specific display handling
and captured factory waveforms. It is not an upstream ATC release.

Panel recovery and hardware testing: **Gerard780**, with AI-assisted
implementation and analysis. The firmware incorporates manufacturer-extracted
waveform material.

Existing project records leave public redistribution permissions for the
combined ATC/manufacturer firmware unresolved. No blanket firmware license or
third-party redistribution permission is granted by this draft. Document the
applicable permissions before publishing the binary publicly.

## AP profiles

The profiles in `tagtypes/` adapt the OpenEPaperLink contributors'
[tag profile/template material](https://github.com/OpenEPaperLink/OpenEPaperLink/tree/master/resources/tagtypes)
under **CC BY-NC-SA 4.0**. The license is included in
[licenses/CC-BY-NC-SA-4.0.txt](../licenses/CC-BY-NC-SA-4.0.txt).

Changes include model names, dimensions, four-color palette, buffer rotation,
disabled image compression, overlay templates, provisional IDs 72/73 and
feature options. These adapted profiles retain CC BY-NC-SA 4.0; this license
does not apply to the firmware binary or the entire repository.
