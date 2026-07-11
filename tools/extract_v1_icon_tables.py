#!/usr/bin/env python3
"""
Extracts the four IconPack geometry tables (equivalents of free-eggbert's
table_icon_blupi/object/element/explo) directly from Speedy Blupi I's
BLUPI.EXE (v1.0), by reading fixed byte offsets - same technique as
free-eggbert's own util/extract_pixtables.py, which does this for Speedy
Blupi II's BLUPI.exe (v2.2).

There is no free-eggbert-equivalent decompilation for v1.0 to read these
offsets out of - they were *found* directly in the binary, in an earlier
session, via a general byte-signature scan (looking for a run of records
that all decode to plausible values: position inside the matching .blp's
own canvas size, width/height in a few-pixel-to-~100-pixel range, a small
draw offset - see analysis.md SS4.6 for the full method and reasoning) and
then confirmed by rendering every resulting rectangle onto the real .blp
sheet and checking it lands on a real sprite. All four offsets below have
been visually verified this way; see plan.md's Phase 2 v1.0 milestone for
the confirmation.

Usage:
    python3 tools/extract_v1_icon_tables.py /path/to/BLUPI.EXE > v1_icon_tables.json

BLUPI.EXE itself (535,552 bytes, from Speedy_Blupi_I.7z on the library
drive) is not committed to this repo - see analysis.md SS8 on not
redistributing original game assets. The four small integer tables this
script extracts *are* committed (tools/data/v1_icon_tables.json) - they're
plain geometry data, the same kind of thing free-eggbert itself commits as
pixtables.hpp.
"""

import json
import struct
import sys

# (name, file offset of the record count, matching .blp canvas size used to
# find/verify it)
TABLES = [
    ("table_icon_blupi", 0x775D8, (784, 770)),
    ("table_icon_object", 0x78380, (896, 987)),
    ("table_icon_element", 0x78F10, (608, 591)),
    ("table_icon_explo", 0x797D8, (784, 868)),
]


def extract(data, name, count_offset, canvas):
    count = struct.unpack_from("<H", data, count_offset)[0]
    icons = []
    p = count_offset + 2
    max_w, max_h = canvas
    for i in range(count):
        px, py, ox, oy, sx, sy = struct.unpack_from("<6H", data, p)
        if px > max_w or py > max_h:
            raise AssertionError(
                f"{name}[{i}] pos ({px},{py}) exceeds its canvas {canvas} - "
                "offset may no longer be correct for this exact BLUPI.EXE build"
            )
        icons.append({"pos_x": px, "pos_y": py, "offset_x": ox, "offset_y": oy, "size_x": sx, "size_y": sy})
        p += 12
    return icons


def main():
    if len(sys.argv) != 2:
        print("usage: extract_v1_icon_tables.py /path/to/BLUPI.EXE", file=sys.stderr)
        sys.exit(1)

    with open(sys.argv[1], "rb") as f:
        data = f.read()

    out = {}
    for name, offset, canvas in TABLES:
        icons = extract(data, name, offset, canvas)
        out[name] = icons
        print(f"# {name}: {len(icons)} icons", file=sys.stderr)

    json.dump(out, sys.stdout, indent=1)
    print(file=sys.stdout)


if __name__ == "__main__":
    main()
