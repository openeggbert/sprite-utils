# NEXT.md — sprite-utils handoff

*Last updated: 2026-07-11 (branch: `develop`, HEAD `f592c07`) — Phases 0-3 all done, plus a new `pack` command (§3.5) added afterward. 34 tests / 99 checks passing (`cmake --build build && ctest` / `./build/sprite_utils_tests`).*

## 1. Project summary

**sprite-utils** is a C++23 CLI tool for working with the sprite sheets of
**Speedy Blupi I** (v1.0) and **Speedy Blupi II** (v2.2, a.k.a. Speedy
Eggbert 2) — annotating them, cutting them into individual sprite images,
and building animated GIFs — driven entirely by a semicolon-delimited CSV
("sprite-sheet CSV") that describes every sprite's rectangle and its
`Group`/`Number in Group` (its animation identity).

The project was started ~2 years ago (initial commit, README, hand-drawn
CSVs with real geometry but only ~13% of rows carrying a real `Group`
name) and left unfinished. This session (2026-07-11, one long continuous
session) took it from "unfinished tool + mostly-unlabeled CSVs" to
"working tool + 62%-labeled CSVs + generated deliverables", using
`free-eggbert` (a decompilation of v2.2) and direct binary analysis of
`BLUPI.EXE` (no decompilation exists for v1.0) as the data source.

`plan.md` is the living execution plan (phases, checkboxes, detailed
per-file "Milestone" writeups with evidence) — read it for full technical
detail on any specific file/table. `analysis.md` is the original
investigation this session started from. This file (`NEXT.md`) is the
higher-level handoff: what's done, why, and what a future session should
do next.

## 2. Current state

- **Phase 0 (analysis):** done — see `analysis.md`.
- **Phase 1 (finish the tool):** done. Commands: `draw`, `extract`,
  `restore`, `gifs`, `pack`, `help`, `version`. Build: CMake + OpenCV
  (`core`/`imgproc`/`imgcodecs` only). 34 tests / 99 checks, all passing.
- **Phase 2 (populate the CSVs with real `Group` data):** done — see §3
  below and §4 for why it's 62%, not 100%.
- **Phase 3 (generate the deliverables):** done. `draw`/`extract`/`gifs`
  ran against every file in both games; output lives in `tmp/`
  (gitignored — derived from copyrighted game assets, not redistributed).
- **Phase 4 (future/optional):** not started — see §5.

Both CSVs (`spritesheets/speedy_blupi_I.spritesheet.csv`,
`speedy_blupi_II.spritesheet.csv`) are fully committed and up to date.
Every `tools/generate_*.py` script that produced their `Group` data is
committed and reproducible (re-running them regenerates the same output
from either `free-eggbert`'s source or the small extracted JSON data
files under `tools/data/`, without needing the copyrighted `.blp`/`.exe`
game files present — those are never committed, see `analysis.md` §8).

## 3. What was done this session, in order

### 3.1 Tool completion (Phase 1)
- Implemented `extract` (cut every sprite rectangle into its own PNG,
  `--out-dir/<file-stem>/`) and `restore` (undo `draw`'s `.backup` files).
- Added `--scale` to `draw`/`extract` (multiply X/Y/W/H by a factor, for
  running a 1x-authored CSV against a 2x/4x/8x re-rendered source image).
- Cleaned up `SpriteSheet`/`SpriteUtilsOptions` (removed dead
  static/shadowed state, added a `getGroup()`/`getGifsOutputDirectory()`/
  etc. options layer).
- Fixed the build environment (OpenCV wasn't installed; switched
  `find_package(OpenCV REQUIRED)` → `COMPONENTS core imgproc imgcodecs`
  and specific headers instead of the catch-all `opencv2/opencv.hpp` —
  a real improvement, not just a local workaround).
- Grew the test suite from ~0 to 34 tests / 99 checks
  (`tests/*.cpp`, dependency-free hand-rolled harness in
  `tests/TestFramework.h`).

### 3.2 `gifs` command (pulled forward from Phase 3, by request)
- Hand-rolled a GIF89a encoder from scratch (`GifWriter.h/.cpp`) — no
  animated-GIF write API exists in this project's OpenCV 4.10 packaging.
  Palette quantization (exact for ≤256 colors, median-cut fallback
  above), GIF-flavored variable-width LZW. Two real encoding bugs found
  by actually decoding output with Pillow (not just checking GIF
  structural markers, which both bugs passed): palette written in BGR
  instead of RGB, and LZW code-width grown one code too early.
- `GifsCommand`: groups rows by `(File, Group)`, sorts by
  `Number in Group` (`std::stable_sort` — `std::sort` was briefly a bug,
  since every `Group="?"` row shares the same placeholder sort key),
  crops, removes background, writes one GIF per group.
- Background-key removal went through **4 real iterations** before it
  was right — see `plan.md`'s "`gifs`, pulled forward" section for the
  full blow-by-blow. Final state: border flood-fill (safe for icons that
  fill their own bounding box) **plus** a position-independent sweep for
  the engine's own declared transparency color (`RGB(0,0,255)`, found by
  grepping every `SetTransparent(CHxxx, RGB(0,0,255))` call in
  `free-eggbert/src/pixmap.cpp`) — the flood-fill alone left ~75% of
  crops with visible leftover blue not connected to the crop's edge.

### 3.3 Data mining (Phase 2) — file by file

**Speedy Blupi II (v2.2)**, all sourced directly from `free-eggbert`'s
already-decompiled/committed C source (`pixtables.hpp`, `dectables.cpp`):
- `blupi000.blp`: `table_blupi` (action→icon-sequence) × `table_icon_blupi`
  (icon→rectangle). 100% of rows labeled.
- `object.blp`/`element.blp`/`explo.blp`: ~130 named animation tables
  (`table_bulldozer_left`, `table_explo1`, ...) traced call-site by
  call-site to their real channel (`CHOBJECT`/`CHELEMENT`/`CHEXPLO`) —
  `CHANNEL_MAP` in `tools/generate_object_element_explo_v2_rows.py`.
- `text.blp`: not a rectangle table at all — a plain 16×16 uniform grid,
  addressed via `table_char[]` (character → grid cell). 98% labeled.
- `button00.blp`: same grid mechanism, but no table gives per-cell
  meaning (`src/button.cpp` shows cells are generic reused button
  *states*, not fixed named icons) — geometry done, `Group` left `?`
  deliberately.
- `jauge.blp`: 1×4 grid, 3/4 rows named by exhaustively grepping every
  `CJauge::SetType`/`Create` call site in the whole decompiled codebase.
- `decor0NN.blp`: investigated and closed as **not applicable** — it's a
  full-screen scrolling background image (`CHDECOR` has `iconDim=(0,0)`),
  not a sprite grid; the real per-tile world data already lives in
  `object.blp`.

**Speedy Blupi I (v1.0)** — no decompiled source exists, so every table
below was found directly inside `BLUPI.EXE` (535,552 bytes) by binary
analysis:
- `table_icon_blupi/object/element/explo` (fixed-record geometry
  tables): found by a byte-signature scan scoring every 4-byte-aligned
  offset by how many consecutive plausible records follow. Committed as
  `tools/data/v1_icon_tables.json` (extracted via
  `tools/extract_v1_icon_tables.py`).
- `table_blupi` (variable-record action→icon table): no known offset:
  built a scoring scanner (chain length by valid-record count) over
  every aligned position; one overwhelming winner (`0x2d5a0`, 67
  records) vs. 552 much-weaker candidates. Cross-validated: the
  action-id visiting *order* matches v2.2's table almost exactly.
  `blupi000.blp`: 81% labeled, all 37 pre-existing hand-verified labels
  (2 years old) preserved exactly — matched by exact-pixel-rectangle
  cross-reference against the new table, all 37 landed at 0px error.
- `text.blp`: same grid formula as v2.2 (byte-identical mechanism, no
  scan needed). `jauge.blp`: same, 3/4 rows.
- `object.blp`/`element.blp`/`explo.blp` named tables: 44/88 (explo: 3;
  object+element: 41) found by **exact byte-pattern search** (searching
  for each v2.2 table's known value sequence, as raw little-endian
  `int32` bytes, directly in the EXE) rather than the general scoring
  scan (too noisy for these — short tables, narrow value ranges). Many
  formed large, gapless, correctly-source-ordered contiguous runs in the
  binary (one unbroken 31-table chain for `object`/`element`) — the
  strongest possible confirmation a byte match can have. Two real false
  positives were caught and excluded along the way (see `plan.md`'s
  `object.blp`/`element.blp` v1.0 milestone) — worth reading before
  trusting/extending this technique further.
- Existing rows were cross-referenced to icon indices by exact
  `(X,Y,Width,Height)` match first, then a min-cost (Hungarian
  algorithm) assignment for the remainder against the leftover
  unclaimed icons — used for `explo.blp` (8/54 rows) and
  `object.blp`/`element.blp` (2/246, 16/188 rows). Geometry was **never**
  touched in this process, only `Group`/`Number in Group` added.

### 3.4 Phase 3 — deliverables
`draw`/`extract`/`gifs` ran against every file in both games using the
final CSVs. Output: `tmp/speedy_blupi_{I,II}/{draw,extract,gifs}/`
(gitignored). 479 GIFs (218 + 261), 3003 extracted sprite images, fully
annotated sheets for both games.

### 3.5 `pack` command (added after Phase 3, same session)
New command, the inverse of `extract`: reads a directory of individual
sprite images (named exactly the way `extract` writes them -
`<numberPerSheet>__<group>__<numberInGroup>.png` under
`<dir>/<file-stem>/`) and pastes each one back at its CSV rectangle to
rebuild one full sheet image, canvas sized from the CSV alone. Built for
the workflow already noted in `plan.md`'s `--scale` section: re-rendering
sprites individually from the original 3D models (rather than the whole
sheet at once) and reassembling them before `draw`/`gifs` are useful
again. Alpha-aware (transparent-background canvas if any input sprite has
real alpha); tolerant of missing/mis-sized individual images (warns,
doesn't fail the whole sheet). Verified with an `extract`→`pack` round
trip that reproduces the original pixel-for-pixel, plus synthetic
`--scale` and alpha-channel tests. See `plan.md`'s "`pack`" section for
full detail. This also confirmed PNG reading/writing already works in
`extract`/`gifs`/`pack` (OpenCV sniffs file content, not extension) - only
`draw` still can't handle a PNG source (`readBmpBpp` unconditionally
requires BMP magic bytes), documented as a real, specific gap in
`plan.md`'s PNG support section rather than left vague.

## 4. Why 62% (1870/3003), not 100%

This is not unfinished work — every file in both CSVs was investigated
as far as the *available evidence* supports. The remaining ~38% breaks
down into three genuinely different reasons:

1. **No source table exists at all** (`button00.blp` in both games,
   `jauge.blp` row 0). The grid *geometry* is 100% correct and complete;
   there is simply no data anywhere — decompiled or in the binary —
   that assigns a name to each cell. `Group="?"` here is not a gap to
   close, it's an honest "unknown", matching this session's rule to
   never guess a semantic label without evidence.
2. **v1.0's icon tables are genuinely smaller than v2.2's** (e.g.
   `table_icon_element` has 187 entries in v1.0 vs. 289 in v2.2).
   Most of the ~35 `object`/`element` named tables that weren't found in
   `BLUPI.EXE` reference icon indices that don't exist in the smaller
   v1.0 table at all — content the sequel (v2.2) added later. These
   can't be found because they aren't there, not because the search was
   incomplete.
3. **Byte-signature search has a real noise floor.** For `explo.blp`
   specifically, only 3/12 named tables were locatable with genuine
   confidence — short tables (5-20 values) drawn from a narrow range
   (0-53) produce too many coincidental matches for a pure "values in
   range" scan, and only 3 tables happened to be both long enough and
   distinctively-valued enough for an *exact* byte match to be
   trustworthy. `object`/`element` did much better (41/76) purely
   because more of their tables happened to sit in large contiguous,
   correctly-ordered chains in the binary, which is a much stronger
   signal than an isolated match — but that's a property of how the
   original compiler laid out the data, not something a smarter search
   can force. This ceiling is a fundamental limit of reverse-engineering
   a binary with no debug symbols, not a shortcut taken this session.

In short: **100% is not achievable from the evidence that exists.** What
would move the needle is either finding a genuine v1.0 decompilation (a
Ghidra project already exists per `analysis.md` — deeper manual
decompilation work, not just byte-scanning, could recover more), or
accepting that some fraction of v1.0's UI/decor sheets simply never had
named tables to begin with, same as v2.2.

## 5. What's left (Phase 4 — not scheduled, all optional)

- **Other binaries/variants** beyond v1.0/v2.2 English: demos, `SE`/
  `_EGAMES` variants, localizations. The byte-signature-scan method
  (§3.3) should generalize — each variant needs its own offset-relocation
  pass (`analysis.md` §8 has pointers to where these files live on the
  library drive).
- **Push `object`/`element.blp` (v1.0) further, if someone wants to
  invest in a real Ghidra-based decompilation** rather than byte
  scanning — the ~35 unfound tables (§4, point 3) reference icons that
  may genuinely not exist in v1.0, so decompilation would first need to
  establish whether v1.0 has *smaller/different* versions of those
  tables at all, not just find offsets for the same ones.
- **Decide long-term deliverable storage.** Right now Phase 3 output
  lives in this repo's gitignored `tmp/`, regenerable on demand. If the
  annotated sheets/GIFs/extracted sprites need to be *shared* (not just
  regenerated locally), that needs a real distribution plan — a
  dedicated assets repo, a release artifact, a hosted gallery, etc.
  (not decided; revisit once there's an actual consumer for the output).
- **This `NEXT.md`/`CLAUDE.md` convention itself** — this file now
  exists; a `CLAUDE.md` (project-specific working conventions, distinct
  from this narrative handoff) does not yet, unlike several sibling
  repos in the `openeggbert` org. Worth adding if this project keeps
  getting worked on across sessions.

## 6. Working conventions (carry forward)

- Never commit copyrighted `.blp`/`.exe` game assets. Small *extracted*
  integer/geometry data (byte offsets, table contents) is fine to commit
  under `tools/data/` — matches `free-eggbert`'s own convention of
  committing `pixtables.hpp`/`dectables.cpp`.
- Every `Group` label must be traceable to real evidence (a decompiled
  table, a call-site trace, a byte-signature match with contiguity/
  uniqueness confirmation) cited in `Notes` and in `plan.md`'s
  per-file milestone. Never guess a semantic name to fill a gap —
  `Group="?"` is the correct, honest output when evidence runs out.
- Never touch already-correct geometry. Every v1.0 file this session
  found with pre-existing hand-measured rows (`blupi000.blp`'s 37,
  `object`/`element`/`explo.blp`'s bulk) had that geometry cross-checked
  by exact-rectangle match before any `Group` label was added on top —
  X/Y/Width/Height were never overwritten by an auto-generation pass.
- Verify visually before trusting generated data: `sprite_utils draw`
  (does every rectangle land on real content?) and, for animations,
  actually rendering the GIF (does the sequence make sense?) — used
  before every single replacement this session, not just at the end.
- Commit messages are detailed and evidence-citing (file:line, byte
  offsets, match counts) — read `git log` on this repo for that style
  if writing a future commit here.

## 7. Resume prompt (copy/paste for a future Claude Code session)

```text
Read NEXT.md and plan.md in the repo root first. sprite-utils is a
finished tool (Phases 0-3 done) with 62% of the two sprite-sheet CSVs
labeled — that ceiling is explained in NEXT.md §4 and is not a shortcut,
don't try to silently "finish" it by guessing labels. If asked to extend
coverage further, treat it as new investigative work (see NEXT.md §5),
not a bug to fix. Any new Group label must cite real evidence (a
decompiled table, a traced call site, or a byte-signature match with
contiguity/uniqueness confirmation) in both the CSV's Notes column and
plan.md - follow the same rigor as the object.blp/element.blp v1.0
milestone (which caught and excluded two false-positive byte matches
before trusting them). Never touch existing geometry (X/Y/Width/Height)
without cross-checking it first. Verify any generated data with
`sprite_utils draw` (and, for animations, an actual rendered GIF) before
trusting it. Never commit copyrighted .blp/.exe game files - only small
extracted integer/geometry data under tools/data/. When done, update
plan.md's status line and this file's §2/§3, then stop.
```
