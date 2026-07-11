# sprite-utils Plan

**Status as of 2026-07-11:** Phase 0 done (analysis). Phase 1 done and
verified (build + unit test suite + a real-data smoke test all pass).
`gifs` (normally Phase 3) was pulled forward and implemented early, by
request, to have a visible/fun result before Phase 2's data work — see the
"gifs, pulled forward" note under Phase 1 below. **Phase 2 and Phase 3 are
both done.** Phase 2 summary, for both games:
- Speedy Blupi II (v2.2): `blupi000.blp`, `object.blp`, `element.blp`,
  `explo.blp` and `text.blp` now have `Group`s auto-generated from
  `free-eggbert`'s own tables; `button00.blp` geometry done (`Group="?"`,
  no source table); `jauge.blp` done (3/4 rows labeled) — **1089/1710 rows
  (64%)** labeled, up from ~22%.
- Speedy Blupi I (v1.0, no decompiled source to read - extracted directly
  from `BLUPI.EXE`): `blupi000.blp`, `text.blp` and `jauge.blp` done the
  same way, all 37 pre-existing hand-verified `blupi000.blp` labels
  preserved exactly; `button00.blp` geometry done (`Group="?"`); `explo.blp`
  partially done (12/54 icons via byte-signature scan); `object.blp`/
  `element.blp` also done via the same scan — 41/76 named tables found
  with confidence (including one 31-table unbroken byte-for-byte chain in
  the binary), giving 43/246 and 111/187 icons a real `Group` — **781/1293
  rows (60%)** labeled.
- **1870/3003 rows (62%) across both CSVs combined are now labeled**, up
  from 330/2478 (13%) at the start of this session. **Phase 2 is
  functionally complete** — every file in both CSVs has been investigated
  as far as the available evidence supports; what's left unlabeled is
  either a deliberate `Group="?"` (no source table exists, e.g.
  `button00.blp`) or genuinely absent from v1.0's smaller icon tables
  (content the sequel added later), not unfinished work.
- Decor tiles (`decor0NN.blp`, both games) investigated and closed as
  **not applicable** — confirmed to be full-screen scrolling background
  images with no per-tile rectangle structure, not a sprite grid at all;
  see the finding under Phase 2 below.
- `gifs` also had its background-key removal hardened: a position-
  independent sweep for the engine's exact declared transparency color now
  runs after the border flood-fill, since ~75% of crops still had visible
  leftover chroma-key blue that wasn't connected to the crop's edge.

See the "Milestone" writeups under Phase 2 for both games — including two
real false-positive byte matches caught and excluded during the
`object.blp`/`element.blp` v1.0 scan (worth reading as methodology, not
just result).

**Phase 3** (the actual deliverables) then ran against the complete Phase 2
data: `draw`/`extract`/`gifs` over every file in both games, output to
`tmp/speedy_blupi_{I,II}/{draw,extract,gifs}/` in this repo (gitignored,
not committed — derived from copyrighted assets). 479 animated GIFs total
(218 + 261), one per real `Group`, plus 3003 individually extracted sprite
images and fully annotated sheets for both games. See Phase 3 below.

Only Phase 4 (future/optional, not scheduled) remains open.

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
- [x] 4th background-key revision: even after the border flood-fill above,
      a lot of blue chroma-key still leaked through into the GIFs
      whenever it wasn't connected to a crop's edge (an icon touching the
      border on every side leaves an enclosed "moat" of background the
      flood-fill can never reach). Measured directly: **~75% of all crops
      across both games (1877/2487) still had >15% of their area left as
      near-exact background blue after the flood-fill pass**, worst in
      `text.blp`'s mostly-empty 16x16 glyph cells (364/384 cells in both
      games) and in small icons on oversized bounding boxes (explosion
      frames, vent fans). Fix, scoped to `GifsCommand.cpp` only (source
      `.blp` files/CSVs untouched): every `SetTransparent(CHxxx, ...)`
      call in `free-eggbert/src/pixmap.cpp` for every channel `gifs`
      touches (`CHOBJECT`, `CHBLUPI*`, `CHELEMENT`, `CHEXPLO`, `CHBUTTON`,
      `CHJAUGE`, `CHTEXT`) declares the *same* fixed engine transparency
      color, `RGB(0,0,255)` — so after the flood-fill, `gifs` now also
      does a position-independent exact-color sweep (small tolerance) that
      replaces any remaining pixel of that exact color with
      `--gif-background-color`, regardless of border connectivity. Since
      that color is the engine's own declared "never actually drawn"
      marker, matching it directly can't mistake real sprite content for
      background — confirmed no false positives (checked every flagged
      crop's `Group` name for anything suggesting legitimate blue content,
      e.g. `jauge.blp`'s progress-bar fill: 0/8 rows flagged). Verified by
      re-measuring the same worst-offender crops post-fix (0% residual
      blue) and visually, composited against a non-white canvas.

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

**Speedy Blupi I (v1.0) — pixel rectangles:**
- [x] `blupi000.blp`/`object.blp`/`element.blp`/`explo.blp`: the four
      offsets from `analysis.md` §4.6 formalized into a committed,
      reproducible script (`tools/extract_v1_icon_tables.py`) — output
      committed as `tools/data/v1_icon_tables.json` (small integer data,
      not the copyrighted `.blp`/`.exe` assets themselves).

**Speedy Blupi I (v1.0) — group names:**
- [x] `blupi000.blp`: found the v1.0 equivalent of `table_blupi` by
      byte-signature scan (same technique as the IconPack tables, extended
      to a variable-length record shape) — see "Milestone: `blupi000.blp`
      (Speedy Blupi I) done" below.
- [ ] `object.blp`/`element.blp`/`explo.blp`: still needs the v1.0
      equivalent of the ~150 named per-entity tables. Checked and **not
      needed for geometry** — see the milestone writeup — but grouping is
      still blocked on finding them. Unlike `table_blupi`'s distinctive
      `[action_id, frame_count, phase, icon...]` shape, these are bare
      integer arrays with no structural signature to scan for; Ghidra
      Version Tracking (`SB v1.0 to ...` sessions, already in the Ghidra
      project) is the more promising path here, not another byte scan.

#### Milestone: `blupi000.blp` (Speedy Blupi I / v1.0) done

There is no free-eggbert-equivalent decompilation for v1.0 — everything
here was extracted directly from `BLUPI.EXE` (535,552 bytes, from
`Speedy_Blupi_I.7z` on the library drive, not committed).

`tools/extract_v1_table_blupi.py` found the `table_blupi` equivalent by
byte-signature scan: score every 4-byte-aligned offset by how many
consecutive `[action_id (1-200), frame_count (1-400), phase, icon...]`
records it can parse before failing or reaching a clean `0` terminator,
then take the longest chain. One candidate stood out overwhelmingly — 552
positions scored `>=3` records, but the top cluster all converged on the
*same* end offset (`0x2fdd4`) with the winning start (`0x2d5a0`) producing
67 records ending at a literal `0`, versus every other candidate managing
only a handful before failing.

Confirmation beyond the chain length itself: v1.0's action-id *visiting
order* matches free-eggbert's v2.2 `table_blupi` almost exactly for every
action id the two games share (both walk `..., 2, 60, 3, 4, 5, 59, 61, 62,
6, 7, 8, 9, 10, 13, 11, ...` in that *exact* sequence — v2.2 just has a few
more actions interleaved, matching it being the sequel with more
vehicles/features), and record 0 in both (`action=1`, i.e. `ACTION_STOP`)
is byte-for-byte the same shape: `frame_count=330, phase=0`, starting
`icons=[0,0,0,0,0,23,23,23,0,0,...]`. That kind of order match is not
something a false-positive byte pattern produces by chance — the
character animation data was evidently carried over from v1.0 into v2.2
largely unchanged.

`Group` names reuse free-eggbert's v2.2 `ACTION_*` constants (no v1.0
header exists) — justified by the same order-match evidence. One record
has `action_id=118`, well outside `def.hpp`'s known 1-87 range; left
unlabeled rather than guessed at.

**Merge with existing hand labels — this file actually had real,
verified prior work**, unlike `blupi000.blp` in `speedy_blupi_II.spritesheet.csv`:
37 rows were already hand-measured and marked `Tags=ok`
(`Yellow_Eggbert_Swimming_Right/Left/Butt`, `Yellow_Eggbert_Born`,
`Yellow_Eggbert_Crouching_Left/Right`, `Yellow_Eggbert_Life`,
`Yellow Bomb`). Resolved each one's `(X,Y,Width,Height)` and matched
against the 291 `table_icon_blupi` rectangles: **all 37 matched
*exactly*, 0px off** — the two-years-ago manual measurement was fully
correct, just walked in visual (left-to-right) order instead of the
table's internal order. All 37 preserved verbatim (`Group`, `Number in
Group`, `Tags=ok`), with the auto-detected `ACTION_*` name kept in `Notes`
as a cross-reference rather than discarded. See `HUMAN_OVERRIDES` in
`tools/generate_blupi000_v1_rows.py`.

Verified with `sprite_utils draw` before replacing (both before and after
applying the human-label overrides) — every one of the 291 rectangles
lands on its sprite across the full 784×770 sheet.

Result: **237/291 icons (81%) now have a real `Group`**, up from 37 (13%,
the pre-existing hand-labeled rows only). `ACTION_MARCH` (2 frames —
v1.0's walk cycle is simpler than v2.2's 6) renders as a clean walk.

**Checked, not replaced: `object.blp`/`element.blp`/`explo.blp` (v1.0)**
— unlike v1.0's `blupi000.blp` or v2.2's whole CSV, these were *not*
junk. Resolved every existing row and matched against
`table_icon_object`/`element`/`explo`: **244/246 (99%), 171/188 (91%) and
46/54 (85%) already match exactly.** This geometry was already
hand-verified (most rows carry a `Notes=ok`) and correct — reordering it
to match the table's internal index order would only lose the existing
row/column organization for no benefit, since `Group` labels can't be
added yet anyway (see above). Left untouched pending named-table
equivalents.

#### Milestone: `explo.blp` (Speedy Blupi I / v1.0) done — 12/54 icons

Revisited once named tables became findable (see the caveat above).
Byte-signature scanning `BLUPI.EXE` v1.0 for each of v2.2's 12 `CHEXPLO`
tables (`table_explo1-8`, `table_sploutch1-3`, `table_tentacule`, from
`free-eggbert/src/dectables.cpp`) found exactly 3 with any real
confidence: `table_explo2`, `table_explo3` and `table_explo4` are
byte-for-byte identical to v2.2's values, laid out contiguously and in
the same declaration order at `0x32590`/`0x325e0`/`0x32630` — an exact +
contiguous + correctly-ordered triple match is not chance. The other 9
were **not** found with usable confidence: an exact-byte search turned up
nothing, and the general "run of values in the valid icon range" scan
that found v1.0's `table_blupi` was far too noisy here (explo's icon
range is only 54 values wide, so short/generic sequences and zero-padding
runs collide constantly) — rather than guess, they were left alone.

Geometry itself was *not* touched — only `Group`/`Number in Group` added
to the existing (already-verified) rows. Getting there took a second
matching pass: 46/54 rows already matched `table_icon_explo` exactly (as
found above); the remaining 8 were resolved with a min-cost assignment
(Hungarian algorithm, summed pixel error across X/Y/Width/Height) between
those 8 rows and the 8 otherwise-unclaimed icons — worst residual 7px
total across all 4 dimensions, and since exactly 54 rows must map to
exactly 54 icons, an assignment is the right tool, not a threshold guess.
That recovered icons 7/14/15, all 3 of which turned out to be referenced
by the confirmed tables. Verified with `sprite_utils draw` (rectangles
still land correctly, unchanged) and by rendering `table_explo4`'s GIF — a
clean 5-frame burst-to-embers fade, exactly what an explosion animation
should look like.

Result: **12/54 icons (22%) now have a real `Group`** — a genuinely
partial result, clearly documented as such (`tools/generate_explo_v1_rows.py`)
rather than padded out with guesses. `object.blp`/`element.blp` (v1.0)
remained geometry-only after this — see the next milestone for why they
turned out much better than expected.

#### Milestone: `object.blp`/`element.blp` (Speedy Blupi I / v1.0) done — 43/246, 111/187

The last open item in either CSV, and expected to be the hardest — v2.2's
`CHANNEL_MAP` names 76 `CHOBJECT`/`CHELEMENT` tables (vs. `explo.blp`'s
12), and `explo.blp`'s own experience suggested a low hit rate. It went
the other way: byte-signature scanning found **41/76 (54%) with real
confidence** — a much better rate than `explo.blp`'s 3/12, because many of
these tables turned out to sit in one enormous, gapless, correctly-ordered
run: `table_bulldozer_left` through `table_glu` is a single **31-table
unbroken chain** from `0x32040` to `0x32818` in `BLUPI.EXE`, plus a second
9-table chain (the `decor_vent*`/`decor_ventillo*` fan tables +
`table_guepe_turn2r`) at `0x2d018`-`0x2d0ac`, plus a handful more resolved
individually. See `tools/data/v1_object_element_confirmed_tables.json` for
every table's exact offset.

This scan also caught two real mistakes before they became wrong data,
worth recording as methodology, not just result:
- **`table_marine`'s first "exact match" was a false positive** — found
  at `0x13e`, which is not 4-byte aligned, sitting inside the PE header
  itself (a periodic data region that produced *hundreds* of overlapping
  "matches" for the same 11-int pattern). Every real table lives at a
  4-byte-aligned offset (they're compiled `int[]` arrays); adding an
  alignment check and cross-checking occurrence *counts* (a genuine table
  is unique or explainable by adjacency to a confirmed neighbor - a false
  one recurs dozens of times) caught this. `table_explo6` (excluded from
  the `explo.blp` milestone above) is the same failure mode: individually
  "found", but short, generic, and unsupported by any cluster.
- **Naive "first occurrence" `.find()` silently misrouted values** —
  several short tables (`table_blupih_right`, `table_blupit_right`,
  `table_glu`) have a coincidental *second* copy of their own content
  elsewhere in the file (self-overlap or reuse), so blindly trusting the
  first match would have used the wrong offset. It didn't change the
  *values* extracted here (both occurrences are byte-identical, so the
  Group data is the same either way) but the wrong-offset version would
  have wrongly reported these as "isolated, unconfirmed" instead of
  correctly slotting into the 31-table chain - the fix mattered for
  confidence, not for the resulting CSV. A separate instance of this bug
  was more serious during investigation: an early coverage count
  accidentally routed the 5 `CHEXPLO` tables' values into `element.blp`'s
  tally via a fallback `else` branch, inflating it from a true 111/187 to
  a wrong 126/187 — caught by re-deriving the count from a clean,
  explicitly-`CHELEMENT`-filtered pass before trusting it.

The other 35 tables were not found, and for a legitimate reason, not a
search failure: most reference icon indices beyond v1.0's smaller
`table_icon_object` (246, vs v2.2's 441) / `table_icon_element` (187, vs
v2.2's 289) tables — content the sequel added that simply doesn't exist
yet in this build.

Geometry untouched, same as `explo.blp`: existing rows already matched
`table_icon_object`/`element` at 244/246 and 171/188 (see "Checked, not
replaced" above); the small remainder resolved by the same min-cost
(Hungarian) assignment, recovering 2 more `object.blp` rows and 16 more
`element.blp` rows (worst residual 30 across 4 dimensions on one
`element.blp` row, all others 1-4px). `element.blp` has 188 CSV rows for
only 187 real icons - one row (index 38) has no counterpart at all and is
explicitly left `?` with a note, rather than force-matched.

Verified with `sprite_utils draw` on both files (every rectangle lands
correctly) and by rendering `table_pollution`'s GIF - a clean 8-frame
dissipating dust-cloud animation, exactly what a "pollution" particle
effect should look like.

Result: **43/246 (17%) `object.blp` and 111/187 (59%) `element.blp` icons
now have a real `Group`** — up from 0. This closes out Phase 2's last open
item; every file in both CSVs has now been investigated as far as the
available evidence supports.

#### Milestone: `text.blp` (both games) done

`text.blp` is **not** addressed through a `pixtables.hpp`-style rectangle
table at all, unlike every sheet handled so far — it's a plain uniform
grid. Confirmed directly from `CPixmap::CacheAll`'s `totalDim`/`iconDim`
for channel `CHTEXT` (`totalDim.x=256, totalDim.y=384, iconDim.x=16,
iconDim.y=16` — `free-eggbert/src/pixmap.cpp:1093-1096`) and `DrawIcon`'s
generic grid-fallback formula (`rect.left=(rank%nbx)*iconDim.x,
rect.top=(rank/nbx)*iconDim.y`) — 16 columns × 24 rows = 384 cells,
identical for both games (same canvas size, same font table).

The grid-cell → character mapping comes from `table_char[]` in
`include/texttables.hpp` (256 entries × 6 shorts: `charIcon, offX, offY,
accentIcon, accentOffX, accentOffY` — see `CDecor::DrawChar` in
`src/text.cpp`). `rank = charIcon + font*128`, for `font` in
`{FONTWHITE=0, FONTGOLD=1, FONTSELECTED=2}` — `3*128=384`, exactly the
grid size. Cross-checked against the printable ASCII range (32-126):
`charIcon == byte value` exactly for every one of those 95 bytes, a 1:1
identity mapping that gives high confidence in the formula overall.
`Group` is `char_<byte>` (or `accent_<charIcon>` for cells only used as
an accent overlay); `Number in Group` is the font index (1/2/3) — reusing
the group/animation-frame mechanism to encode "the same character in its
3 font styles" as a harmless side effect (`tools/generate_text_rows.py`).

Verified with `sprite_utils draw` on both games' `text.blp` before
replacing — every one of the 384 cells lands on its glyph.

Result: **375/384 cells (98%) now have a real `Group`**, up from 0. The 9
unlabeled cells are grid positions no character in `table_char` maps to
(charIcon values with a gap).

#### Milestone: `button00.blp` (both games) done — geometry only

Same grid mechanism as `text.blp`, confirmed the same way: `CHBUTTON`'s
`totalDim`/`iconDim` in `CPixmap::CacheAll`
(`free-eggbert/src/pixmap.cpp:1078-1081`) gives `iconDim.x=40,
iconDim.y=40`, canvas width 240 (6 columns). Canvas *height* differs by
game — 840 (21 rows) for Speedy Blupi I, 1040 (26 rows) for Speedy Blupi
II (fewer menu icons in the earlier game) — confirmed both from
`pixmap.cpp`'s literal `totalDim.y=1040` for the v2.2 build and directly
measuring each game's actual `button00.blp` file (840 vs 1040), so the
generator takes canvas height as an argument rather than assuming one
constant (`tools/generate_button00_rows.py`).

Unlike `text.blp`, there is **no lookup table giving each grid cell a
confirmed meaning** — `src/button.cpp` shows ranks 0-5 are generic button
*states* (normal/hover/pressed/.../locked, reused by every button) and
rank 6+ is `m_iconMenu[i] + 6`, an arbitrary per-caller menu-icon index,
not a fixed named set. So — deliberately, unlike every other sheet done
this session — `Group="?"` for every cell here; the geometry is solid
(same grid mechanism already visually verified for `text.blp`) but a
per-cell semantic name would be a guess, not a finding. `Notes` records
which cells are "state 0-5" vs. "menu-icon rank N" so the geometry is at
least self-documenting.

Verified with `sprite_utils draw` on both games (126 cells for v1.0, 156
for v2.2) — every rectangle lands on its icon across the full sheet.

**Decor tiles (both games, `decor0NN.blp`) — finding: not a sprite grid,
nothing to generate.** `CHDECOR` is loaded through `BackgroundCache`
(like every other channel) but with `iconDim.x=0, iconDim.y=0` and
`totalDim = LXIMAGE × LYIMAGE` = `640×480` — literally the game's screen
resolution, not a tile cell size (`free-eggbert/src/pixmap.cpp:1068-1076`,
`include/def.hpp:40-41`). `CDecor::Build`
(`free-eggbert/src/decor.cpp:421-450`) confirms why: it's drawn as one
large scrolling background, blitted through a viewport `rect` that wraps
around modulo `DIMDECORX`/`DIMDECORY` as the camera moves — there is no
per-tile rectangle table and no discrete "sprite" to number, because the
whole file *is* one image. The actual per-cell world data (`Cellule.icon`
in `CDecor::m_decor[MAXCELX][MAXCELY]`) indexes into `object.blp`
world-tile icons, not into `decor0NN.blp` — so `object.blp`'s existing
coverage already accounts for the real per-tile sprite data; `decor0NN.blp`
itself has no sprite-sheet structure to annotate. Closed, not deferred.

#### Milestone: `jauge.blp` (both games) done — 3/4 rows labeled

Same 124×88 canvas in both games (byte-identical file). Geometry confirmed
the same reliable way as `text.blp`/`button00.blp`: `CHJAUGE`'s
`totalDim`/`iconDim` in `CPixmap::CacheAll`
(`free-eggbert/src/pixmap.cpp:1090-1094`, `DIMJAUGEX`/`DIMJAUGEY` in
`include/def.hpp:64-65`) gives a plain 1-column × 4-row grid, 124×22 per
cell. Unlike `button00.blp`, though, this one *could* be partially named:
`CJauge::Draw` itself (`src/jauge.cpp`) is not usable evidence — its
decompilation is visibly broken (writes one byte into a 12-byte buffer,
then reads it back as a 32-bit `LOWORD`/`HIWORD` pair) — but the 4 rows
turned out to just be 4 flat-colored full-width bars (black/red/cyan/
yellow), and every call site touching `CJauge::m_type` (the row index)
across the *entire* decompiled codebase was grep'd: `decor.cpp:81`
(`m_jauges[JAUGE_AIR].Create(..., type=1, ...)`), `decblupi.cpp:2051`
(`SetType(1)`), `decblupi.cpp:2780` (`SetType(2)`), `decor.cpp:83`
(`m_jauges[JAUGE_POWER].Create(..., type=3, ...)`) — no call anywhere sets
`type=0`. That's exhaustive, not a sample, so row 0 (black) is left
`Group="?"` rather than guessed, while rows 1/2 (red/cyan) are grouped as
`JAUGE_AIR` (the oxygen gauge, 2 states) and row 3 (yellow) as
`JAUGE_POWER` (the shield/power gauge, 1 state). Verified with
`sprite_utils draw` on both games — all 4 rectangles land exactly on
their bar.

**Merge and finalize:**
- [x] Merge policy established and applied for every file done so far:
      human labels win where they exist and geometry checks out (exact
      pixel match required, not assumed — see the `blupi000.blp` v1.0
      milestone for the 37/37 case, and the `object`/`element`/`explo.blp`
      v1.0 "checked, not replaced" case for when *not* to touch existing
      work); auto-generated data fills the rest; the source table/action
      name is cross-referenced in `Notes` either way, never silently
      dropped.
- [x] Sanity-check via `sprite_utils draw` before every replacement so
      far (`blupi000.blp`, `object`/`element`/`explo.blp` for v2.2,
      `text.blp`, `button00.blp`, `jauge.blp`, `explo.blp`,
      `object.blp`/`element.blp` for v1.0) — every rectangle generated to
      date has landed correctly.
- [x] `text.blp` (both games): done, 375/384 cells (98%) labeled.
- [x] `button00.blp` (both games): geometry done (126/156 cells per
      game), `Group` deliberately left `?` — no source table exists.
- [x] `jauge.blp` (both games): done, 3/4 rows labeled (`JAUGE_AIR` ×2,
      `JAUGE_POWER` ×1) from an exhaustive call-site trace; row 0 left
      `?` since no caller sets it.
- [x] `explo.blp` (Speedy Blupi I / v1.0): partially done, 12/54 icons
      (22%) — only 3 of its 12 named tables were locatable in `BLUPI.EXE`
      with real confidence; the other 9 left `?` rather than guessed.
- [x] `object.blp`/`element.blp` (Speedy Blupi I / v1.0): done, 41/76
      named tables found by byte-signature scan (43/246 and 111/187
      icons labeled) — better than `explo.blp`'s hit rate thanks to two
      large gapless contiguous chains in the binary; two false-positive
      matches (misaligned/coincidental) caught and excluded along the way.
- [x] Decor tiles: investigated and closed — not a sprite grid, nothing
      to generate (see finding above).
- [x] **Phase 2 is functionally complete.** Every file in both CSVs has
      been investigated; nothing remains marked "not yet attempted".

---

### Phase 3 — Generate the actual deliverables

Depends on Phase 1 (tool) + Phase 2 (complete data). Done.

- [x] Ran `sprite-utils draw` over every file in both games → annotated
      sheets with every sprite rectangle + number visible, in
      `tmp/speedy_blupi_{I,II}/draw/`.
- [x] Ran `sprite-utils extract` over both games → one image file per
      sprite (1293 for Speedy Blupi I, 1710 for Speedy Blupi II),
      organized by file/group, in `tmp/speedy_blupi_{I,II}/extract/`.
- [x] Ran `sprite-utils gifs` over both games now that Phase 2 has
      populated real groups for most rows → 218 + 261 = 479 meaningful
      animated GIFs (up from the handful of hand-labelled groups plus one
      big `?` catch-all per file this produced before Phase 2), in
      `tmp/speedy_blupi_{I,II}/gifs/`.
- [x] Outputs live in this repo, under `/tmp/` (gitignored, not
      committed — derived from copyrighted game assets, same reasoning as
      `/gifs_output/` and `/gen/`; see `analysis.md` §8) rather than a
      separate assets repo. Regenerate anytime with `draw`/`extract`/
      `gifs` against the two spritesheet CSVs and each game's own `.blp`
      files (not distributed with this repo either).

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
