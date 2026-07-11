#!/usr/bin/env python3
"""
Generates sprite-sheet CSV rows for Speedy Blupi I's blupi000.blp from
tools/data/v1_icon_tables.json (table_icon_blupi equivalent) and
tools/data/v1_table_blupi.json (table_blupi equivalent) - both extracted
directly from BLUPI.EXE v1.0 by tools/extract_v1_icon_tables.py and
tools/extract_v1_table_blupi.py (see those scripts and plan.md's Phase 2
v1.0 milestone for how they were found and validated).

Group names use the same ACTION_* constants from free-eggbert's
include/def.hpp (v2.2) applied to v1.0's action ids. This is a deliberate
inference, not a direct read of v1.0 source (none exists) - justified by
the near-identical action-id *ordering* and identical ACTION_STOP record
shape between the two tables (see plan.md). Only action ids that fall in
def.hpp's known 1-87 ACTION_* range get a name; anything outside it (v1.0
has one record with action id 118, well outside that range) is left
unlabeled rather than guessed.

HUMAN_OVERRIDES below preserves real, pre-existing hand labeling: the
*previous* speedy_blupi_I.spritesheet.csv already had 37 manually
measured and verified (Tags="ok") blupi000.blp rows with human-friendly
group names (Yellow_Eggbert_Swimming_Right, Yellow_Eggbert_Born, Yellow
Bomb, ...). Matching each one's resolved (X,Y,Width,Height) against
table_icon_blupi's 291 rectangles found an *exact* pixel-perfect match
(0px center distance) for all 37 - the two-years-ago manual measurement
was fully correct, just walked in a different (visual left-to-right, not
internal-table) order, and worth keeping over the generic ACTION_* name
where it exists. The matched ACTION_* name is preserved in Notes rather
than discarded.

Usage:
    python3 tools/generate_blupi000_v1_rows.py > blupi000_v1_rows.csv
"""

import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).parent
DEF_HPP = Path("/rv/data/development/github.com/openeggbert/free-eggbert/include/def.hpp")

COMBINATORIAL_THRESHOLD = 40

# icon index (0-based, into table_icon_blupi) -> (human group, number in group)
HUMAN_OVERRIDES = {
    278: ("YELLOW_EGGBERT_BORN", 1), 283: ("Yellow_Eggbert_Born", 2), 279: ("Yellow_Eggbert_Born", 3),
    282: ("Yellow_Eggbert_Born", 4), 284: ("Yellow_Eggbert_Born", 5), 277: ("Yellow_Eggbert_Born", 6),
    280: ("Yellow_Eggbert_Born", 7), 276: ("Yellow_Eggbert_Born", 8), 281: ("Yellow_Eggbert_Born", 9),
    260: ("Yellow Bomb", 1), 259: ("Yellow Bomb", 2), 261: ("Yellow Bomb", 3), 258: ("Yellow Bomb", 4),
    257: ("Yellow Bomb", 5), 262: ("Yellow Bomb", 6),
    48: ("Yellow_Eggbert_Life", 1),
    89: ("Yellow_Eggbert_Swimming_Right", 1), 39: ("Yellow_Eggbert_Swimming_Right", 2),
    76: ("Yellow_Eggbert_Swimming_Right", 3), 77: ("Yellow_Eggbert_Swimming_Right", 4),
    81: ("Yellow_Eggbert_Swimming_Right", 5), 79: ("Yellow_Eggbert_Swimming_Right", 6),
    80: ("Yellow_Eggbert_Swimming_Right", 7), 88: ("Yellow_Eggbert_Swimming_Right", 8),
    78: ("Yellow_Eggbert_Swimming_Right", 9),
    92: ("Yellow_Eggbert_Swimming_Left", 1), 40: ("Yellow_Eggbert_Swimming_Left", 2),
    82: ("Yellow_Eggbert_Swimming_Left", 3), 87: ("Yellow_Eggbert_Swimming_Left", 4),
    83: ("Yellow_Eggbert_Swimming_Left", 5), 86: ("Yellow_Eggbert_Swimming_Left", 6),
    85: ("Yellow_Eggbert_Swimming_Left", 7), 91: ("Yellow_Eggbert_Swimming_Left", 8),
    84: ("Yellow_Eggbert_Swimming_Left", 9),
    90: ("Yellow_Eggbert_Swimming_Butt", 1),
    35: ("Yellow_Eggbert_Crouching_Left", 1), 38: ("Yellow_Eggbert_Crouching_Right", 1),
}


def parse_action_names():
    text = DEF_HPP.read_text()
    actions = {}
    for m in re.finditer(r"#define\s+(ACTION_\w+)\s+(\d+)(?:\s*//\s*(.*))?", text):
        name, val, comment = m.group(1), int(m.group(2)), (m.group(3) or "").strip()
        actions[val] = {"name": name, "comment": comment}
    return actions


def csv_field(s):
    return str(s).replace(";", ",").replace("\n", " ").replace("\r", " ")


def main():
    icons = json.loads((HERE / "data/v1_icon_tables.json").read_text())["table_icon_blupi"]
    records = json.loads((HERE / "data/v1_table_blupi.json").read_text())
    action_names = parse_action_names()

    icon_uses = {i: [] for i in range(len(icons))}
    for r in records:
        for pos, icon in enumerate(r["icons"]):
            if icon < 0:
                continue
            icon_uses[icon].append((r["action"], pos, r["frame_count"]))

    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    unnamed_actions = 0
    human_applied = 0
    for i, icon in enumerate(icons):
        uses = icon_uses[i]
        # only keep uses whose action id resolves to a known v2.2 ACTION_* name
        named_uses = [(a, p, fc) for a, p, fc in uses if a in action_names]
        if not named_uses and uses:
            unnamed_actions += 1

        note_parts = []
        tags = "auto:table_icon_blupi_v1"
        if named_uses:
            primary_action, primary_pos, fc = named_uses[0]
            info = action_names[primary_action]
            group = info["name"]
            number_in_group = primary_pos + 1
            note_parts.append("names borrowed from Speedy Blupi II's def.hpp - see tools/generate_blupi000_v1_rows.py")
            if info["comment"]:
                note_parts.append(info["comment"])
            if fc >= COMBINATORIAL_THRESHOLD:
                note_parts.append(
                    f"part of a {fc}-entry table - likely a state lookup, not a plain animation sequence"
                )
            others = {}
            for a, _p, _fc in named_uses[1:]:
                oname = action_names[a]["name"]
                others[oname] = others.get(oname, 0) + 1
            self_reuse = others.pop(group, 0)
            if self_reuse:
                note_parts.append(f"reused {self_reuse} more time(s) within {group} itself")
            if others:
                note_parts.append("also used by: " + ", ".join(f"{n}({c}x)" for n, c in others.items()))
            tags = "auto:table_blupi_v1"
        else:
            group = "?"
            number_in_group = 1

        if i in HUMAN_OVERRIDES:
            human_applied += 1
            if named_uses:
                note_parts.insert(0, f"originally auto-labeled {group}")
            group, number_in_group = HUMAN_OVERRIDES[i]
            note_parts.append("hand-measured and verified in the original speedy_blupi_I.spritesheet.csv")
            tags = "ok"

        notes = " | ".join(note_parts)
        fields = [
            "blupi000.blp", group, str(number_in_group), str(i + 1), "1",
            str(icon["pos_x"]), str(icon["pos_y"]), str(icon["size_x"]), str(icon["size_y"]),
            csv_field(notes), tags, str(i + 1),
        ]
        out.append(";".join(fields))

    sys.stdout.write("\n".join(out) + "\n")
    labeled = sum(
        1 for i in range(len(icons))
        if i in HUMAN_OVERRIDES or (icon_uses[i] and any(a in action_names for a, _p, _fc in icon_uses[i]))
    )
    print(
        f"# {labeled}/{len(icons)} icons labeled ({unnamed_actions} had a use but no known ACTION_* name, "
        f"{human_applied} use a preserved hand-verified label)",
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
