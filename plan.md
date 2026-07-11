# sprite-utils Plan

**Status as of 2026-07-11:** Phase 0 done (analysis). Phase 1 starting.

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
  `draw` — already implemented),
- individual per-sprite image files (via `extract` — not yet implemented),
- animated GIFs per animation group (via `gifs` — not yet implemented).

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

- [ ] Implement `extract` — crop `[X, Y, X+Width, Y+Height]` out of the
      source image for every CSV row and write it to its own file.
      (`include/SpriteUtilsCommand.h` already has the `EXTRACT` enum value;
      needs an `ExtractCommand` class mirroring `DrawCommand`'s structure and
      wiring in `src/SpriteUtils.cpp`.)
- [ ] Implement `restore` — copy every `<file>.backup` back over `<file>` in
      the working directory. Mirrors backup logic already in
      `DrawCommand::run` (`src/DrawCommand.cpp`).
- [ ] Fix `HelpCommand.cpp` — replace the stale option names (`color`,
      `files`, `groups`, `positon`, `number-per-group`) with the real
      `--`-prefixed options in `SpriteUtilsArgs`/`SpriteUtilsOptions`, and
      document `extract`/`restore` once implemented.
- [ ] Add a test suite covering the CSV-parsing edge cases already documented
      in `web/file-formats.html` / `web/tutorials/csv-format.html`: implicit
      `X` computation, the height-negation encoding for multi-column rows,
      the `skipskip` sentinel, first-row (`row==1,column==1`) validation,
      sequential row/column validation, minimum-column validation.
- [ ] Remove `SpriteSheet`'s global static state (`lastX`, `lastWidth`,
      `lastHeight` in `include/SpriteSheet.h`/`src/SpriteSheet.cpp`) so the
      class is reentrant — needed once Phase 2 tooling processes multiple
      sheets/files in one process or the test suite runs multiple cases.
- [ ] Wrap `main()` in a top-level try/catch so a `SpriteUtilsException`
      prints a clean error instead of an unhandled-exception crash.
- [ ] Build + smoke-test: run `draw`, `extract`, `restore` against the real
      CSVs in `spritesheets/` and confirm output looks correct.

`gifs` is deliberately **not** in this phase — it only produces something
useful once `Group`/`Number in Group` data actually exists for most rows,
which is what Phase 2 delivers. Implementing it now would mean testing it
against today's mostly-`Group=?` data.

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
- [ ] Implement and run `sprite-utils gifs` → one animated GIF per
      `Group`/animation sequence, now that groups are populated.
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
