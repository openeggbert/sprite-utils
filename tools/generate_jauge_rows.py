#!/usr/bin/env python3
"""
Generates sprite-sheet CSV rows for jauge.blp (channel CHJAUGE) - identical
124x88 canvas in both games. Confirmed as a plain 1-column x 4-row uniform
grid (rank 0-3), cell 124x22, from CPixmap::CacheAll's totalDim/iconDim for
CHJAUGE (free-eggbert/src/pixmap.cpp:1090-1094, and DIMJAUGEX/DIMJAUGEY in
include/def.hpp:64-65) - the same grid mechanism already confirmed for
text.blp/button00.blp.

Unlike button00.blp, CJauge::Draw's own drawing logic (src/jauge.cpp) is
not usable as evidence - its decompilation is visibly broken (writes one
byte into a 12-byte buffer then reads it back as a 32-bit LOWORD/HIWORD
pair, uninitialized-memory territory), so no rectangle formula from it can
be trusted. Instead, the 4 rows were identified by what's actually in the
image (each is a full, edge-to-edge horizontal bar, a single flat color)
cross-referenced against every call site that touches CJauge::m_type - the
row index into this grid - across the whole decompiled codebase:

  - decor.cpp:81  m_jauges[JAUGE_AIR].Create(...,   type=1, ...)
  - decblupi.cpp:2051  m_jauges[JAUGE_AIR].SetType(1)
  - decblupi.cpp:2780  m_jauges[JAUGE_AIR].SetType(2)
  - decor.cpp:83  m_jauges[JAUGE_POWER].Create(..., type=3, ...)

(JAUGE_AIR=0, JAUGE_POWER=1 in def.hpp are gauge *slots*, unrelated to
m_type/the row index - CDecor keeps exactly 2 CJauge instances, each
independently selecting one of the 4 image rows via m_type.) That is an
exhaustive grep of every SetType/Create call in free-eggbert/src - no call
anywhere sets type=0, so row 0 (black) has no confirmed caller and is left
Group="?" rather than guessed. Rows 1/2 (red/cyan) are both driven by the
JAUGE_AIR slot (an oxygen gauge used while Blupi is underwater - decblupi.cpp
swim-related code around both SetType calls) and share one Group, giving a
2-frame "low air" cyan/red-style toggle; row 3 (yellow) is the JAUGE_POWER
slot's only type, used for the shield/power-up gauge.

Usage:
    python3 tools/generate_jauge_rows.py > jauge_rows.csv
"""

import sys

CELL_W, CELL_H = 124, 22
ROWS = 4

# rank (0-3) -> (group, number_in_group, note)
ROW_INFO = {
    0: ("?", 1, "black bar - no CJauge::Create/SetType call anywhere in the "
                "decompiled source sets m_type=0; meaning unconfirmed"),
    1: ("JAUGE_AIR", 1, "red bar - CJauge::Create's initial type for the "
                        "JAUGE_AIR slot (decor.cpp:81) - oxygen/air gauge"),
    2: ("JAUGE_AIR", 2, "cyan bar - JAUGE_AIR.SetType(2) (decblupi.cpp:2780) "
                        "- oxygen/air gauge, alternate state"),
    3: ("JAUGE_POWER", 1, "yellow bar - CJauge::Create's fixed type for the "
                          "JAUGE_POWER slot (decor.cpp:83), never changed "
                          "elsewhere - shield/power-up gauge"),
}


def csv_field(s):
    return str(s).replace(";", ",").replace("\n", " ").replace("\r", " ")


def main():
    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    for rank in range(ROWS):
        group, number_in_group, note = ROW_INFO[rank]
        y = rank * CELL_H
        fields = [
            "jauge.blp", group, str(number_in_group), str(rank + 1), "1",
            "0", str(y), str(CELL_W), str(CELL_H),
            csv_field(note), "auto:CHJAUGE_grid", str(rank + 1),
        ]
        out.append(";".join(fields))

    sys.stdout.write("\n".join(out) + "\n")
    print(f"# {ROWS} cells generated (1 col x {ROWS} rows), 3/4 rows labeled", file=sys.stderr)


if __name__ == "__main__":
    main()
