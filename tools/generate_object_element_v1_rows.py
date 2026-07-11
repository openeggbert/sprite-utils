#!/usr/bin/env python3
"""
Adds Group names to Speedy Blupi I's existing object.blp/element.blp rows -
the last open item in either CSV (see plan.md's Phase 2 status).

Same approach as tools/generate_explo_v1_rows.py, at larger scale: v2.2's
dectables.cpp/dectables.hpp declares ~130 named tables; CHANNEL_MAP in
tools/generate_object_element_explo_v2_rows.py maps 88 of them to
CHOBJECT/CHELEMENT/CHEXPLO by call-site tracing. Byte-signature scanning
BLUPI.EXE v1.0 for each of the 76 CHOBJECT/CHELEMENT ones found 41 with
real confidence - see tools/data/v1_object_element_confirmed_tables.json
for the exact byte offset of each (also mostly forming several large,
gapless, correctly-ordered runs in the binary - e.g. a single 31-table
unbroken chain from table_bulldozer_left through table_glu - which is the
strongest evidence a byte-signature match can have). The other 35 were not
found: most reference icon indices well beyond v1.0's smaller
table_icon_object (246, vs v2.2's 441) / table_icon_element (187, vs
v2.2's 289) tables, meaning they simply can't exist unchanged in v1.0 -
not a search failure, a real absence (the sequel added creatures/items
that don't exist yet in the original game).

Two false positives were caught and excluded before finalizing this list:
table_marine's "exact match" turned out to be a not-4-byte-aligned
coincidence sitting inside the PE header itself (hundreds of overlapping
"matches" in a periodic data region - obviously not real table data), and
table_explo6's match (already excluded from generate_explo_v1_rows.py) is
a short, generic ascending run with no contiguous-cluster support. Every
one of the 41 confirmed tables here is either individually unique in the
whole 535KB file, or resolved by exact contiguous adjacency to an already-
confirmed neighbor (in source declaration order) - not "first occurrence
found", which is not the same thing and was a real bug caught mid-
investigation (it silently misrouted the 5 CHEXPLO tables' values into
element.blp's count on a first pass, inflating it from a true 111/187 to
an incorrect 126/187).

Geometry is untouched, exactly as in generate_explo_v1_rows.py - only
Group/Number in Group is added to the existing (already fully verified -
99%/91% exact geometry match, see plan.md) rows. The row->icon mapping
(tools/data/v1_object_row_to_icon.json, v1_element_row_to_icon.json) was
built the same way: exact (X,Y,Width,Height) match first (244/246,
171/188), then a min-cost assignment (Hungarian algorithm) for the
remainder against the otherwise-unclaimed icons. element.blp has 188 CSV
rows for only 187 real icons - one row is left permanently unassigned
(row index 38, 0-indexed) since there is nothing for it to match; every
other file/table in this session reconciled exactly, so this is called
out explicitly rather than silently dropped or force-matched.

Usage:
    python3 tools/generate_object_element_v1_rows.py

Writes object_v1_rows.csv and element_v1_rows.csv. Splice each over its
file's existing block in spritesheets/speedy_blupi_I.spritesheet.csv (same
line count and X/Y/W/H/Notes/Tags values, only Group/Number in Group
change) and re-verify with sprite_utils draw before trusting it - this
script does not touch the repo CSV directly.
"""

import csv
import json
import sys
from pathlib import Path

HERE = Path(__file__).parent
REPO = HERE.parent
SPRITESHEET = REPO / "spritesheets" / "speedy_blupi_I.spritesheet.csv"

sys.path.insert(0, str(HERE))
import generate_object_element_explo_v2_rows as v2  # noqa: E402

# Rows with no exact (X,Y,Width,Height) match against table_icon_object/
# element - resolved instead by nearest-geometry (min-cost) assignment.
# See module docstring.
OBJECT_NEAR_MATCH_ROWS = {102, 103}
ELEMENT_NEAR_MATCH_ROWS = {37, 51, 52, 64, 69, 70, 88, 89, 90, 91, 104, 105, 122, 123, 141, 142}
ELEMENT_UNASSIGNED_ROWS = {38}  # genuinely no matching icon (188 rows, 187 icons)


def csv_field(s):
    return str(s).replace(";", ",").replace("\n", " ").replace("\r", " ")


def generate(file_name, channel, n_icons, row_to_icon, near_rows, unassigned_rows,
             named_tables, decl_order, confirmed_names):
    icon_uses = {i: [] for i in range(n_icons)}
    for name in decl_order:
        if name not in confirmed_names or v2.CHANNEL_MAP.get(name) != channel:
            continue
        for pos, val in enumerate(named_tables[name]):
            if val < 0 or val >= n_icons:
                continue
            icon_uses[val].append((name, pos))

    text = SPRITESHEET.read_text()
    reader = csv.DictReader(text.splitlines(), delimiter=";")
    rows = [r for r in reader if r["File"] == file_name]

    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    labeled = 0
    for n, row in enumerate(rows):
        note_parts = [row["Notes"]] if row["Notes"] else []
        if n in unassigned_rows:
            note_parts.append("no matching table_icon entry found - geometry has no counterpart in the extracted icon table")
            group, number_in_group = "?", 1
            out.append(";".join([
                file_name, group, str(number_in_group), row["Row"], row["Column"],
                row["X"], row["Y"], row["Width"], row["Height"],
                csv_field(" | ".join(note_parts)), row["Tags"], row["Number per file"],
            ]))
            continue

        icon_idx = row_to_icon[str(n)]
        if n in near_rows:
            note_parts.append("icon index resolved by nearest-geometry match, not exact")
        uses = icon_uses[icon_idx]
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
            group, number_in_group = "?", 1

        fields = [
            file_name, group, str(number_in_group), row["Row"], row["Column"],
            row["X"], row["Y"], row["Width"], row["Height"],
            csv_field(" | ".join(note_parts)), row["Tags"], row["Number per file"],
        ]
        out.append(";".join(fields))

    print(f"# {file_name} (v1.0): {labeled}/{n_icons} icons labeled", file=sys.stderr)
    return out


def main():
    named_tables, decl_order = v2.parse_named_tables()
    confirmed_names = set(
        json.loads((HERE / "data/v1_object_element_confirmed_tables.json").read_text()).keys()
    )

    icon_tables = json.loads((HERE / "data/v1_icon_tables.json").read_text())
    obj_row_to_icon = json.loads((HERE / "data/v1_object_row_to_icon.json").read_text())
    elem_row_to_icon = json.loads((HERE / "data/v1_element_row_to_icon.json").read_text())

    jobs = [
        ("object.blp", v2.OBJECT, len(icon_tables["table_icon_object"]), obj_row_to_icon,
         OBJECT_NEAR_MATCH_ROWS, set()),
        ("element.blp", v2.ELEMENT, len(icon_tables["table_icon_element"]), elem_row_to_icon,
         ELEMENT_NEAR_MATCH_ROWS, ELEMENT_UNASSIGNED_ROWS),
    ]
    for file_name, channel, n_icons, row_to_icon, near_rows, unassigned_rows in jobs:
        rows = generate(file_name, channel, n_icons, row_to_icon, near_rows, unassigned_rows,
                         named_tables, decl_order, confirmed_names)
        out_path = Path(f"{file_name.replace('.blp', '')}_v1_rows.csv")
        out_path.write_text("\n".join(rows) + "\n")
        print(f"wrote {out_path} ({len(rows) - 1} rows)", file=sys.stderr)


if __name__ == "__main__":
    main()
