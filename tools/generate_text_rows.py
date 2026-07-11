#!/usr/bin/env python3
"""
Generates sprite-sheet CSV rows for text.blp (used by both Speedy Blupi I
and II - same 256x384 canvas, same font table). Unlike every other sheet
handled so far, text.blp is NOT addressed through a pixtables.hpp-style
rectangle table at all - it's a plain uniform grid (16x16 cells, 16
columns x 24 rows = 384 cells), confirmed directly from
CPixmap::CacheAll's totalDim/iconDim for channel CHTEXT
(src/pixmap.cpp) and DrawIcon's generic grid-fallback formula
(rect.left=(rank%nbx)*iconDim.x, rect.top=(rank/nbx)*iconDim.y).

Character -> grid cell mapping comes from table_char[] in
include/texttables.hpp (256 entries x 6 shorts: charIcon, offX, offY,
accentIcon, accentOffX, accentOffY - see CDecor::DrawChar in
src/text.cpp). Rank = charIcon + font*128, for font in
{FONTWHITE=0, FONTGOLD=1, FONTSELECTED=2} (include/text.hpp) - 3*128=384,
exactly the grid size. Cross-checked against the printable ASCII range
(32-126): charIcon == byte value exactly for every one of those 95 bytes,
a 1:1 identity mapping that gives high confidence in the formula overall.

Usage:
    python3 tools/generate_text_rows.py > text_rows.csv
"""

import re
import sys
from pathlib import Path

TEXTTABLES_HPP = Path("/rv/data/development/github.com/openeggbert/free-eggbert/include/texttables.hpp")

CELL_W, CELL_H = 16, 16
COLS = 16  # 256 / 16
FONTS = ["FONTWHITE", "FONTGOLD", "FONTSELECTED"]

# human-readable names for control/whitespace/non-printable byte values so
# Notes doesn't end up with a literal control character in it
NAMED_BYTES = {32: "space", 9: "tab", 10: "newline", 13: "cr", 127: "del"}


def describe_byte(b):
    if b in NAMED_BYTES:
        return NAMED_BYTES[b]
    if 33 <= b <= 126:
        return chr(b)
    return f"byte_0x{b:02x}"


def parse_table_char():
    text = TEXTTABLES_HPP.read_text()
    m = re.search(r"extern char table_char\[\]\s*=\s*\{(.*?)\};", text, re.S)
    nums = [int(x) for x in re.findall(r"-?\d+", m.group(1))]
    assert len(nums) == 256 * 6, f"table_char size mismatch: {len(nums)}"
    entries = []
    for i in range(256):
        charIcon, offX, offY, accentIcon, accentOffX, accentOffY = nums[i * 6 : i * 6 + 6]
        entries.append({"charIcon": charIcon, "accentIcon": accentIcon})
    return entries


def csv_field(s):
    return str(s).replace(";", ",").replace("\n", " ").replace("\r", " ")


def main():
    entries = parse_table_char()

    # charIcon (0-127) -> list of byte values using it as their primary glyph
    primary_of = {}
    for byte, e in enumerate(entries):
        primary_of.setdefault(e["charIcon"], []).append(byte)
    # charIcon (0-127) -> list of byte values using it as an accent overlay
    accent_of = {}
    for byte, e in enumerate(entries):
        if e["accentIcon"] != -1:
            accent_of.setdefault(e["accentIcon"], []).append(byte)

    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    labeled = 0
    for rank in range(16 * 24):  # 384 grid cells
        font_idx, char_icon = divmod(rank, 128)
        font_name = FONTS[font_idx]

        primaries = sorted(primary_of.get(char_icon, []))
        accents = sorted(accent_of.get(char_icon, []))

        def summarize(bytes_):
            shown = [f"{describe_byte(b)}({b})" for b in bytes_[:5]]
            if len(bytes_) > 5:
                shown.append(f"+{len(bytes_) - 5} more")
            return ", ".join(shown)

        note_parts = [f"font={font_name}"]
        if primaries:
            group = f"char_{primaries[0]}"
            desc = describe_byte(primaries[0])
            note_parts.append(f"glyph for '{desc}' (byte {primaries[0]})")
            if len(primaries) > 1:
                note_parts.append(f"also the primary glyph for: {summarize(primaries[1:])}")
            labeled += 1
        elif accents:
            group = f"accent_{char_icon}"
            note_parts.append(f"accent overlay glyph used by: {summarize(accents)}")
            labeled += 1
        else:
            group = "?"

        if accents and primaries:
            note_parts.append(f"also used as an accent overlay by: {summarize(accents)}")

        row = rank + 1
        col = 1
        x = (rank % COLS) * CELL_W
        y = (rank // COLS) * CELL_H
        # same Group across all 3 fonts of one character, NumberInGroup =
        # font index (1/2/3) - a font-style "animation" is a harmless and
        # arguably fun side effect of reusing the group mechanism this way
        number_in_group = font_idx + 1

        fields = [
            "text.blp", group, str(number_in_group), str(row), str(col),
            str(x), str(y), str(CELL_W), str(CELL_H),
            csv_field(" | ".join(note_parts)), "auto:table_char", str(rank + 1),
        ]
        out.append(";".join(fields))

    sys.stdout.write("\n".join(out) + "\n")
    print(f"# {labeled}/384 cells labeled", file=sys.stderr)


if __name__ == "__main__":
    main()
