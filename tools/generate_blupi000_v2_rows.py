#!/usr/bin/env python3
"""
Generates sprite-sheet CSV rows for Speedy Blupi II's blupi000.blp from data
that is *already* extracted, and committed as plain C source, inside the
`free-eggbert` decompilation repo:

  - include/pixtables.hpp: table_icon_blupi[] - 336 IconPack records
    (pos.x, pos.y, offset.x, offset.y, size.x, size.y), i.e. the exact
    pixel rectangle of every character sprite in blupi000.blp (and, since
    they share this one table, blupi001-003.blp too - not generated here).
  - src/dectables.cpp: table_blupi[] - a flat stream of
    [action_id, frame_count, phase, icon_0 .. icon_(frame_count-1)] records
    (see decblupi.cpp's consumer) mapping each ACTION_* to the icon indices
    it uses, in order.
  - include/def.hpp: the ACTION_* #defines (name + inline comment) that
    table_blupi's action_id values refer to.

See sprite-utils/analysis.md (SS4.1-4.3) for how this was originally found,
and plan.md's Phase 2 section for the merge decision (blupi000.blp's rows
in speedy_blupi_II.spritesheet.csv were 100% unlabeled "?" placeholders
with a geometry that didn't correspond to this table at all, so this
replaces them outright rather than merging).

Usage:
    python3 tools/generate_blupi000_v2_rows.py > blupi000_v2_rows.csv

Then splice the output over the existing blupi000.blp block in
spritesheets/speedy_blupi_II.spritesheet.csv (find its line range with
`grep -n '^blupi000.blp'`) and re-verify with `sprite_utils draw` before
trusting it - this script does not touch the repo CSV directly.
"""

import re
import sys
from pathlib import Path

FREE_EGGBERT = Path("/rv/data/development/github.com/openeggbert/free-eggbert")
PIXTABLES_HPP = FREE_EGGBERT / "include/pixtables.hpp"
DECTABLES_CPP = FREE_EGGBERT / "src/dectables.cpp"
DEF_HPP = FREE_EGGBERT / "include/def.hpp"

# fc >= this: flag the record as a likely combinatorial state lookup rather
# than a plain animation sequence (see plan.md - many of table_blupi's
# larger records are not literal walk-cycle-style frame lists; e.g.
# ACTION_STOP alone has 330 entries, mostly repeats of a handful of idle
# poses, not 330 distinct animation frames).
COMBINATORIAL_THRESHOLD = 40


def parse_table_icon_blupi():
    text = PIXTABLES_HPP.read_text()
    m = re.search(r"extern short table_icon_blupi\[\]\s*=\s*\{(.*?)\};", text, re.S)
    nums = [int(x) for x in re.findall(r"-?\d+", m.group(1))]
    count = nums[0]
    rest = nums[1:]
    assert len(rest) == count * 6, "table_icon_blupi size mismatch - source may have changed"
    icons = []
    for i in range(count):
        px, py, ox, oy, sx, sy = rest[i * 6 : i * 6 + 6]
        icons.append({"pos_x": px, "pos_y": py, "offset_x": ox, "offset_y": oy, "size_x": sx, "size_y": sy})
    return icons


def parse_table_blupi():
    text = DECTABLES_CPP.read_text()
    m = re.search(r"int table_blupi\[\]\s*=\s*\{(.*?)\};", text, re.S)
    nums = [int(x) for x in re.findall(r"-?\d+", m.group(1))]
    n = len(nums)
    action_ids = set(range(1, 88))  # ACTION_STOP=1 .. ACTION_PUTDYNAMITE=87

    i = 0
    records = []
    while i < n:
        action = nums[i]
        if action == 0:
            break  # clean terminator
        if action not in action_ids:
            # table_blupi has one known ~30-int block of -1 padding between
            # the ACTION_HIDE and ACTION_CLEAR2-ish records (verified: the
            # parse below reaches the array's true final 0 terminator with
            # nothing left over, which it would not if this were eating
            # real record data) - skip one int at a time and resync.
            i += 1
            continue
        frame_count = nums[i + 1]
        phase = nums[i + 2]
        if frame_count < 0 or frame_count > 500 or i + 3 + frame_count > n:
            i += 1
            continue
        icons = nums[i + 3 : i + 3 + frame_count]
        records.append({"action": action, "frame_count": frame_count, "phase": phase, "icons": icons})
        i += frame_count + 3

    assert i == n - 1 and nums[-1] == 0, (
        f"table_blupi did not parse cleanly to its terminator (stopped at {i} of {n}) "
        "- source may have changed, re-derive the parser before trusting the output"
    )
    return records


def parse_action_names():
    text = DEF_HPP.read_text()
    actions = {}
    for m in re.finditer(r"#define\s+(ACTION_\w+)\s+(\d+)(?:\s*//\s*(.*))?", text):
        name, val, comment = m.group(1), int(m.group(2)), (m.group(3) or "").strip()
        actions[val] = {"name": name, "comment": comment}
    return actions


def csv_field(s):
    # SpriteSheetRow's CSV parser splits on a literal ";" with no quoting or
    # escaping support - keep generated text free of both ";" and newlines
    # so nothing silently shifts into the wrong column.
    return str(s).replace(";", ",").replace("\n", " ").replace("\r", " ")


def main():
    icons = parse_table_icon_blupi()
    records = parse_table_blupi()
    action_names = parse_action_names()

    icon_uses = {i: [] for i in range(len(icons))}
    for r in records:
        for pos, icon in enumerate(r["icons"]):
            if icon < 0:
                continue
            icon_uses[icon].append((r["action"], pos, r["frame_count"]))

    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    for i, icon in enumerate(icons):
        uses = icon_uses[i]
        if uses:
            primary_action, primary_pos, frame_count = uses[0]
            info = action_names.get(primary_action, {})
            group = info.get("name", f"ACTION_{primary_action}")
            number_in_group = primary_pos + 1

            note_parts = []
            if info.get("comment"):
                note_parts.append(info["comment"])
            if frame_count >= COMBINATORIAL_THRESHOLD:
                note_parts.append(
                    f"part of a {frame_count}-entry table_blupi record - likely a state lookup, "
                    "not a plain animation sequence"
                )
            others = {}
            for a, _pos, _fc in uses[1:]:
                others[a] = others.get(a, 0) + 1
            if others:
                bits = [f"{action_names.get(a, {}).get('name', f'ACTION_{a}')}({cnt}x)" for a, cnt in others.items()]
                note_parts.append("also used by: " + ", ".join(bits))
            notes = " | ".join(note_parts)
            tags = "auto:table_blupi"
        else:
            group = "?"
            number_in_group = 1
            notes = ""
            tags = "auto:table_icon_blupi"

        fields = [
            "blupi000.blp", group, str(number_in_group), str(i + 1), "1",
            str(icon["pos_x"]), str(icon["pos_y"]), str(icon["size_x"]), str(icon["size_y"]),
            csv_field(notes), tags, str(i + 1),
        ]
        out.append(";".join(fields))

    sys.stdout.write("\n".join(out) + "\n")
    labeled = sum(1 for i in range(len(icons)) if icon_uses[i])
    print(f"# {labeled}/{len(icons)} icons labeled from table_blupi", file=sys.stderr)


if __name__ == "__main__":
    main()
