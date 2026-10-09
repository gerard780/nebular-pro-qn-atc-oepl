# Experimental 437Q-N test firmware

[Download the latest 437Q-N test package](https://github.com/gerard780/nebular-pro-qn-atc-oepl/raw/refs/heads/main/firmware/Nebular_437Q_N_Test_Firmware_latest.zip)

That URL stays the same as new numbered packages are uploaded. The package
includes the application binary, source, AP profile, test image, checksums,
installation instructions and recovery planning tool. Check its README and
manifest for the current validation status before testing.

[Current version and hash](Nebular_437Q_N_Test_Firmware_latest.json) ·
[ZIP checksum](Nebular_437Q_N_Test_Firmware_latest.zip.sha256)

The 437Q-N package is separate from the unified 154Q/266Q firmware. Older
numbered packages remain available for reproduction and comparison.

## Maintaining the download

Upload packages as `Nebular_437Q_N_Test_Firmware_vN.zip` in this directory on
`main`. The `Update latest 437 test firmware` workflow checks the packaged
checksums and firmware manifest, then updates the stable ZIP and metadata to
the highest numeric version. For example, v10 sorts after v9. An invalid newest
package fails validation and does not replace the published latest download.

The workflow also supports manual runs in GitHub Actions. If another workflow
uploads a numbered package using `GITHUB_TOKEN`, dispatch this workflow
explicitly because those pushes do not trigger further push workflows.

To refresh the files locally:

```sh
python3 tools/update_latest_437.py
```
