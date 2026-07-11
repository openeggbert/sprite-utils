#!/usr/bin/env python3
"""
Extracts the equivalent of free-eggbert's table_blupi (the Blupi
character's action/frame table) directly from Speedy Blupi I's BLUPI.EXE
(v1.0). There is no free-eggbert-equivalent decompilation for v1.0 to read
an offset out of, so - like the four IconPack tables in
extract_v1_icon_tables.py - this was *found* by a byte-signature scan (see
plan.md's Phase 2 v1.0 milestone for the search + validation method) and is
hardcoded here once confirmed.

Confirmation, beyond "the scan found a long, cleanly-terminated chain of
structurally valid records" (67 records, ending at a literal 0
terminator, every icon index within table_icon_blupi's valid 0-290
range): the action-id *order* this table walks in matches
free-eggbert's table_blupi almost exactly for every action id v1.0 and
v2.2 have in common (e.g. both start their "core" run with
2, 60, 3, 4, 5, 59, 61, 62, 6, 7, 8, 9, 10, 13, 11, ... in that exact
order), and record 0 (action_id=1, frame_count=330, phase=0) is
byte-for-byte the same shape as v2.2's ACTION_STOP record. That kind of
order match is not something a false-positive byte pattern produces by
chance - this is the real table, carried over largely unchanged from
v1.0 into v2.2.

Usage:
    python3 tools/extract_v1_table_blupi.py /path/to/BLUPI.EXE > v1_table_blupi.json
"""

import json
import struct
import sys

START_OFFSET = 0x2D5A0  # confirmed by the byte-signature scan, see docstring
MAX_ICON = 290  # table_icon_blupi has 291 entries (0..290) for v1.0
ACTION_MIN, ACTION_MAX = 1, 200
FC_MIN, FC_MAX = 1, 400


def extract(data):
    n = len(data)
    i = START_OFFSET
    records = []
    while True:
        action = struct.unpack_from("<i", data, i)[0]
        if action == 0:
            break
        if not (ACTION_MIN <= action <= ACTION_MAX):
            raise AssertionError(f"unexpected action id {action} at 0x{i:x} - offset may be stale")
        fc = struct.unpack_from("<i", data, i + 4)[0]
        if not (FC_MIN <= fc <= FC_MAX) or i + 12 + fc * 4 > n:
            raise AssertionError(f"unexpected frame_count {fc} at 0x{i:x} - offset may be stale")
        phase = struct.unpack_from("<i", data, i + 8)[0]
        icons = list(struct.unpack_from(f"<{fc}i", data, i + 12))
        if any(v < -1 or v > MAX_ICON for v in icons):
            raise AssertionError(f"icon index out of range in record at 0x{i:x} - offset may be stale")
        records.append({"action": action, "frame_count": fc, "phase": phase, "icons": icons})
        i += 12 + fc * 4
    return records


def main():
    if len(sys.argv) != 2:
        print("usage: extract_v1_table_blupi.py /path/to/BLUPI.EXE", file=sys.stderr)
        sys.exit(1)
    with open(sys.argv[1], "rb") as f:
        data = f.read()
    records = extract(data)
    print(f"# {len(records)} records, action ids: {sorted(set(r['action'] for r in records))}", file=sys.stderr)
    json.dump(records, sys.stdout, indent=1)
    print(file=sys.stdout)


if __name__ == "__main__":
    main()
