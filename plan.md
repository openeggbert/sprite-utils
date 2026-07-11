# sprite-utils Plan

**Status as of 2026-07-11:** Phase 0 done (analysis). Phase 1 done and
verified (build + unit test suite + a real-data smoke test all pass).
`gifs` (normally Phase 3) was pulled forward and implemented early, by
request, to have a visible/fun result before Phase 2's data work — see the
"gifs, pulled forward" note under Phase 1 below. Phase 2 (auto-generating
the complete CSVs) not started yet.

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

---

### Phase 2 — Auto-generate complete CSVs for both games

Reference: `analysis.md` §3, §4, §5 "Track B", §4.6. This is the actual data
work — turning tables that already exist inside `free-eggbert` (for Speedy
Blupi II / v2.2) and tables located directly inside `BLUPI.EXE` v1.0 in this
session (for Speedy Blupi I, see `analysis.md` §4.6) into complete CSV rows,
without hand-measuring pixels.

**Speedy Blupi II (v2.2) — pixel rectangles:**
- [ ] Dump `table_icon_blupi`/`table_icon_object`/`table_icon_element`/
      `table_icon_explo` from `free-eggbert/include/pixtables.hpp` to a
      portable format (JSON).
- [ ] Generate CSV rows (`File`, `X`, `Y`, `Width`, `Height`,
      `Number per file`) for `blupi000-003.blp`, `object.blp`,
      `element.blp`, `explo.blp` from those tables (see `analysis.md` §4.2
      for the channel → file mapping).

**Speedy Blupi II (v2.2) — group names:**
- [ ] Dump `table_blupi` and the ~150 named tables (`table_bulldozer_left`,
      `table_shield`, `table_explo1`, etc.) from
      `free-eggbert/src/dectables.cpp`.
- [ ] Overlay `Group`/`Number in Group` on the rows above: `table_blupi`
      (keyed by `ACTION_*` in `include/def.hpp`) for `blupi0NN.blp`; the
      named tables for `object.blp`/`element.blp`/`explo.blp`.

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
