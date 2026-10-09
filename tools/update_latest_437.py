#!/usr/bin/env python3
"""Refresh the stable 437 ZIP and metadata from the highest numbered package."""
import hashlib
import json
import re
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / 'firmware'
STEM = 'Nebular_437Q_N_Test_Firmware'


def main():
    candidates = []
    for path in FIRMWARE.glob(STEM + '_v*.zip'):
        match = re.fullmatch(STEM + r'_v([1-9][0-9]*)\.zip', path.name)
        if match:
            candidates.append((int(match[1]), path))
    if not candidates:
        raise ValueError('No numbered 437 test firmware package found')
    version, source = max(candidates)
    with zipfile.ZipFile(source) as archive:
        for line in archive.read('SHA256SUMS').decode().splitlines():
            expected, name = line.split('  ', 1)
            actual = hashlib.sha256(archive.read(name)).hexdigest()
            if actual != expected:
                raise ValueError('Package checksum mismatch: ' + name)
        manifest = json.loads(archive.read('manifest.json'))
        firmware = archive.read(manifest['firmware'])
        if (len(firmware) != manifest['bytes'] or
                hashlib.sha256(firmware).hexdigest() != manifest['sha256']):
            raise ValueError('Firmware does not match its manifest')
        if manifest['firmware'] != f'EXPERIMENTAL_ATC_Nebular_437Q_N_v{version}.bin':
            raise ValueError('Numbered package and firmware version differ')
    data = source.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    latest = FIRMWARE / (STEM + '_latest.zip')
    latest.write_bytes(data)
    (FIRMWARE / (latest.name + '.sha256')).write_text(
        digest + '  ' + latest.name + '\n')
    metadata = dict(version=version, package=source.name, bytes=len(data),
                    sha256=digest, firmware=manifest['firmware'],
                    firmware_sha256=manifest['sha256'], status=manifest['status'])
    (FIRMWARE / (STEM + '_latest.json')).write_text(
        json.dumps(metadata, indent=2) + '\n')
    print(f'Latest 437 package: v{version}, {source.name}, SHA256 {digest}')


if __name__ == '__main__':
    main()
