#!/usr/bin/env python3
"""
Generates sprite-sheet CSV rows for Speedy Blupi II's object.blp,
element.blp and explo.blp from data already extracted, and committed as
plain C source, inside the `free-eggbert` decompilation repo:

  - include/pixtables.hpp: table_icon_object[] (441), table_icon_element[]
    (289), table_icon_explo[] (100) - the exact pixel rectangle of every
    icon in each of the three sheets.
  - src/dectables.cpp: ~130 further named tables (table_bulldozer_left,
    table_explo1, table_shield, table_decor_lave, ...), each a flat list of
    icon indices for one entity/animation.

Unlike blupi000.blp's table_blupi (one table, one consumer, explicit
action IDs), these ~130 tables have no built-in channel information - the
same table name could in principle feed any of the three icon geometry
tables (or none - several turned out not to be icon tables at all). The
CHANNEL_MAP below was derived by a dedicated read-only investigation
tracing every table to its actual call site (DrawIcon/QuickIcon channel
argument, or an explicit `.channel = CHxxx` assignment) across decor.cpp,
decblock.cpp, decmove.cpp, decdesign.cpp and decblupi.cpp - see plan.md's
Phase 2 "Milestone: object/element/explo.blp done" writeup for the full
per-table evidence trail. Tables that call-site tracing showed are *not*
icon-rank tables at all (table_decor_action = camera-scroll deltas,
table_vitesse_* = movement speed, table_blitz = FX timing, etc.) are
deliberately excluded, not guessed at.

Usage:
    python3 tools/generate_object_element_explo_v2_rows.py

Writes three CSVs (object_v2_rows.csv, element_v2_rows.csv,
explo_v2_rows.csv) next to this script's cwd. Splice each over its file's
existing block in spritesheets/speedy_blupi_II.spritesheet.csv (find the
line range with `grep -n '^object.blp'` etc.) and re-verify with
`sprite_utils draw` before trusting it - this script does not touch the
repo CSV directly.
"""

import re
import sys
from pathlib import Path

FREE_EGGBERT = Path("/rv/data/development/github.com/openeggbert/free-eggbert")
PIXTABLES_HPP = FREE_EGGBERT / "include/pixtables.hpp"
DECTABLES_HPP = FREE_EGGBERT / "include/dectables.hpp"
DECTABLES_CPP = FREE_EGGBERT / "src/dectables.cpp"

OBJECT, ELEMENT, EXPLO = "CHOBJECT", "CHELEMENT", "CHEXPLO"

# Confirmed by tracing each table to its DrawIcon/QuickIcon channel
# argument or an explicit `.channel = CHxxx` assignment (see module
# docstring). Tables not listed here are either confirmed non-icon tables,
# confirmed unused, phase-dependent/mixed-channel (table_electro), or
# genuinely unresolved by the investigation - all deliberately left out
# rather than guessed.
CHANNEL_MAP = {
    # CHOBJECT (25)
    "table_blup": OBJECT, "table_bridge": OBJECT, "table_charge": OBJECT,
    "table_marine": OBJECT, "table_plouf": OBJECT, "table_ressort": OBJECT,
    "table_tiplouf": OBJECT,
    "table_decor_eau1": OBJECT, "table_decor_eau2": OBJECT,
    "table_decor_ecraseur": OBJECT, "table_decor_goutte": OBJECT,
    "table_decor_lave": OBJECT, "table_decor_piege1": OBJECT,
    "table_decor_piege2": OBJECT, "table_decor_scie": OBJECT,
    "table_decor_temp": OBJECT, "table_decor_ventb": OBJECT,
    "table_decor_ventd": OBJECT, "table_decor_ventg": OBJECT,
    "table_decor_venth": OBJECT, "table_decor_ventillob": OBJECT,
    "table_decor_ventillod": OBJECT, "table_decor_ventillog": OBJECT,
    "table_decor_ventilloh": OBJECT,
    "table_adapt_decor": OBJECT, "table_adapt_fromage": OBJECT,

    # CHELEMENT (43)
    "table_bulldozer_left": ELEMENT, "table_bulldozer_right": ELEMENT,
    "table_bulldozer_turn2l": ELEMENT, "table_bulldozer_turn2r": ELEMENT,
    "table_poisson_left": ELEMENT, "table_poisson_right": ELEMENT,
    "table_poisson_turn2l": ELEMENT, "table_poisson_turn2r": ELEMENT,
    "table_oiseau_left": ELEMENT, "table_oiseau_right": ELEMENT,
    "table_oiseau_turn2l": ELEMENT, "table_oiseau_turn2r": ELEMENT,
    "table_guepe_left": ELEMENT, "table_guepe_right": ELEMENT,
    "table_guepe_turn2l": ELEMENT, "table_guepe_turn2r": ELEMENT,
    "table_creature_left": ELEMENT, "table_creature_right": ELEMENT,
    "table_creature_turn2": ELEMENT,
    "table_cle": ELEMENT, "table_cle1": ELEMENT, "table_cle2": ELEMENT, "table_cle3": ELEMENT,
    "table_follow1": ELEMENT, "table_follow2": ELEMENT,
    "table_dynamitef": ELEMENT, "table_skate": ELEMENT, "table_shield": ELEMENT,
    "table_power": ELEMENT, "table_invert": ELEMENT, "table_magictrack": ELEMENT,
    "table_shieldtrack": ELEMENT, "table_tresortrack": ELEMENT,
    "table_pollution": ELEMENT, "table_invertstart": ELEMENT, "table_invertstop": ELEMENT,
    "table_glu": ELEMENT, "table_clear": ELEMENT, "table_drinkeffect": ELEMENT,
    "table_shield_blupi": ELEMENT, "table_shieldloop": ELEMENT, "table_magicloop": ELEMENT,

    # CHEXPLO (12)
    "table_explo1": EXPLO, "table_explo2": EXPLO, "table_explo3": EXPLO,
    "table_explo4": EXPLO, "table_explo5": EXPLO, "table_explo6": EXPLO,
    "table_explo7": EXPLO, "table_explo8": EXPLO,
    "table_sploutch1": EXPLO, "table_sploutch2": EXPLO, "table_sploutch3": EXPLO,
    "table_tentacule": EXPLO,
}

# Verified used (icon values feed .icon for TYPE_BLUPIHELICO/TYPE_BLUPITANK
# via the same MoveObjectStepIcon code shape as 5 sibling entity tables,
# all of which are CHELEMENT) but the investigation could not find the
# trailing `.channel =` line for these two types in the traced files, so
# treat the channel as an inferred pattern match, not confirmed evidence -
# included with an explicit caveat rather than silently guessed.
PATTERN_MATCHED_ELEMENT = {
    "table_blupih_left", "table_blupih_right", "table_blupih_turn2l", "table_blupih_turn2r",
    "table_blupit_left", "table_blupit_right", "table_blupit_turn2l", "table_blupit_turn2r",
}
for _name in PATTERN_MATCHED_ELEMENT:
    CHANNEL_MAP[_name] = ELEMENT

# Confirmed NOT icon tables, confirmed unused, or genuinely unresolved -
# never fed into any channel's Group labeling:
#   table_decor_action, table_tutorial, table_blitz, table_vitesse_march,
#   table_vitesse_nage, table_vitesse_surf, table_drinkoffset (not icon
#   tables) / table_invertpanel (unused) / table_electro (phase-dependent,
#   mixed CHBLUPI2+CHELEMENT) / table_chenille, table_chenillei (channel
#   genuinely unresolved).


def parse_short_table(name):
    text = PIXTABLES_HPP.read_text()
    m = re.search(rf"extern short {name}\[\]\s*=\s*\{{(.*?)\}};", text, re.S)
    nums = [int(x) for x in re.findall(r"-?\d+", m.group(1))]
    count = nums[0]
    rest = nums[1:]
    assert len(rest) == count * 6, f"{name}: size mismatch - source may have changed"
    icons = []
    for i in range(count):
        px, py, ox, oy, sx, sy = rest[i * 6 : i * 6 + 6]
        icons.append({"pos_x": px, "pos_y": py, "offset_x": ox, "offset_y": oy, "size_x": sx, "size_y": sy})
    return icons


def parse_named_tables():
    hpp = DECTABLES_HPP.read_text()
    decl_names = re.findall(r"extern\s+(?:const\s+)?int\s+(\w+)\[\]", hpp)
    cpp = DECTABLES_CPP.read_text()
    tables = {}
    for name in decl_names:
        if name in ("table_blupi", "table_mirror"):
            continue
        m = re.search(rf"\bint {re.escape(name)}\[\]\s*=\s*\{{(.*?)\}}\s*;", cpp, re.S)
        if not m:
            continue
        tables[name] = [int(x) for x in re.findall(r"-?\d+", m.group(1))]
    return tables, decl_names  # decl_names preserves dectables.hpp declaration order


def csv_field(s):
    return str(s).replace(";", ",").replace("\n", " ").replace("\r", " ")


def generate(file_name, table_icon_name, channel, icons, named_tables, decl_order):
    # icon_uses[i] = list of (table_name, position) in dectables.hpp
    # declaration order, restricted to tables mapped to this channel.
    icon_uses = {i: [] for i in range(len(icons))}
    out_of_range = 0
    for name in decl_order:
        if CHANNEL_MAP.get(name) != channel or name not in named_tables:
            continue
        for pos, val in enumerate(named_tables[name]):
            if val < 0:
                continue
            if val >= len(icons):
                out_of_range += 1
                continue
            icon_uses[val].append((name, pos))
    if out_of_range:
        print(f"# {file_name}: {out_of_range} out-of-range icon references skipped", file=sys.stderr)

    out = ["File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file"]
    for i, icon in enumerate(icons):
        uses = icon_uses[i]
        if uses:
            primary_name, primary_pos = uses[0]
            group = primary_name
            number_in_group = primary_pos + 1
            note_parts = []
            if primary_name in PATTERN_MATCHED_ELEMENT:
                note_parts.append("channel inferred from sibling code pattern, not directly confirmed")
            others = {}
            for n, _p in uses[1:]:
                others[n] = others.get(n, 0) + 1
            self_reuse = others.pop(primary_name, 0)
            if self_reuse:
                note_parts.append(f"reused {self_reuse} more time(s) within {primary_name} itself")
            if others:
                note_parts.append("also used by: " + ", ".join(f"{n}({c}x)" for n, c in others.items()))
            notes = " | ".join(note_parts)
            tags = "auto:table_blupi_style"
        else:
            group = "?"
            number_in_group = 1
            notes = ""
            tags = f"auto:{table_icon_name}"

        fields = [
            file_name, group, str(number_in_group), str(i + 1), "1",
            str(icon["pos_x"]), str(icon["pos_y"]), str(icon["size_x"]), str(icon["size_y"]),
            csv_field(notes), tags, str(i + 1),
        ]
        out.append(";".join(fields))

    labeled = sum(1 for i in range(len(icons)) if icon_uses[i])
    print(f"# {file_name}: {labeled}/{len(icons)} icons labeled", file=sys.stderr)
    return out


def main():
    named_tables, decl_order = parse_named_tables()

    jobs = [
        ("object.blp", "table_icon_object", OBJECT),
        ("element.blp", "table_icon_element", ELEMENT),
        ("explo.blp", "table_icon_explo", EXPLO),
    ]
    for file_name, table_icon_name, channel in jobs:
        icons = parse_short_table(table_icon_name)
        rows = generate(file_name, table_icon_name, channel, icons, named_tables, decl_order)
        out_path = Path(f"{file_name.replace('.blp', '')}_v2_rows.csv")
        out_path.write_text("\n".join(rows) + "\n")
        print(f"wrote {out_path} ({len(rows) - 1} rows)", file=sys.stderr)


if __name__ == "__main__":
    main()
