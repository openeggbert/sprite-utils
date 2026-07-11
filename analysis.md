# sprite-utils Completion Analysis

**Date:** 2026-07-11
**Author:** Prepared by Claude Code, at the request of Robert Vokac (project owner)
**Scope:** `sprite-utils` (this repo) + the sprite-sheet CSVs on the library drive
(`drive.openeggbert.com/Blupi/Games/Speedy_Blupi_(Windows)/spritesheets/`) +
the `free-eggbert` decompilation repo + the Ghidra project
`speedy-blupi-2_robertvokac/speedy-blupi-2`.

**Purpose:** `sprite-utils` was started about two years ago and never finished.
This document (a) inventories exactly what `sprite-utils` can and cannot do
today, (b) identifies the *actual* bottleneck (it is not the tool — it is the
manual sprite-sheet annotation work), (c) shows that a large part of that
manual work has **already been solved, unused, inside the `free-eggbert`
repository**, extracted directly from the original game executable, and
(d) lays out a concrete, AI-assisted plan to finish both the tool and the
data.

---

## 1. Executive summary

`sprite-utils` itself is a small, mostly-working C++/OpenCV command-line tool
that annotates sprite sheets (draws a dashed rectangle + sprite number over
each sprite, based on a semicolon-CSV describing rectangles). Only 3 of its 6
planned commands are wired up (`draw`, `help`, `version`); `restore`,
`extract`, and `gifs` exist only as an enum value with no implementation.
Finishing the tool itself is a small, mechanical, well-scoped job — a good
fit for AI-assisted one-shot implementation (see Track A below).

**The real bottleneck is not the tool, it is the data.** Inspection of the
two sprite-sheet CSVs shows:

- `speedy_blupi_I.spritesheet.csv`: 1157 data rows, **1116 (96%) still have
  `Group=?`** — i.e. the pixel rectangle may be known, but which animation/
  character/object it belongs to is not recorded.
- `speedy_blupi_II.spritesheet.csv`: 1301 data rows, **1012 (78%) still have
  `Group=?`**.
- Both CSVs only cover `blupi000.blp` + a handful of single-purpose sheets
  (`button00.blp`, `element.blp`, `explo.blp`, `jauge.blp`, `object.blp`,
  and — CSV I only — `text.blp`). **`blupi001–003.blp` (3 more character-color
  variants) and all 32 `decor000–031.blp` world-tile sheets have not been
  started at all.**

This is consistent with two years of slow, manual, visual pixel-measurement
work (using `sprite-utils draw` to check guesses against the rendered
rectangles) — exactly the kind of work that does not scale by hand.

**The unlock:** the `free-eggbert` decompilation of this exact game already
contains, sitting unused in `include/pixtables.hpp` and `src/dectables.cpp`,
tables that were mechanically extracted from the original `BLUPI.exe` (v2.2
EN) at fixed byte offsets by `util/extract_pixtables.py` and
`util/extract_tableblupi.py`. These tables contain, per sprite ("icon" in the
game's own terminology):

- the **exact pixel rectangle** (`X`, `Y`, `Width`, `Height`) inside a
  specific sprite sheet file — precisely the columns `sprite-utils`' CSV
  needs and that are currently measured by hand, and
- for a large fraction of sprites, the **semantic animation/group name**
  (via ~150 named tables like `table_bulldozer_left`, `table_shield`,
  `table_explo1`, and the character animation table `table_blupi`, which is
  keyed by the game's own `ACTION_*` names in `include/def.hpp`).

In other words: the game's own binary already "knows" the answer to most of
what `sprite-utils`' CSVs are trying to reconstruct by hand. Section 5
(Track B) lays out how to turn that into an auto-generated, mostly-complete
CSV instead of continuing to measure pixels by eye.

**`free-eggbert` only decompiles Speedy Eggbert/Blupi II — Speedy Blupi I
needed its own binary.** `free-eggbert`'s tables are specific to
`BLUPI.exe` v2.2 (the source for `speedy_blupi_II.spritesheet.csv`); they
say nothing about the binary behind `speedy_blupi_I.spritesheet.csv`.
Rather than leave this as a future task, this session located the actual
Speedy Blupi I game files (`Speedy_Blupi_I.7z` on the library drive, outside
the Ghidra project and outside `free-eggbert`), wrote a small byte-signature
scanner, found the four equivalent `IconPack` tables directly inside that
binary, and **visually confirmed all four by rendering their rectangles
onto the real v1.0 sprite sheets** — every rectangle lands correctly on its
sprite. See Section 4.6 for the exact offsets, the method, and the
rendered proof. The pixel-rectangle half of the problem is now solved for
*both* games; only the `Group`-name overlay for v1.0 remains open.

---

## 2. Current state of `sprite-utils`

Implemented (`include/SpriteUtilsCommand.h`, `src/SpriteUtils.cpp`):

| Command | Status | What it does |
|---|---|---|
| `draw` | **Implemented** (`src/DrawCommand.cpp`) | For every image in a working directory that has rows in the sprite-sheet CSV: makes/restores a `.backup` copy, draws a dashed rectangle around each sprite rect, optionally overlays the sprite's computed sequence number (small 3×5 or "double-sized" hand-coded bitmap font), and writes the image back preserving its original bit depth (handles 8-bit BMP, 16-bit RGB565 `.blp`/BMP specially with a custom writer). |
| `help` | Implemented, but **stale** | `src/HelpCommand.cpp` documents options (`color`, `files`, `groups`, `positon`, `number-per-group`) that do not match the real `--`-prefixed CLI options actually implemented in `SpriteUtilsArgs`/`SpriteUtilsOptions`. |
| `version` | Implemented | Trivial. |
| `restore` | **Not implemented** | Enum value exists (`SpriteUtilsCommand.h:34`); no `RestoreCommand` class/dispatch exists. Today you restore a clean image by manually copying `<file>.backup` back over `<file>`. |
| `extract` | **Not implemented** | Would cut each CSV rectangle out of a sheet into its own image file — the "core cutting operation" the tool cannot yet do (per `web/roadmap.html`, which is itself an AI-generated evidence-based roadmap already present in the repo and consistent with this analysis). |
| `gifs` | **Not implemented** | Would build an animated GIF per sprite group (animation preview). Low priority until `Group` data actually exists for most rows (see Section 3). |

Other known gaps (already correctly documented in `web/known-limitations.html`,
which reads as a prior AI-assisted documentation pass over this same repo):

- No automated tests.
- `SpriteSheet::lastX/lastWidth/lastHeight` are `static` class members →
  non-reentrant, global mutable state.
- No exception handling in `main()` — any error is an unhandled-exception
  crash.
- CSV parser requires strict `row==1,column==1` first row and strictly
  sequential row/column values per file — sparse/non-standard sheets are
  rejected outright.
- The height "negation" convention for multi-column rows
  (`src/SpriteSheet.cpp:processLine`) is a real but non-obvious encoding —
  worth documenting inline, not just in `web/`.
- `numbers.png` / `numbers_double_sized.png` at the repo root are **not
  referenced by any source file** (`DrawCommand.cpp` draws its own digits
  from hand-coded `bool[]` bitmap masks). They appear to be leftover
  reference images from an earlier Java predecessor of this tool. Candidate
  for deletion or for an explicit comment explaining why they're kept.

None of this is architecturally hard to finish — see Track A in Section 5.

---

## 3. Current state of the sprite-sheet CSVs (the actual bottleneck)

Format (already well documented in `web/file-formats.html` /
`web/tutorials/csv-format.html`, confirmed against the real files):

```
File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file
```

Measured directly from the two CSVs on the library drive:

| File | Data rows | `Group="?"` rows | Distinct sheet files referenced |
|---|---:|---:|---|
| `speedy_blupi_I.spritesheet.csv` | 1157 | 1116 (96.5%) | `blupi000.blp`, `button00.blp`, `element.blp`, `explo.blp`, `jauge.blp`, `object.blp`, `text.blp` |
| `speedy_blupi_II.spritesheet.csv` | 1301 | 1012 (77.8%) | `blupi000.blp`, `button00.blp`, `element.blp`, `explo.blp`, `jauge.blp`, `object.blp` |

Sheets that exist on disk (`free-eggbert/gamefiles/IMAGE08/`) but have **zero**
rows in either CSV: `blupi001.blp`, `blupi002.blp`, `blupi003.blp`, and all of
`decor000.blp`…`decor031.blp` (32 files).

Where a `Group` value *is* filled in (e.g. `Yellow_Eggbert_Swimming_Right`,
`Yellow_Eggbert_Born`, `Yellow Bomb`, `spring`), it was clearly assigned by a
human recognizing the sprite visually — exactly the labor `sprite-utils` was
built to make faster to *check* (via `draw`) but that it never helped
*generate*.

**Conclusion:** the pixel-rectangle reconstruction (X/Y/Width/Height) and the
semantic grouping (Group/Number in Group) are two separate jobs, both mostly
unfinished, and — as Section 4 shows — both are largely already solved by
data sitting in `free-eggbert`.

---

## 4. The `free-eggbert` / Ghidra findings

### 4.1 Pixel-rect tables already extracted from the game EXE

`free-eggbert/include/pixtables.hpp` defines, with an explicit comment
documenting the struct layout:

```c
typedef struct { short x; short y; } ShortPOINT;
typedef struct { ShortPOINT pos; ShortPOINT offset; ShortPOINT size; } IconPack;
```

Four arrays of these 6-short records exist, each starting with a count:

| Array | Count (`arr[0]`) | Extracted from `BLUPI.exe` at offset |
|---|---:|---|
| `table_icon_blupi` | 336 | `0x81bf8` |
| `table_icon_object` | 441 | `0x82bc0` |
| `table_icon_element` | 289 | `0x84070` |
| `table_icon_explo` | 100 | `0x84e00` |

They were produced by `free-eggbert/util/extract_pixtables.py`, a ~30-line
script (comment: *"extract pixtables from speedy blupi 2.2 english ver"*,
signed "JUMMY WAS HERE" — a collaborator from the original decompilation
project `free-eggbert` was forked from) that reads these fixed byte offsets
directly out of `BLUPI.exe` and dumps them as C arrays. **This is exactly
the technique you hypothesized** — and it was already done, two years ago,
by someone else on this project, for exactly this purpose (building the
in-game icon-drawing tables), just never connected to `sprite-utils`.

Each `IconPack` record's `pos` (x,y) and `size` (w,h) are **directly** the
`X`, `Y`, `Width`, `Height` columns `sprite-utils`' CSV wants. `offset` is a
separate draw-position adjustment (for centering the sprite against its
in-game anchor point), not needed for the sheet CSV itself but potentially
useful `Notes` metadata.

### 4.2 Runtime confirmation — which file each table belongs to

A dedicated investigation of `free-eggbert/src/pixmap.cpp` (function
`CPixmap::QuickIcon`, `pixmap.cpp:312-358`, and `CPixmap::DrawIcon`,
`pixmap.cpp:1226-1299`) confirms exactly how these tables are used at
runtime: each record is read as `table[rank*6 + N + 1]` (the leading `+1`
skips the count at index 0) and used to build the source rectangle for a
blit. The channel (`CHOBJECT`, `CHELEMENT`, `CHBLUPI`/`CHBLUPI1-3`,
`CHEXPLO`) selects which table to index into, and each channel is bound to
exactly one sprite-sheet file in `CPixmap::CacheAll` (`pixmap.cpp:925-1115`):

| Channel | Table | File | Confirmed on-disk BMP dimensions |
|---|---|---|---|
| `CHOBJECT` | `table_icon_object` | `object.blp` | 1024×1327 |
| `CHELEMENT` | `table_icon_element` | `element.blp` | 896×644 |
| `CHEXPLO` | `table_icon_explo` | `explo.blp` | 496×1885 |
| `CHBLUPI` / `CHBLUPI1` / `CHBLUPI2` / `CHBLUPI3` | `table_icon_blupi` | `blupi000.blp` / `blupi001.blp` / `blupi002.blp` / `blupi003.blp` | all four: 800×906 |

The four `blupi0NN.blp` files are byte-identical in size (725,878 bytes) and
share the *same* geometry table — they are recolored variants (multiplayer
team colors), selected purely by `CDecor::GetBlupiChannelActual()`
(`decblupi.cpp:15-31`), not by icon rank. The measured file dimensions line
up with the table's coordinate ranges (e.g. `table_icon_blupi` entries reach
`pos.x` up to ~779, consistent with an 800px-wide canvas), which is a strong
independent sanity check that this mapping is correct.

**Practical meaning:** you do not need to run anything by hand — the exact
pixel rectangle for all 336 + 441 + 289 + 100 = **1166 sprite icons** across
7 files (`blupi000–003.blp` sharing one table, `object.blp`, `element.blp`,
`explo.blp`) is already sitting, extracted, in the `free-eggbert` source
tree.

### 4.3 Semantic group names — also already in the source tree

`free-eggbert/include/dectables.hpp` declares roughly 150 more tables beyond
`table_icon_*`, with names that are themselves the sprite "Group" you'd
otherwise assign by hand: `table_bulldozer_left/right/turn2l/turn2r`,
`table_poisson_left/right/...` (fish), `table_oiseau_*` (bird),
`table_guepe_*` (wasp), `table_creature_*`, `table_explo1`…`table_explo8`,
`table_sploutch1-3` (splash), `table_shield`, `table_shield_blupi`,
`table_charge`, `table_glu` (glue), `table_chenille*` (caterpillar/tank
tread), `table_marine`, `table_tresortrack` (treasure), and more.

Their definitions in `src/dectables.cpp` are plain, flat integer arrays of
**icon indices** — e.g.:

```c
int table_bulldozer_left[]  = { 66, 66, 67, 67, 66, 66, 65, 65 };
int table_shield[]          = { 144,145,146,147,148,149,150,151,266,267,268,269,270,271,272,273 };
```

Each value is an index into `table_icon_object` (for these particular
tables) — i.e. `Group = "BULLDOZER_LEFT"`, `NumberInGroup = <position in
array>`, and the pixel rect comes straight from `table_icon_object[value]`.

The Blupi character himself is handled specially by `table_blupi`
(`dectables.cpp:597`, 2842 ints, extracted by `util/extract_tableblupi.py`
from EXE offset `0x34e60`). Confirmed record format (from tracing its
consumer, `decblupi.cpp:169-181`):

```
[action_id, frame_count, default_phase, icon_0, icon_1, ..., icon_(frame_count-1)]
```
repeated until `action_id == 0`. `action_id` corresponds to the ~87
`ACTION_*` `#define`s in `include/def.hpp:142-228` (several already carry
English/French comments, e.g. `ACTION_MARCH` = walk). This is a
ready-made `Group` name plus correctly ordered animation frames for the
`blupi0NN.blp` sheets.

There is also `table_mirror` (335 ints, `dectables.cpp:588-590`, extracted
by `util/extract_tablemirror.py` from offset `0x37ac8`) — a left/right
mirrored-icon lookup applied only to `CHBLUPI`, useful for a `Notes`/`Tags`
annotation like "mirrored variant of icon N" rather than a distinct sprite
to re-measure.

**Caveat:** `table_icon_object`/`element`/`explo` indices that are *not*
referenced by any named table in `dectables.cpp` have no recovered semantic
name (some `dectables.cpp` arrays are still named after their raw EXE
offset, e.g. `table_36418`, meaning their purpose hasn't been reverse
engineered yet either). Expect close to full `Group` coverage for the
`blupi0NN.blp` sheets (everything reachable from `table_blupi`) and partial
— but still very substantial — coverage for `object.blp`/`element.blp`/
`explo.blp`, with a residual of genuinely-unidentified icons for later.

### 4.4 What this does *not* cover: the 32 `decor0NN.blp` tile sheets

`CHDECOR` is deliberately **not** backed by a `pixtables.hpp` table. In
`CPixmap::CacheAll` it's cached with `iconDim = {0,0}`
(`pixmap.cpp:1069-1076`), which makes the table/grid lookup path in
`DrawIcon` a no-op for this channel (`pixmap.cpp:1277` guard). Decor tiles
are instead blitted with directly-computed pixel rects via `DrawPart`/
`DrawMap` (e.g. `pixmap.cpp:1135`), and the filename is built with
`sprintf(filename, "decor%.3d.blp", region)` (`pixmap.cpp:1068`) — matching
the 32 files on disk.

This means decor tiles need a **different**, likely simpler extraction
approach: since world tiles are drawn from a uniform grid rather than an
arbitrary per-icon rectangle table, the CSV rows for `decor0NN.blp` can
probably be generated from a small number of grid parameters (tile
width/height, tiles-per-row) once those constants are located in
`pixmap.cpp`/`decor.hpp`, rather than requiring a 1166-entry lookup table.
This is real follow-up work, not yet done as part of this analysis — flagged
here so it isn't mistaken for something Section 5's plan already solves.

### 4.5 Binary/version specificity — a real constraint, but a solved one

The offsets in `util/extract_pixtables.py` / `extract_tableblupi.py` /
`extract_tablemirror.py` are hardcoded for one exact binary: `BLUPI.exe`,
"speedy blupi 2.2 english ver" (per the scripts' own comments) — i.e. the
source for `speedy_blupi_II.spritesheet.csv`. They do **not** generalize
to the binary behind `speedy_blupi_I.spritesheet.csv` without re-locating
the equivalent tables in that binary — sprite art and table layout differ
release to release. Two independent ways to do that relocation:

- **Ghidra Version Tracking** (no byte-scanning needed, but requires Ghidra
  and existing labels): the Ghidra project already has both
  `SpeedyBlupi2_v2.2_EN.exe` and `SpeedyBlupi_v1.0_EN.exe` imported, **and
  an existing correlation session between them** (`"SB2 v2.2 to SB v1.0"`,
  per `free-eggbert/audit.md` Appendix A, independently confirmed by reading
  the Ghidra project's `idata/~index.dat`/`.prp` files directly in this
  session). This is the right tool if the v2.2 data locations are already
  labeled as symbols in that Ghidra program — it would propose the matching
  v1.0 address directly. (Not verified in this session whether those labels
  exist yet; see Section 4.6 for a way that doesn't need them.)
- **A standalone byte-signature scan** (no Ghidra needed at all) — see
  Section 4.6, which carries this out end-to-end for the actual Speedy
  Blupi I binary and visually confirms the result.

### 4.6 Speedy Blupi I (v1.0) — tables located and visually confirmed in this session

Per your note that `free-eggbert` only covers *Speedy Eggbert/Blupi II*, and
that game EXEs also exist outside the Ghidra project: the original v1.0
game files are archived directly on the library drive at
`Blupi/Games/Speedy_Blupi_(Windows)/Speedy_Blupi_I.7z`, containing
`Speedy_Blupi_I/BLUPI.EXE` (535,552 bytes, dated 1998-10-14) plus its own
`IMAGE08/BLUPI000-003.BLP`, `OBJECT.BLP`, `ELEMENT.BLP`, `EXPLO.BLP` (all
structurally the same *kind* of assets as v2.2, but different pixel
dimensions — this is a different, older build, not a re-export of v2.2's
data).

Because the `IconPack` record shape is known exactly (6 little-endian
`unsigned short`s: `pos.x, pos.y, offset.x, offset.y, size.x, size.y`,
preceded by a `u16` record count — see Section 4.1), it is possible to
**find the equivalent tables in a new binary without Ghidra at all**, by
brute-force scanning every 2-byte-aligned file offset for a run of records
that (a) all decode to plausible small values (position inside the known
`.blp` canvas size, width/height in a few-pixel-to-~100-pixel range, a small
draw offset) and (b) span at least ~15-20 records validating cleanly in a
row (real tables are 50-300+ records; random code/relocation bytes essentially
never pass this many checks in sequence). This was implemented as a ~40-line
Python script in this session and run directly against `BLUPI.EXE` (v1.0),
using the real `.blp` canvas sizes read from each file's own BMP header as
the plausibility bound:

| Table (v1.0) | Canvas (`.blp` file) | File offset (count field) | Record count | Records start |
|---|---|---|---:|---|
| `table_icon_blupi` equiv. | `BLUPI000.BLP` (784×770) | `0x775d8` | 291 | `0x775da` |
| `table_icon_object` equiv. | `OBJECT.BLP` (896×987) | `0x78380` | 246 | `0x78382` |
| `table_icon_element` equiv. | `ELEMENT.BLP` (608×591) | `0x78f10` | 187 | `0x78f12` |
| `table_icon_explo` equiv. | `EXPLO.BLP` (784×868) | `0x797d8` | 54 | `0x797da` |

Notably, these four tables sit **back-to-back in the file** (each one's byte
range ends within a few bytes of the next one's start), exactly like
`table_icon_blupi/object/element/explo` do in the v2.2 binary (Section 4.1)
— strong independent structural confirmation that this is the same group of
four tables, just at v1.0-specific addresses and with v1.0-specific counts
(291/246/187/54 vs. v2.2's 336/441/289/100 — different games/art, similar
role).

**All four were visually confirmed** by decoding each table and drawing its
rectangles directly onto the matching `.blp` file (converted to PNG,
inspected as an image): every rectangle lands cleanly on exactly one
sprite/tile/character-frame/explosion-frame, sheet by sheet, with no
systematic offset or drift across ~780 combined records. This is the same
level of confidence as the v2.2 tables already merged into `free-eggbert` —
it is not a guess.

Reproduction script (adapt paths as needed; only reads the `.exe`/`.blp`
files, does not modify or redistribute them):

```python
import struct
from PIL import Image, ImageDraw

TABLES = [
    # (file_offset_of_count, record_count, matching_blp_file)
    (0x775d8, 291, "BLUPI000.BLP"),
    (0x78380, 246, "OBJECT.BLP"),
    (0x78f10, 187, "ELEMENT.BLP"),
    (0x797d8, 54,  "EXPLO.BLP"),
]

with open("BLUPI.EXE", "rb") as f:
    data = f.read()

for count_off, count, blp in TABLES:
    p = count_off + 2  # skip the leading u16 record count
    img = Image.open(blp).convert("RGB")
    draw = ImageDraw.Draw(img)
    records = []
    for _ in range(count):
        px, py, ox, oy, sw, sh = struct.unpack_from("<6H", data, p)
        records.append((px, py, ox, oy, sw, sh))
        draw.rectangle([px, py, px + sw, py + sh], outline=(255, 0, 0))
        p += 12
    img.save(blp + ".check.png")   # visual sanity check
    # records[i] = (X, Y, offsetX, offsetY, Width, Height) for icon i
```

**Practical meaning:** the same X/Y/Width/Height auto-generation described
in Track B (Section 5) is now directly runnable for **both** games, not
just v2.2 — closing the gap you flagged. The semantic `Group` overlay
(Section 4.3, `table_blupi`/named `dectables.cpp` tables) still needs its
own v1.0 location — that table has a different, self-describing record
format (`[action_id, frame_count, phase, icon...]` rather than fixed-size
records), so it needs either the same byte-scanning approach adapted to a
variable-length record, or Ghidra/Version Tracking against the confirmed
v2.2 `table_blupi` location. Not yet done as part of this session.

**One important caveat on the game/CSV pairing**: `speedy_blupi_I.spritesheet.csv`'s
existing hand-filled rows (`File=blupi000.blp`) should be checked against
*this* v1.0 table before trusting it as ground truth for validation, since
Speedy Blupi I's `blupi000.blp` (784×770) is a different image than Speedy
Blupi II's (800×906) — they are not interchangeable, so make sure any
cross-check (Section 6, step 1) uses the v1.0 table against the v1.0 CSV
rows/images, and the v2.2 table against the v2.2 CSV rows/images.

---

## 5. Recommended plan

Two tracks, independent and safe to run in parallel or interleaved.

### Track A — Finish `sprite-utils` as a tool

Small, mechanical, low-risk; a good target for an AI coding agent to
implement close to end-to-end in one or two sessions:

1. **Implement `extract`**: for each CSV row, crop `[X, Y, X+Width, Y+Height]`
   out of the source image and write it to its own file (naming convention
   TBD — e.g. `<file-stem>__<group>__<numberInGroup>.png`). This is the
   single highest-value missing command — it's the actual "cut the sprite
   out" step, and Track B below will make it immediately useful on ~1166
   already-rectangled icons.
2. **Implement `restore`**: iterate the working directory, and for every
   `<file>.backup` present, copy it back over `<file>`. A few lines, mirrors
   logic already in `DrawCommand::run`.
3. **Fix `HelpCommand.cpp`**: rewrite the help text to match the real
   `--`-prefixed options in `SpriteUtilsArgs`/`SpriteUtilsOptions` (already
   precisely diagnosed in `web/known-limitations.html`).
4. **Add a test suite**: CSV-parsing edge cases are the highest-value target
   first (auto-X computation, height-negation encoding, `skipskip`
   sentinel, first-row/sequential-row validation, minimum-column
   validation) — all already precisely described in
   `web/tutorials/csv-format.html` / `web/file-formats.html`, so writing
   tests from that spec is close to mechanical.
5. **Remove the global static state** in `SpriteSheet` (`lastX`,
   `lastWidth`, `lastHeight`) so the class is reentrant — needed if Track B
   ends up processing multiple sheets in one process/one test run.
6. **`gifs`**: deprioritize until `Group`/`Number in Group` data actually
   exists for most rows (Track B fixes exactly this), since a GIF exporter
   is only useful once sprites are grouped into real animations.
7. **New command for Track B** (name suggestion: `import` or `generate`):
   reads a small JSON/CSV dump of one of `free-eggbert`'s tables (see Track
   B step 1) and emits sprite-sheet CSV rows directly — this is the command
   that actually closes the loop described in this document.

### Track B — Auto-populate the CSVs from data `free-eggbert` already has

1. **Dump the tables to a portable format.** Write a small one-off script
   (Python, or a tiny throwaway C++ program compiled against
   `free-eggbert`'s existing headers) that reads `table_icon_blupi`,
   `table_icon_object`, `table_icon_element`, `table_icon_explo` (from
   `include/pixtables.hpp`) and the ~150 named tables + `table_blupi` +
   `table_mirror` (from `src/dectables.cpp`) and dumps them as JSON. No
   Ghidra or EXE access is needed for this step — the v2.2 values are
   already committed as C source in `free-eggbert`.
2. **Generate rect rows.** For each of the 7 files bound to
   `table_icon_blupi`/`object`/`element`/`explo` (Section 4.2's table), walk
   the corresponding array and emit one CSV row per icon:
   `File=<mapped file>`, `X=pos.x`, `Y=pos.y`, `Width=size.x`,
   `Height=size.y`, `Number per file=<icon rank>`. Because `sprite-utils`'
   CSV parser already supports an explicit (non-empty) `X` value
   (`web/file-formats.html`, "Automatic X Computation" section — empty is
   optional, not required), **no `SpriteSheet`/`SpriteSheetRow` code changes
   are needed to consume fully-precomputed rows** — `Row`/`Column` can be
   synthesized as a trivial running sequence (e.g. `Row=1`,
   `Column=<running count>`) purely to satisfy the parser's sequential-index
   validation, since the real geometry no longer depends on it.
3. **Overlay `Group`/`Number in Group`.** For every icon index that appears
   in `table_blupi` or in one of the ~150 named `dectables.cpp` tables,
   set `Group` to the table's name (e.g. `ACTION_MARCH`, `BULLDOZER_LEFT`,
   `SHIELD`) and `Number in Group` to its position within that
   table/action's frame list. This is the step that should move `Group`
   coverage from today's 4% (CSV I) / 22% (CSV II) to something close to
   100% for the `blupi0NN.blp`/`object.blp`/`element.blp`/`explo.blp` sheets,
   automatically, with zero pixel-by-pixel guessing.
4. **Reconcile with the existing CSVs rather than overwriting them.** The
   existing hand-assigned `Group` values (e.g.
   `Yellow_Eggbert_Swimming_Right`) may be more human-readable than the raw
   `ACTION_*`/table names — treat the generated CSV as a merge candidate
   (fill only `Group="?"` rows, or emit a second `tags`/`notes` column
   cross-referencing the `ACTION_*`/table name) rather than a blind replace,
   and diff the generated `X`/`Y`/`Width`/`Height` against any already
   manually-verified rows as a correctness check on the whole pipeline
   before trusting it for the untouched rows.
5. **Decor tiles (`decor000-031.blp`, 32 files): separate, follow-up
   effort.** Locate the grid constants used by `CHDECOR` in `pixmap.cpp`
   (search around `pixmap.cpp:1069-1076` and `decor.hpp`) and generate a
   uniform-grid CSV per file rather than reusing the icon-table approach —
   see Section 4.4.
6. **Speedy Blupi I (v1.0) coverage — pixel-rect tables already located.**
   Section 4.6 locates and visually confirms all four `IconPack` tables
   directly inside `Speedy_Blupi_I/BLUPI.EXE` (extracted from
   `Speedy_Blupi_I.7z` on the library drive), by byte-signature scanning —
   no Ghidra needed. Steps 2 and 4 above are therefore already directly
   runnable for v1.0 using the offsets in Section 4.6's table. Still open:
   locating `table_blupi`'s v1.0 equivalent for the `Group` overlay (step
   3) — either extend the byte-scan to `table_blupi`'s variable-length
   record format, or use Ghidra Version Tracking's `"SB v1.0 to ..."`
   sessions once the v2.2 `table_blupi` address is labeled as a symbol.

---

## 6. Concrete first actions

Ordered so each step is cheap to verify before investing in the next:

1. Dump just `table_icon_blupi` (smallest, 336 entries) to JSON/CSV and
   generate rows for `blupi000.blp` only. Run `sprite-utils draw` on it and
   visually compare against the existing hand-measured rows already present
   for `blupi000.blp` in `speedy_blupi_II.spritesheet.csv` — if the
   rectangles land in the same places, the extraction pipeline is validated
   end-to-end on real, already-trusted data before scaling up.
2. Extend to `table_blupi` for `Group`/`Number in Group` on the same file,
   spot-check a handful of `ACTION_*` groups against what a human would call
   them (e.g. does `ACTION_MARCH`'s frame sequence actually look like
   walking when drawn in order?).
3. Once validated, generate the remaining `object.blp`/`element.blp`/
   `explo.blp` rows plus the ~150 named-table `Group` overlay in one pass.
4. Implement `sprite-utils extract` (Track A step 1) so the newly-complete
   `blupi000-003.blp`/`object.blp`/`element.blp`/`explo.blp` rows can
   immediately be cut into individual sprite files — a concrete, visible
   payoff before tackling the harder decor-tile / v1.0 follow-up work.
5. Decor tiles (Section 5, Track B step 5) remain a separate, independently-
   scoped follow-up — the grid constants haven't been located yet.
6. Speedy Blupi I (v1.0) pixel rects are **already unblocked**: Section 4.6
   found and visually validated all four `IconPack` tables directly in
   `BLUPI.EXE` (v1.0) in this session — generating v1.0 `X`/`Y`/`Width`/
   `Height` rows (step 2 above) can proceed immediately using those offsets.
   Only the v1.0 `Group` overlay (step 3, equivalent to `table_blupi`) is
   still open.

---

## 7. Other resources in the `openeggbert` ecosystem worth checking before re-deriving anything

- **`free-eggbert/audit.md`** (dated 2026-07-08, three days before this
  document) — a much broader decompilation-completion audit covering all of
  `free-eggbert`, not just sprite tables. Recommends adopting a
  `CLAUDE.md`/`NEXT.md`/`plan.md` knowledge-persistence convention (already
  proven in sibling repos `free-direct`/`sharp-runtime`) so that multi-year,
  multi-session AI-assisted work doesn't get re-derived from scratch each
  time. **The same convention would directly benefit `sprite-utils`**,
  which currently has none of these files — worth adding once Track A/B
  above are underway, so the next session (AI or human) doesn't have to
  re-read this whole document to get oriented.
- **`tiled-blupi`** (sibling repo, confirmed present at
  `/rv/data/development/github.com/openeggbert/tiled-blupi`) — per
  `audit.md`, already has documentation pages (`blp-format.html`,
  `map-properties.html`) about the `.blp` format family. Check before
  re-deriving `.blp` file structure from scratch (this analysis did not
  need to, since `sprite-utils`/`free-eggbert` already load `.blp` as plain
  BMP-with-a-different-extension, but it may be useful for the decor-tile
  follow-up in Section 5 step 5).
- **`speedyblupi-data`** (sibling repo, per `audit.md`) — canonical original
  asset archive; potentially a cleaner source of the raw `.blp` files than
  ad hoc copies, if provenance/licensing ever needs to be double-checked.

---

## 8. Risks and caveats

- **Provenance/licensing**: the extracted tables and the game assets
  (`.blp` files) come from a commercial game whose source was never
  released. Consistent with `free-eggbert`'s existing framing (GPLv3 for
  the *reconstructed code*, assets not redistributed, "must be obtained
  from an original copy"), any generated CSV/tooling should be treated the
  same way — the tables (small integer arrays) are fine to commit as data,
  the original `.blp`/`.exe` assets should not be redistributed via this
  repo.
- **Table completeness is not 100%**: several `dectables.cpp` arrays are
  still named after raw EXE offsets (`table_36418`, etc.), meaning a
  residual set of `object.blp`/`element.blp`/`explo.blp` icons will still
  come out with `Group=?` even after Track B — expect a large improvement,
  not total completion, in one pass.
- **Do not trust table semantics without a visual check.** `ACTION_*`
  names and French/English comments in `def.hpp` are a strong prior but not
  proof — validate a sample of generated `Group` assignments by actually
  looking at the rendered sprites (via `sprite-utils draw`, or a quick
  `extract` + grid contact sheet) before treating the auto-generated CSV as
  ground truth, the same "ground yourself in the actual artifact, not
  fluent-sounding inference" principle `free-eggbert/audit.md` §7.2 already
  argues for at the code level.
- **Binary specificity (Section 4.5–4.6)** applies to any future variant
  beyond v1.0/v2.2 English too (demos, `SE`/`_EGAMES` variants,
  localizations) — each will need its own offset-relocation pass. Section
  4.6 shows the byte-signature-scan method works even without Ghidra/Version
  Tracking, as long as you can get the target `.exe` and the matching
  `.blp` files (for the canvas-size plausibility bound) — both are
  available for most known variants either in the Ghidra project or, as
  with Speedy Blupi I, directly on the library drive.
- **Where the v1.0 game files came from**: `Speedy_Blupi_I.7z` /
  `Speedy_Blupi_Demo.7z` on the library drive (outside both `free-eggbert`
  and the Ghidra project) — worth remembering that game EXEs/assets exist
  in more places than just the Ghidra project (per your own note), and it's
  worth checking the library drive directly before assuming Ghidra is
  required for a given variant.

---

## Appendix — Key file/line references

| Claim | File:line |
|---|---|
| Sprite-sheet CSV column spec | `sprite-utils/web/file-formats.html` (also `include/SpriteSheetRow.h`, `src/SpriteSheet.cpp`) |
| Only `draw`/`help`/`version` wired | `sprite-utils/src/SpriteUtils.cpp`; enum at `include/SpriteUtilsCommand.h:34` |
| Stale help text | `sprite-utils/src/HelpCommand.cpp:29-55` |
| `IconPack` struct + 4 pixel-rect tables | `free-eggbert/include/pixtables.hpp:1-19` |
| Pixtable extraction script + offsets | `free-eggbert/util/extract_pixtables.py` |
| `table_icon_*` runtime usage / `rank*6+N+1` indexing | `free-eggbert/src/pixmap.cpp:312-358` (`QuickIcon`), `:1226-1299` (`DrawIcon`) |
| Channel → file binding | `free-eggbert/src/pixmap.cpp:925-1115` (`CacheAll`) |
| Blupi color-variant channel selection | `free-eggbert/src/decblupi.cpp:15-31` |
| `table_blupi` record format | `free-eggbert/src/dectables.cpp:597` (definition), `src/decblupi.cpp:169-181` (consumer) |
| `ACTION_*` names | `free-eggbert/include/def.hpp:142-228` |
| ~150 named animation tables | `free-eggbert/include/dectables.hpp`, defined in `free-eggbert/src/dectables.cpp` |
| `table_mirror` | `free-eggbert/src/dectables.cpp:588-590`, consumer `src/decblupi.cpp:204-207` |
| `CHDECOR` uniform-grid / no pixtable | `free-eggbert/src/pixmap.cpp:1068-1076`, `:1277` |
| Ghidra Version Tracking session `SB2 v2.2 to SB v1.0` | `free-eggbert/audit.md` Appendix A; confirmed directly in `Speedy Blupi 2.rep/idata/*/*.prp` (`CONTENT_TYPE=VersionTracking`, `NAME="SB2 v2.2 to SB v1.0"`) |
| Speedy Blupi I (v1.0) game files | `drive.openeggbert.com/Blupi/Games/Speedy_Blupi_(Windows)/Speedy_Blupi_I.7z` → `Speedy_Blupi_I/BLUPI.EXE` (535,552 bytes) + `IMAGE08/BLUPI000-003.BLP`, `OBJECT.BLP`, `ELEMENT.BLP`, `EXPLO.BLP` |
| Speedy Blupi I `IconPack` table offsets (found + visually confirmed this session) | Section 4.6 table: `0x775d8`/291 (blupi), `0x78380`/246 (object), `0x78f10`/187 (element), `0x797d8`/54 (explo), all inside `BLUPI.EXE` v1.0 |
