# sprite-utils Plan

**Status as of 2026-07-11:** Phase 0 done (analysis). Phase 1 done and
verified (build + unit test suite + a real-data smoke test all pass).
`gifs` (normally Phase 3) was pulled forward and implemented early, by
request, to have a visible/fun result before Phase 2's data work — see the
"gifs, pulled forward" note under Phase 1 below. **Phase 2 well underway
for Speedy Blupi II (v2.2):** all of `blupi000.blp`, `object.blp`,
`element.blp` and `explo.blp` now have `Group`s auto-generated from
`free-eggbert`'s own tables — **711/1321 rows (54%) in
`speedy_blupi_II.spritesheet.csv` are labeled, up from ~22% at the start of
this session.** See the two "Milestone" writeups under Phase 2. Still open:
all of Speedy Blupi I (v1.0), decor tiles (both games), and
`button00.blp`/`jauge.blp` (small UI sheets, not yet attempted — no known
source table for them).

This is the living execution plan for finishing `sprite-utils` and using it to
produce complete, correctly-annotated sprite-sheet data for both **Speedy
Blupi I** and **Speedy Blupi II**. It picks up directly from `analysis.md`
(read that first for the full investigation and evidence — this file is the
*plan*, not the analysis) and follows the same file/line-referenced,
evidence-based style. Update the checkboxes below as work lands; this file
should always reflect current reality, not history (keep history in commit
messages / `analysis.md`).

## Goal

Two fully-populated sprite-sheet CSVs — `speedy_blupi_I.spritesheet.csv` and
`speedy_blupi_II.spritesheet.csv` — with correct `X`/`Y`/`Width`/`Height` and
meaningful `Group`/`Number in Group` for **every** sprite sheet file in both
games (character variants, object/element/explo/UI sheets, and the decor
tile sheets), not just the small hand-measured subset that exists today.
Then use a finished `sprite-utils` to turn that data into:

- annotated sheets (dashed rectangle + sprite number over every sprite, via
  `draw` — implemented),
- individual per-sprite image files (via `extract` — implemented),
- animated GIFs per animation group (via `gifs` — implemented; see the
  "gifs, pulled forward" note below for why it landed before Phase 2/3).

## Phases

Four phases, meant to run mostly in order — each phase's output is a
prerequisite for making the next phase's output *correct*, even though some
prep work can overlap.

---

### Phase 1 — Finish `sprite-utils` as a tool

Reference: `analysis.md` §2 and §5 "Track A". Small, mechanical, low-risk.
This phase does not depend on Phase 2's data — it can and should be done
first/in parallel, using the two CSVs already in this repo
(`spritesheets/*.csv`) as real test input.

- [x] Implement `extract` — crop `[X, Y, X+Width, Y+Height]` out of the
      source image for every CSV row and write it to its own file, under
      `<out-dir>/<source-file-stem>/<numberPerFile>__<group>__<numberInGroup>.png`
      (`--out-dir`, default `<dir>/extracted`). `include/ExtractCommand.h` /
      `src/ExtractCommand.cpp`, wired into `src/SpriteUtils.cpp`.
- [x] Implement `restore` — copies every `<file>.backup` back over `<file>`
      in the working directory (optionally filtered by `--file-name`).
      `include/RestoreCommand.h` / `src/RestoreCommand.cpp`.
- [x] Fix `HelpCommand.cpp` — replaced the stale option names with the real
      `--`-prefixed options per command, and documented `extract`/`restore`.
- [x] Added a test suite (`tests/`, custom header-only harness in
      `tests/TestFramework.h`, no external dependency): 28 test cases / 71
      checks covering `Utils::split` edge cases, `SpriteSheetRow` parsing
      (minimum columns, optional `X`, the height-negation-for-column>1
      encoding), `SpriteSheetArgs` CLI parsing, and `SpriteSheet`'s
      documented CSV semantics (auto-X, height inheritance/delta, `skipskip`,
      first-row and sequential row/column validation) — plus a regression
      test that two `SpriteSheet` instances processing same-named files
      don't leak state into each other. Wired into CTest
      (`add_test(NAME sprite_utils_tests ...)`).
- [x] Removed `SpriteSheet`'s dead static state. On inspection,
      `lastX`/`lastWidth`/`lastHeight` (`static int` class members) were
      write-only — nothing ever read them — so this was a pure dead-code
      removal, not a behavior change. There was also a second,
      **shadowed** `static std::optional<SpriteSheetRow> lastSpriteSheetRow`
      at file scope in `SpriteSheet.cpp`, fully hidden by the (already
      correctly instance-scoped) member of the same name declared in
      `SpriteSheet.h` — also dead, also removed. `SpriteSheet` was already
      reentrant via that instance member; it's just cleaner now with the
      dead statics gone. Locked in by the leak-regression test above.
- [x] Wrapped `main()` in a top-level try/catch — a `SpriteUtilsException`
      now prints `Error: <message>` and exits 1 instead of crashing with an
      unhandled-exception message.
- [x] Build + smoke-test: `cmake --build` succeeds (0 warnings surfaced),
      `ctest` / the 71-check suite passes, and a real-data smoke test
      (`draw`, `extract`, `restore` run against `gamefiles/IMAGE08/jauge.blp`
      + the real `speedy_blupi_II.spritesheet.csv` rows for that file) was
      visually confirmed: `draw` produced 4 correctly-numbered dashed
      rectangles, `extract` produced 4 correctly-cropped PNGs, `restore`
      byte-for-byte restored the original file (`md5sum` match).
- [x] Bonus (needed to get a clean build, and a genuine improvement either
      way): `find_package(OpenCV REQUIRED)` now requests
      `COMPONENTS core imgproc imgcodecs` explicitly — those are the only
      modules the code actually uses (`Mat`/`Rect`/`Scalar`,
      `rectangle`/`line`/`cvtColor`, `imread`/`imwrite`) — and
      `DrawCommand.h`/`ExtractCommand.cpp` now include the specific
      `opencv2/core.hpp`/`imgproc.hpp`/`imgcodecs.hpp` headers instead of
      the catch-all `opencv2/opencv.hpp`, which used to pull in every
      OpenCV module (calib3d, dnn, viz, ...) whether used or not.

### `gifs`, pulled forward

`gifs` was originally scoped for Phase 3 (it only produces something
*broadly* useful once `Group` data exists for most rows, which is Phase 2's
job) — but implemented now, by request, to get a real, visible result before
the data work. It works fine today against whatever groups already exist
(e.g. `Yellow_Eggbert_Swimming_Right` in `speedy_blupi_I.spritesheet.csv`);
running it with no `--group` filter on the full CSV will also lump every
still-`Group=?` row per file into one (large, not very meaningful) GIF,
which is expected and will stop happening naturally once Phase 2 fills in
real groups.

- [x] `GifWriter` (`include/GifWriter.h` / `src/GifWriter.cpp`): a
      self-contained animated GIF89a encoder (palette quantization — exact
      for <=256 colors, median-cut fallback above that — plus the
      GIF-flavoured variable-width LZW compressor), written by hand because
      this project's OpenCV 4.10 packaging has no animated GIF/WebP write
      API (`cv::Animation`/`imwriteanimation` isn't present in every OpenCV
      4.x build).
  - Two real encoding bugs were found and fixed by actually decoding
    generated files with Python/Pillow (not just checking GIF structural
    markers, which both bugs passed): (1) the palette was written to the
    GIF in BGR order instead of RGB (red/blue swapped); (2) the LZW code
    width grew one code too early, which never showed up on trivial
    test data but corrupted real, longer sprite frames ("broken data
    stream" on decode). Both are covered by `tests/GifWriterTests.cpp`
    structurally; the pixel-level fix was verified out-of-band (Pillow
    decode + a visual contact sheet of a real 9-frame swimming animation,
    plus numeric spot-checks against a synthetic gradient for the
    median-cut path) rather than re-implementing a GIF decoder just for
    the test suite.
- [x] `GifsCommand` (`include/GifsCommand.h` / `src/GifsCommand.cpp`,
      registered as `gifs`): groups sprite-sheet rows by `(File, Group)`,
      sorts each group by `Number in Group`, crops each row's rectangle,
      and writes one GIF per group to
      `--out-dir/<source-file-stem>/<group>.gif` (default
      `--out-dir={--dir}/gifs`). New options: `--group` (build just one
      named group), `--frame-delay-ms` (default 100), `--gif-background-color`
      (default `255,255,255`, used to pad frames up to the animation's
      largest frame size).
- [x] Documented in `HelpCommand.cpp`; `Utils::sanitizeForFilename` was
      factored out of `ExtractCommand` so `gifs` doesn't duplicate it.
- [x] Verified end-to-end against real game data: ran `gifs --group
      Yellow_Eggbert_Swimming_Right` against Speedy Blupi I's real
      `blupi000.blp` (extracted locally from `Speedy_Blupi_I.7z` for this
      check, not committed — see `analysis.md` §8 on not redistributing
      original assets) and the CSV already in this repo; the resulting
      9-frame GIF decodes correctly and shows a genuine swimming animation.
- [x] `gifs`: the sprite sheets are drawn on a color-key background — swap
      it for `--gif-background-color` (default white) before quantizing
      each frame, instead of leaving the game's blue transparency key
      visible as a box around every sprite. Went through two revisions
      after generating GIFs for *every* file in both games (see "Full
      corpus generation" below) surfaced cases the first pass missed:
      1. First pass sampled one color-key from each *sheet's* top-left
         corner. Broke on tightly-packed sheets (`object.blp`) where the
         corner pixel is real sprite content, not background.
      2. Second pass switched to the *most frequent color within each
         cropped frame*. Fixed the corner problem, but still wrong for
         icons that nearly fill their own bounding box (trees, spheres) —
         the icon's own color can outweigh the sliver of true background,
         so the "background" guess was sometimes actually part of the
         icon.
      3. Final approach: flood-fill in from 8 points around each frame's
         own border (corners + edge midpoints, small per-step color
         tolerance so a soft gradient still connects), replacing only
         background pixels *reachable from the edge*. An icon that fills
         its whole cell has no reachable border strip of background, so
         it's correctly left alone instead of risking erosion into real
         sprite content — confirmed by re-inspecting the same previously-
         broken `object.blp` icons: most are now clean, and the handful
         that still show their original color turn out to have no
         separate background margin at all within their bounding box
         (verified by inspecting the source pixels directly), so leaving
         them untouched is the correct, safe behavior rather than a bug.
- [x] Fixed `gifs` sorting each group by `Number in Group` with `std::sort`
      instead of `std::stable_sort`. Every still-unlabeled (`Group=?`) row
      shares the same placeholder `Number in Group` value, so for that
      bucket the sort key is constant and `std::sort` does not guarantee
      which order equal-key rows come out in — found because the "?"
      catch-all GIFs were visibly not in the sheet's actual reading order.
      `std::stable_sort` preserves the CSV's original row order as the
      fallback for ties, which is what the "?" bucket needs.

#### Full corpus generation (both games, every file, no filters)

Ran `gifs` with no `--group`/`--file-name` filter against every sheet
referenced by both CSVs (`blupi000.blp`, `button00.blp`, `element.blp`,
`explo.blp`, `jauge.blp`, `object.blp`, and — Speedy Blupi I only —
`text.blp`; source images pulled locally from `Speedy_Blupi_I.7z` and
`free-eggbert/gamefiles/IMAGE08`, neither committed). Result: **17 GIFs for
Speedy Blupi I, 294 for Speedy Blupi II** (311 total, ~3.4 MB), written to
`gifs_output/` in this repo (gitignored — see `.gitignore`). Most of these
are single-frame or lumped into one large `?`-group GIF per file, since
today's CSVs are still mostly unlabeled — expected, and exactly the gap
Phase 2 closes. Ran end-to-end without errors or crashes across the whole
corpus, including a 421-frame GIF, which is a reasonable stress test of
`GifWriter` beyond the smaller examples used during development.

### Higher-resolution source images (`--scale`)

Robert is going to re-render these sprites directly from the original 3D
models in Ray Dream Studio at 2x/4x/8x resolution. The existing CSVs stay
authored at 1x — `sprite-utils` needs to be able to read a higher-resolution
source image with the *same* 1x CSV and land on the right rectangles.

- [x] `SpriteSheet` takes an optional integer `scale` (`--scale`, default 1,
      any positive integer — not restricted to 2/4/8). All of the *existing*
      auto-X / height-inheritance / row-column-validation arithmetic keeps
      running entirely in the CSV's original 1x space, untouched — `scale`
      is applied as a pure read-side transform, multiplying `X`/`Y`/`Width`/
      `Height` by it only in `getSpriteSheetRows(...)`, on a copy. This
      matters: multiplying each row in place *during* parsing, immediately
      after it's resolved, looks equivalent but isn't — the next row's
      auto-X/height computation reads the previous row back out of internal
      state, and scaling that row before it's read compounds (e.g. row 3
      would come out scaled twice). Covered by
      `spriteSheet_scaleMultipliesResolvedCoordinatesOnly` in
      `tests/SpriteSheetTests.cpp`, which specifically checks a 3-row
      auto-computed chain (the same fixture as the Phase 1 auto-X test)
      stays internally consistent under `scale=2` rather than drifting.
- [x] Wired `--scale` through `SpriteUtilsOptions::getScale()` into `draw`,
      `extract`, and `gifs` (all three just read
      `SpriteSheet(path, opt.getScale())`).
- [x] Documented in `HelpCommand.cpp`.
- [x] Verified end-to-end: took the real Speedy Blupi I `blupi000.blp`,
      upscaled it 2x with nearest-neighbor as a stand-in for a real Ray
      Dream Studio re-render, and ran `gifs --scale 2` against it with the
      *unmodified* 1x CSV — output canvas came out exactly 114x64 (2x the
      1x version's 57x32) and the animation is pixel-correct, just bigger.
- [ ] Deliberately unscaled for now: `draw`'s dashed-rectangle line weight
      and digit font size stay fixed-pixel regardless of `--scale` — at 8x
      they'll look like a hairline next to a giant sprite. Not asked for;
      flagged here so it isn't mistaken for an oversight if it comes up
      later (leaving this one unchecked on purpose — it's a known
      limitation, not a completed task).

### PNG support (analysis only — not implemented yet)

The game engine that currently reads these sprites out of `.blp` (which is
just BMP under a different extension) is being updated separately to also
support PNG. `sprite-utils` should eventually be able to read/write PNG
sprite sheets too, not just BMP/`.blp`.

- [ ] **Analysis task**: work out what actually has to change — where
      `DrawCommand`/`ExtractCommand`/`GifsCommand` currently assume BMP
      specifically (the bit-depth sniffing in `DrawCommand::readBmpBpp`,
      the custom `writeBmp16BGR565` RGB565 writer used for `.blp`/16-bit
      BMP output, the always-BLP-is-RGB565 branch in `DrawCommand::run`),
      versus what's already format-agnostic through OpenCV
      (`cv::imread`/`cv::imwrite` already handle PNG transparently, so
      *reading* a PNG sheet may already work today for `draw`'s non-BLP
      path — needs verifying, not assuming). Cover what "write PNG output
      instead of overwriting the source in place" should mean for `draw`'s
      backup/overwrite model, and whether PNG's real alpha channel should
      replace the color-key-background approach `gifs` just grew. Write
      the findings up (in `analysis.md` or a new `docs/png-support.md`)
      before writing any PNG-handling code.

---

### Phase 2 — Auto-generate complete CSVs for both games

Reference: `analysis.md` §3, §4, §5 "Track B", §4.6. This is the actual data
work — turning tables that already exist inside `free-eggbert` (for Speedy
Blupi II / v2.2) and tables located directly inside `BLUPI.EXE` v1.0 in this
session (for Speedy Blupi I, see `analysis.md` §4.6) into complete CSV rows,
without hand-measuring pixels.

**Speedy Blupi II (v2.2) — pixel rectangles:**
- [x] `blupi000.blp`: `table_icon_blupi` dumped and used — see the
      "Milestone: blupi000.blp done" writeup below.
- [x] `object.blp`/`element.blp`/`explo.blp`: `table_icon_object` (441)/
      `table_icon_element` (289)/`table_icon_explo` (100) dumped and used —
      see "Milestone: object/element/explo.blp done" below.
- [ ] `blupi001-003.blp` share `table_icon_blupi` with `blupi000.blp` (same
      geometry, recolored) but aren't referenced by any CSV row yet — low
      priority until something actually needs them.

**Speedy Blupi II (v2.2) — group names:**
- [x] `blupi000.blp`: `table_blupi` dumped and used to label 231/336 icons
      via their `ACTION_*` name — see "Milestone: blupi000.blp done" below.
- [x] `object.blp`/`element.blp`/`explo.blp`: 101 of the ~150 named tables
      (`table_bulldozer_left`, `table_shield`, `table_explo1`, etc.) traced
      to a confirmed channel and used to label 480 icons — see "Milestone:
      object/element/explo.blp done" below.

#### Milestone: `blupi000.blp` (Speedy Blupi II) done

`tools/generate_blupi000_v2_rows.py` (committed, reproducible — re-running
it against the current `free-eggbert` checkout reproduces the exact CSV
rows byte-for-byte) parses `table_icon_blupi` (336 `IconPack` records) and
`table_blupi` (a flat `[action_id, frame_count, phase, icon...]` record
stream — see below) directly out of `free-eggbert`'s committed C source,
and cross-references `ACTION_*` names from `include/def.hpp`.

Parsing `table_blupi` needed one correction beyond `analysis.md` §4.3's
original description: the record-advance algorithm
(`i += table_blupi[i+1] + 3`) is right, but the array also contains one
~30-int block of `-1` padding between two records that isn't itself a
record — naively parsing straight through desyncs the rest of the array.
Fix: skip a lone int at a time whenever the current position isn't a valid
`ACTION_*` id, which resyncs automatically. Validated by parsing all the
way to the array's *true* final `0` terminator with nothing left over
(2842 ints fully accounted for) — the parser now asserts this instead of
silently trusting a partial parse.

A second finding changed how the data gets used, not just parsed: many
`table_blupi` records are far too large to be literal frame-by-frame
animations (`ACTION_STOP` alone has 330 entries, heavily repeating a
handful of idle-pose icons — clearly a combinatorial state lookup, not a
330-frame animation). Records with `frame_count >= 40` are still labeled
with their `ACTION_*` group (that claim is solid — the icon genuinely
belongs to that action), but get an explicit "likely a state lookup, not a
plain animation sequence" note rather than being presented as a clean
animation. An icon referenced by more than one action keeps its *first*
table-order action as the primary `Group` and lists the others in `Notes`
(the CSV format has no way to give one physical rectangle two groups at
once).

Merge decision: **replaced, not merged.** All 336 existing `blupi000.blp`
rows in `speedy_blupi_II.spritesheet.csv` had `Group="?"` (zero hand-labels
to lose) *and* used a completely different, uncorrelated grid-scan geometry
— not just unlabeled but measuring the wrong thing. Verified with
`sprite_utils draw` before replacing: every one of the 336 generated
rectangles lands exactly on its sprite, confirmed visually across the
entire 800×906 sheet.

Result: **231/336 icons (69%) now have a real `Group`**, up from 0. Running
`gifs` on `blupi000.blp` now produces dozens of genuine named animations
(`ACTION_MARCH` — walking, `ACTION_STOPSKATE` — skateboard balancing,
`ACTION_TURNTANK`, ...) instead of one 336-frame `?` dump. Spot-checked
several visually; they're real, coherent animations, not noise.

#### Milestone: `object.blp`/`element.blp`/`explo.blp` (Speedy Blupi II) done

Same pixel-rectangle approach as `blupi000.blp`
(`tools/generate_object_element_explo_v2_rows.py`, also committed and
verified byte-for-byte reproducible), but grouping was a materially harder
problem: unlike `table_blupi` (one table, one consumer, explicit
`ACTION_*` ids baked in), `object.blp`/`element.blp`/`explo.blp` share
~150 further named tables in `free-eggbert/src/dectables.cpp`
(`table_bulldozer_left`, `table_poisson_right`, `table_explo1`, ...) with
**no built-in channel information** — nothing in the table declarations
says whether a given table's icon indices belong to `table_icon_object`,
`table_icon_element` or `table_icon_explo`.

Resolved with a dedicated read-only investigation (41 tool calls) tracing
every table to its actual call site — either the `channel` argument of a
`QuickIcon`/`DrawIcon` call, or an explicit `.channel = CHxxx` assignment
in `CDecor::MoveObjectStepIcon` (`decmove.cpp`) — across `decor.cpp`,
`decblock.cpp`, `decmove.cpp`, `decdesign.cpp`, `decblupi.cpp`. Confirmed
101 of ~130 real tables this way: 25 → `CHOBJECT`, 43 → `CHELEMENT`, 12 →
`CHEXPLO`, plus 8 more (`table_blupih_*`/`table_blupit_*`) pattern-matched
to `CHELEMENT` from an identical-but-incomplete code shape (included, with
an explicit "not directly confirmed" note on affected rows, rather than
silently asserted). The remaining ~29 were positively excluded, not
skipped by omission:
- **7 turned out not to be icon tables at all** despite `table_`-prefixed,
  sprite-adjacent names — `table_decor_action` is camera-scroll deltas,
  `table_tutorial` is help-text trigger regions, `table_blitz` is FX
  timing, `table_vitesse_march/nage/surf` are movement speed tables,
  `table_drinkoffset` is a time-offset list. A useful reminder that
  `table_decor_*` naming is not a reliable signal by itself — most
  `table_decor_*` tables *did* turn out to be `CHOBJECT` icon tables (used
  for world-hazard sprites like lava, saws, fans — confirmed via the same
  `QuickIcon(1, ...)` call-site pattern), just not because of the "decor"
  in their name.
- **1 confirmed unused** (`table_invertpanel` — defined, declared, no call
  site anywhere in the codebase).
- **1 mixed-channel** (`table_electro`: `CHBLUPI2` below phase 30,
  `CHELEMENT` at/above it) and **2 genuinely unresolved**
  (`table_chenille`/`table_chenillei` — icon assignment confirmed, channel
  not found in any traced file) were left out of the `Group` labeling
  entirely rather than guessed.

Merge decision: same as `blupi000.blp` — replaced outright.
`speedy_blupi_II.spritesheet.csv`'s existing `object.blp`/`element.blp`/
`explo.blp` rows had **zero real `Group` labels** (either literal `?`, or
— for `element.blp` specifically — 289 individual numeric strings used as
disposable per-row placeholders, not real grouping) and — for `object.blp`
— even the wrong *row count* (421 vs. the authoritative table's 441), so
there was no hand-curated signal to preserve. Verified with `sprite_utils
draw` against all three real images before replacing: every rectangle on
all three sheets (1024×1327, 896×644, 496×1885) lands correctly.

Result: **480 more icons labeled** (192/441 object, 230/289 element,
58/100 explo). Combined with `blupi000.blp`, **711/1321 rows (54%) of
`speedy_blupi_II.spritesheet.csv` are now labeled, up from ~22% at the
start of this session.** `gifs` with no filter now produces 133 GIFs for
this file set (down from 352 with the pre-consolidation numeric
placeholder groups, but the right kind of "down" — `element.blp` alone
used to yield ~289 disposable 1-frame "groups"; it now yields a smaller
number of real multi-frame animations like `table_bulldozer_left`,
`table_poisson_right`, `table_creature_turn2`). Spot-checked several
visually (`table_bridge`, `table_bulldozer_left`, `table_explo3`) — real,
coherent animations.

**Speedy Blupi I (v1.0) — pixel rectangles (offsets already found & visually
confirmed, see `analysis.md` §4.6):**
- [ ] Reproduce the byte-signature-scan script from `analysis.md` §4.6
      against `Speedy_Blupi_I/BLUPI.EXE` and generate CSV rows for
      `blupi000-003.blp`, `object.blp`, `element.blp`, `explo.blp` using the
      four offsets already recorded there.

**Speedy Blupi I (v1.0) — group names (open problem):**
- [ ] Locate the v1.0 equivalent of `table_blupi` (variable-length
      `[action_id, frame_count, phase, icon...]` records — different shape
      than the fixed 12-byte `IconPack` records, needs its own scan
      approach) and/or the v1.0 equivalents of the named per-entity tables.
      Ghidra Version Tracking (`SB v1.0 to ...` sessions) is a fallback if
      the byte-scan approach doesn't generalize cleanly to this record
      shape.

**Decor tiles (both games, `decor0NN.blp` — currently 0% covered):**
- [ ] Locate the grid constants `CPixmap`/`CHDECOR` uses
      (`free-eggbert/src/pixmap.cpp:1068-1076` and `include/decor.hpp`) —
      tile width/height, tiles-per-row/region layout.
- [ ] Generate uniform-grid CSV rows per `decor0NN.blp` file from those
      constants (no per-tile lookup table needed, unlike the icon sheets).
- [ ] Confirm whether Speedy Blupi I has an equivalent decor tile set and
      grid, or whether its world-tile format differs (not yet checked).

**Merge and finalize:**
- [ ] Merge auto-generated rows with the existing hand-assigned `Group`
      values already in `spritesheets/*.csv` (e.g.
      `Yellow_Eggbert_Swimming_Right`) rather than discarding them — treat
      human labels as higher-priority where both exist, fill `Group="?"`
      rows from the auto-generated data, and cross-reference the raw
      `ACTION_*`/table name in `Notes`/`Tags` either way.
- [ ] Sanity-check: run `sprite-utils draw` (Phase 1) over a sample and
      visually confirm rectangles land correctly, the same way `analysis.md`
      §4.6 did for the v1.0 discovery.
- [ ] Replace `spritesheets/speedy_blupi_I.spritesheet.csv` and
      `spritesheets/speedy_blupi_II.spritesheet.csv` in this repo with the
      completed versions. Commit.

---

### Phase 3 — Generate the actual deliverables

Depends on Phase 1 (tool) + Phase 2 (complete data).

- [ ] Run `sprite-utils draw` over the full asset set for both games →
      annotated sheets with every sprite rectangle + number visible.
- [ ] Run `sprite-utils extract` over both games → one image file per
      sprite, organized by file/group.
- [ ] Run `sprite-utils gifs` (already implemented, see Phase 1's "gifs,
      pulled forward" note) over both games now that Phase 2 has populated
      real groups for (almost) every row → one meaningful animated GIF per
      `Group`, instead of today's handful of hand-labelled groups plus one
      big `?` catch-all per file.
- [ ] Decide where outputs live (this repo vs. a dedicated assets
      repo/output directory — not yet decided, revisit once Phase 2 shows
      the real output volume).

---

### Phase 4 — Future / optional (not scheduled yet)

- [ ] Other binaries/variants beyond v1.0/v2.2 English (demos, `SE`/
      `_EGAMES` variants, localizations) — see `analysis.md` §8 for the
      byte-scan method, which should generalize.
- [ ] Adopt the `CLAUDE.md`/`NEXT.md` knowledge-persistence convention used
      elsewhere in the `openeggbert` org (`analysis.md` §7) so a future
      session doesn't need to re-read this whole plan to get oriented.

---

## Working conventions

- This file tracks **current** status via checkboxes; keep it in sync with
  reality after every meaningful step, not just at the end of a phase.
- Don't redistribute original game `.blp`/`.exe` assets through this repo.
  The two sprite-sheet CSVs (small text data describing rectangles) and any
  small extracted integer tables are fine to commit — see `analysis.md` §8.
- Validate generated data visually before trusting it (render rectangles
  onto the real sheet, look at it) — don't chain automated steps on
  unverified assumptions about table semantics.
