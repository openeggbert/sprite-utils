#!/usr/bin/env python3
"""
Generates sprite-sheet CSV rows for button00.blp (channel CHBUTTON) - a
plain uniform grid, cell 40x40, confirmed from CPixmap::CacheAll's
totalDim/iconDim for CHBUTTON (src/pixmap.cpp) and the same DrawIcon
grid-fallback formula used for text.blp
(rect.left=(rank%nbx)*iconDim.x, rect.top=(rank/nbx)*iconDim.y).

Unlike text.blp, there's no lookup table giving each grid cell a
confirmed meaning - src/button.cpp shows ranks 0-5 are generic button
*states* (normal/hover/pressed/.../locked, reused by every button) and
rank 6+ is "m_iconMenu[i] + 6" (an arbitrary per-caller menu-icon index,
not a fixed named set). So unlike every other sheet handled so far, this
script deliberately leaves Group="?" for every cell - the geometry is
solid (same grid mechanism already visually confirmed for text.blp), but
claiming a specific semantic name per cell here would be a guess, not a
finding.

button00.blp's canvas height differs by game (240x840 for Speedy Blupi I,
240x1040 for Speedy Blupi II - fewer menu icons in the earlier game), so
row count is a required argument.

Usage:
    python3 tools/generate_button00_rows.py <canvas_height> > button00_rows.csv
    python3 tools/generate_button00_rows.py 840   # Speedy Blupi I
    python3 tools/generate_button00_rows.py 1040  # Speedy Blupi II
"""

import sys

CELL_W, CELL_H = 40, 40
CANVAS_W = 240
COLS = CANVAS_W // CELL_W  # 6


def main():
    if len(sys.argv) != 2:
        print("usage: generate_button00_rows.py <canvas_height>", file=sys.stderr)
        sys.exit(1)
    canvas_h = int(sys.argv[1])
    rows_count = canvas_h // CELL_H
    total_cells = COLS * rows_count

    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    for rank in range(total_cells):
        x = (rank % COLS) * CELL_W
        y = (rank // COLS) * CELL_H
        note = "state 0-5 (normal/hover/pressed/.../locked)" if rank < 6 else f"menu-icon rank {rank - 6}"
        fields = [
            "button00.blp", "?", "1", str(rank + 1), "1",
            str(x), str(y), str(CELL_W), str(CELL_H),
            note, "auto:CHBUTTON_grid", str(rank + 1),
        ]
        out.append(";".join(fields))

    sys.stdout.write("\n".join(out) + "\n")
    print(f"# {total_cells} cells generated ({COLS} cols x {rows_count} rows), all Group=?", file=sys.stderr)


if __name__ == "__main__":
    main()
