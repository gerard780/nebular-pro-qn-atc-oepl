#!/usr/bin/env python3
"""Verify this package, optionally matching a private full-tag backup. No hardware I/O."""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import sys
import zlib


def require(condition, message):
    if not condition:
        raise ValueError(message)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--backup', type=Path, help='Your own complete 512 KiB tag backup')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    sums = root / 'SHA256SUMS'
    listed = set()
    for line in sums.read_text().splitlines():
        expected, relative = line.split('  ', 1)
        path = (root / relative).resolve()
        require(path.is_relative_to(root), 'Checksum path escapes package')
        require(path.is_file(), f'Missing file: {relative}')
        require(hashlib.sha256(path.read_bytes()).hexdigest() == expected,
                f'Hash mismatch: {relative}')
        listed.add(relative)
    manifest = json.loads((root / 'manifest.json').read_text())
    require({'manifest.json', manifest['firmware'], *manifest['profiles'],
             'tools/verify.py'}.issubset(listed), 'Incomplete checksum list')
    firmware = (root / manifest['firmware']).read_bytes()
    require(len(firmware) == manifest['bytes'], 'Firmware size mismatch')
    require(hashlib.sha256(firmware).hexdigest() == manifest['sha256'], 'Firmware SHA mismatch')
    require(firmware[8:12] == b'KNLT', 'Missing Telink signature')
    require(struct.unpack_from('<I', firmware, 24)[0] == len(firmware), 'Header length mismatch')
    require(struct.unpack_from('<I', firmware, len(firmware) - 4)[0]
            == (zlib.crc32(firmware[:-4]) ^ 0xffffffff), 'Firmware CRC mismatch')
    erase_end = (len(firmware) + 4095) // 4096 * 4096
    require(erase_end <= 0x20000, 'Application exceeds reserved boundary')
    require(erase_end == manifest['application_erase_end_exclusive'], 'Erase boundary mismatch')
    records = manifest['accepted_product_records']
    require(len(records) == 3, 'Expected three accepted product records')
    for record in records:
        raw = bytes.fromhex(record['record_hex'])
        require(len(raw) == 92 and raw in firmware, 'Product record missing from firmware')
    for hw, geometry in [('72', (296, 152)), ('73', (200, 200))]:
        profile = json.loads((root / 'tagtypes' / f'{hw}.json').read_text())
        require((profile['width'], profile['height']) == geometry, f'{hw}: wrong geometry')
        require(profile['version'] == 2 and profile['bpp'] == 2 and profile['rotatebuffer'] == 1,
                f'{hw}: wrong rendering settings')
        require(set(profile['colortable']) == {'white', 'black', 'red', 'yellow'},
                f'{hw}: wrong palette')
        require(not profile.get('zlib_compression') and not profile.get('g5_compression'),
                f'{hw}: compression must be disabled')
        require(profile['options'] == ['led'] and 14 not in profile['contentids'],
                f'{hw}: unexpected feature options')
    if args.backup:
        backup = args.backup.read_bytes()
        require(len(backup) == 524288, 'Backup must be a full 512 KiB flash image')
        matches = [r for r in records if bytes.fromhex(r['record_hex']) == backup[0x7e000:0x7e05c]]
        require(len(matches) == 1, 'Unrecognized product record; do not flash this build')
        print('Recognized backup model:', matches[0]['name'])
    print(f'PASS: {len(listed)} file hashes; firmware signature, length and CRC; both AP profiles')
    print(f'UART application erase range: 0x00000–0x{erase_end:05x} (end exclusive)')
    print('These checks do not establish physical browser flashing or BLE/OTA support.')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, OSError, struct.error) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        sys.exit(1)
