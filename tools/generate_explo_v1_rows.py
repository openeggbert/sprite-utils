#!/usr/bin/env python3
"""
Adds Group names to Speedy Blupi I's existing explo.blp rows - the last
remaining unlabeled file in either CSV.

Unlike every other v1.0 file done this session, explo.blp's geometry was
*already* correct (see plan.md's "Checked, not replaced" v1.0 milestone -
matched table_icon_explo, tools/data/v1_icon_tables.json, at 46/54 exact
and the rest within a few pixels), so this script does not touch X/Y/
Width/Height at all - it only adds Group/Number in Group to rows whose
icon index is confirmed by name.

Where the icon-name data comes from - a *partial* result, honestly:
v2.2's dectables.cpp/dectables.hpp declares 12 CHEXPLO tables
(table_explo1-8, table_sploutch1-3, table_tentacule). Byte-signature
scanning BLUPI.EXE v1.0 (same technique as tools/extract_v1_table_blupi.py)
found exactly 3 of them - table_explo2, table_explo3 and table_explo4 -
byte-for-byte identical to v2.2's values, laid out contiguously and in the
same declaration order (0x32590, 0x325e0, 0x32630). That triple match
(exact + contiguous + correctly ordered) is not chance. The other 9 tables
were NOT found with any usable confidence: an exact-byte search for each
found nothing, and a general "run of values in explo's valid icon range"
scan (the technique that found v1.0's table_blupi) was far too noisy here
- explo's icon range is only 54 values wide, so short/generic sequences
(ascending runs, all-zero padding) collide constantly. Rather than guess,
those 9 tables - and therefore Group names for icons 0-6, 16-31, 33, 36-38,
40-53 - are simply not attempted.

The 3 confirmed tables are hardcoded below (not parsed - there is no clean
v1.0 source to parse them from, this *is* the source, established by the
byte-signature scan). ROW_TO_ICON maps each of the 54 existing CSV rows
(in on-disk order) to its table_icon_explo index, established by matching
each row's (X,Y,Width,Height) against v1_icon_tables.json's table_icon_explo:
46/54 matched exactly; the remaining 8 were resolved by a min-cost
assignment (Hungarian algorithm) between the 8 unmatched rows and the 8
otherwise-unclaimed icons - largest total residual across all 4 dimensions
was 7px, versus every icon needing to be used exactly once (54 rows, 54
icons), which is why an assignment - not a threshold guess - is the right
tool here. This recovered icons 7, 14 and 15, which happen to be 3 of the
12 icons the 3 confirmed tables reference.

Usage:
    python3 tools/generate_explo_v1_rows.py > explo_v1_rows.csv

Splice the output over explo.blp's existing block in
spritesheets/speedy_blupi_I.spritesheet.csv (same line count and X/Y/W/H/
Notes/Tags values, only Group/Number in Group change) and re-verify with
sprite_utils draw before trusting it - this script does not touch the
repo CSV directly.
"""

import csv
import sys
from pathlib import Path

REPO = Path(__file__).parent.parent
SPRITESHEET = REPO / "spritesheets" / "speedy_blupi_I.spritesheet.csv"

# established by byte-signature scan of BLUPI.EXE v1.0 at 0x32590/0x325e0/
# 0x32630 - see module docstring
NAMED_TABLES_DECL_ORDER = ["table_explo2", "table_explo3", "table_explo4"]
NAMED_TABLES = {
    "table_explo2": [12, -1, 13, 14, -1, 15, 13, -1, 14, 15, 12, -1, 13, 15, 14, 14, -1, 14, 15, 13],
    "table_explo3": [32, 32, 34, 34, 32, 32, 34, 34, 32, 32, 34, 34, 32, 32, 35, 35, 32, 32, 35, 35],
    "table_explo4": [12, 13, 14, 15, 7, 8, 9, 10, 11],
}

# CSV row (0-indexed, on-disk order) -> table_icon_explo index. 46 established
# by an exact (X,Y,Width,Height) match; 8 (flagged "near") by a min-cost
# assignment against the 8 otherwise-unclaimed icons - see module docstring.
ROW_TO_ICON = {
    0: 0, 1: 24, 2: 25, 3: 23, 4: 39, 5: 11, 6: 40, 7: 1, 8: 10, 9: 35,
    10: 9, 11: 8, 12: 53, 13: 27, 14: 6, 15: 50, 16: 44, 17: 2, 18: 33,
    19: 7, 20: 29, 21: 26, 22: 34, 23: 5, 24: 4, 25: 36, 26: 17, 27: 48,
    28: 32, 29: 3, 30: 49, 31: 13, 32: 18, 33: 12, 34: 45, 35: 38, 36: 43,
    37: 14, 38: 22, 39: 28, 40: 51, 41: 52, 42: 20, 43: 15, 44: 19, 45: 47,
    46: 16, 47: 21, 48: 30, 49: 46, 50: 31, 51: 37, 52: 42, 53: 41,
}
NEAR_MATCH_ROWS = {17, 18, 19, 20, 37, 38, 43, 44}


def csv_field(s):
    return str(s).replace(";", ",").replace("\n", " ").replace("\r", " ")


def main():
    icon_uses = {i: [] for i in range(54)}
    for name in NAMED_TABLES_DECL_ORDER:
        for pos, val in enumerate(NAMED_TABLES[name]):
            if val >= 0:
                icon_uses[val].append((name, pos))

    text = SPRITESHEET.read_text()
    reader = csv.DictReader(text.splitlines(), delimiter=";")
    explo_rows = [r for r in reader if r["File"] == "explo.blp"]
    assert len(explo_rows) == 54, f"expected 54 explo.blp rows, found {len(explo_rows)}"

    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    labeled = 0
    for n, row in enumerate(explo_rows):
        icon_idx = ROW_TO_ICON[n]
        uses = icon_uses[icon_idx]
        note_parts = [row["Notes"]] if row["Notes"] else []
        if n in NEAR_MATCH_ROWS:
            note_parts.append("icon index resolved by nearest-geometry match, not exact")
        if uses:
            primary_name, primary_pos = uses[0]
            group = primary_name
            number_in_group = primary_pos + 1
            others = {}
            for name, _pos in uses[1:]:
                others[name] = others.get(name, 0) + 1
            self_reuse = others.pop(primary_name, 0)
            if self_reuse:
                note_parts.append(f"reused {self_reuse} more time(s) within {primary_name} itself")
            if others:
                note_parts.append("also used by: " + ", ".join(f"{n}({c}x)" for n, c in others.items()))
            labeled += 1
        else:
            group = "?"
            number_in_group = 1

        fields = [
            "explo.blp", group, str(number_in_group), row["Row"], row["Column"],
            row["X"], row["Y"], row["Width"], row["Height"],
            csv_field(" | ".join(note_parts)), row["Tags"], row["Number per file"],
        ]
        out.append(";".join(fields))

    sys.stdout.write("\n".join(out) + "\n")
    print(f"# explo.blp (v1.0): {labeled}/54 icons labeled (from 3/12 named tables found)", file=sys.stderr)


if __name__ == "__main__":
    main()
