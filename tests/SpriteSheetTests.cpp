/*
 * MIT License
 * Copyright (c) 2024-2025 Robert Vokac
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "TestFramework.h"
#include "SpriteSheet.h"
#include "Utils.h"

#include <filesystem>

namespace {

const char* HEADER = "File;Group;Number in Group;Row;Column;X;Y;Width;Height;Notes;Tags;Number per file\n";

std::filesystem::path writeTempCsv(const std::string& testName, const std::string& body) {
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / ("sprite_utils_test_" + testName + ".csv");
    Utils::writeTextToFile(HEADER + body, path);
    return path;
}

} // namespace

SP_TEST_CASE(spriteSheet_autoXAndHeightComputation) {
    // Column 1: explicit height (32). Column 2: height inherited unchanged
    // from column 1 (32). Column 3: negative height encodes "+15 relative
    // to the previous resolved height", per web/file-formats.html.
    auto path = writeTempCsv("autoXAndHeight",
        "img.bmp;walk;1;1;1;;0;32;32;;;0\n"
        "img.bmp;walk;2;1;2;;0;28;0;;;0\n"
        "img.bmp;walk;3;1;3;;0;30;15;;;0\n"
        "skipskip\n");

    SpriteSheet sheet(path);
    auto rows = sheet.getSpriteSheetRows("img.bmp");
    SP_CHECK_EQ(rows.size(), size_t{3});

    SP_CHECK_EQ(rows[0].x, 0);
    SP_CHECK_EQ(rows[0].height, 32);
    SP_CHECK_EQ(rows[0].numberPerSheet, 1);

    SP_CHECK_EQ(rows[1].x, 32); // previous x (0) + previous width (32)
    SP_CHECK_EQ(rows[1].height, 32); // inherited unchanged
    SP_CHECK_EQ(rows[1].numberPerSheet, 2);

    SP_CHECK_EQ(rows[2].x, 60); // previous x (32) + previous width (28)
    SP_CHECK_EQ(rows[2].height, 47); // 32 + 15
    SP_CHECK_EQ(rows[2].numberPerSheet, 3);
}

SP_TEST_CASE(spriteSheet_scaleMultipliesResolvedCoordinatesOnly) {
    // Same fixture as spriteSheet_autoXAndHeightComputation, at --scale=2.
    // A naive "multiply each row as soon as it's resolved" implementation
    // would compound: row 1's auto-X reads row 0's (already-scaled) x/width
    // back out of internal state, so it would come out scaled twice. The
    // expected values here are exactly 2x the *unscaled* result (0/32/32,
    // 32/32/28, 60/47/30), not double-scaled (which would show up as an
    // x of 128 instead of 120 on the third row).
    auto path = writeTempCsv("scale",
        "img.bmp;walk;1;1;1;;0;32;32;;;0\n"
        "img.bmp;walk;2;1;2;;0;28;0;;;0\n"
        "img.bmp;walk;3;1;3;;0;30;15;;;0\n"
        "skipskip\n");

    SpriteSheet sheet(path, 2);
    auto rows = sheet.getSpriteSheetRows("img.bmp");
    SP_CHECK_EQ(rows.size(), size_t{3});

    SP_CHECK_EQ(rows[0].x, 0);
    SP_CHECK_EQ(rows[0].width, 64);
    SP_CHECK_EQ(rows[0].height, 64);

    SP_CHECK_EQ(rows[1].x, 64);
    SP_CHECK_EQ(rows[1].width, 56);
    SP_CHECK_EQ(rows[1].height, 64);

    SP_CHECK_EQ(rows[2].x, 120);
    SP_CHECK_EQ(rows[2].width, 60);
    SP_CHECK_EQ(rows[2].height, 94);
}

SP_TEST_CASE(spriteSheet_invalidScaleThrows) {
    auto path = writeTempCsv("invalidScale", "img.bmp;g;1;1;1;0;0;10;10;;;1\n");
    SP_CHECK_THROWS(SpriteSheet(path, 0));
    SP_CHECK_THROWS(SpriteSheet(path, -1));
}

SP_TEST_CASE(spriteSheet_skipskipSentinelStopsProcessing) {
    auto path = writeTempCsv("skipskip",
        "img.bmp;g;1;1;1;0;0;10;10;;;1\n"
        "skipskip\n"
        "img.bmp;g;2;1;2;10;0;10;10;;;2\n");

    SpriteSheet sheet(path);
    auto rows = sheet.getSpriteSheetRows("img.bmp");
    SP_CHECK_EQ(rows.size(), size_t{1});
}

SP_TEST_CASE(spriteSheet_firstRowMustBeRow1Column1) {
    auto path = writeTempCsv("firstRowInvalid",
        "img.bmp;g;1;1;2;0;0;10;10;;;1\n");

    SP_CHECK_THROWS(SpriteSheet(path));
}

SP_TEST_CASE(spriteSheet_rowMustIncreaseByOne) {
    auto path = writeTempCsv("rowSkip",
        "img.bmp;g;1;1;1;0;0;10;10;;;1\n"
        "img.bmp;g;2;3;1;0;20;10;10;;;2\n");

    SP_CHECK_THROWS(SpriteSheet(path));
}

SP_TEST_CASE(spriteSheet_columnMustIncreaseByOneWithinARow) {
    auto path = writeTempCsv("columnSkip",
        "img.bmp;g;1;1;1;0;0;10;10;;;1\n"
        "img.bmp;g;2;1;3;0;0;10;10;;;2\n");

    SP_CHECK_THROWS(SpriteSheet(path));
}

SP_TEST_CASE(spriteSheet_getSpriteSheetRowsFiltersByFile) {
    auto path = writeTempCsv("multiFile",
        "a.bmp;g;1;1;1;0;0;10;10;;;1\n"
        "b.bmp;g;1;1;1;0;0;20;20;;;1\n");

    SpriteSheet sheet(path);
    SP_CHECK_EQ(sheet.getSpriteSheetRows("a.bmp").size(), size_t{1});
    SP_CHECK_EQ(sheet.getSpriteSheetRows("b.bmp").size(), size_t{1});
    SP_CHECK_EQ(sheet.getSpriteSheetRows().size(), size_t{2});
    SP_CHECK_EQ(sheet.getSpriteSheetRows("nonexistent.bmp").size(), size_t{0});
}

SP_TEST_CASE(spriteSheet_instancesDoNotLeakStateIntoEachOther) {
    // Regression guard: two SpriteSheet objects processing a file with the
    // *same name* ("a.bmp") must not see each other's parsing state. Before
    // the dead SpriteSheet::lastX/lastWidth/lastHeight statics (and a
    // shadowed file-static "lastSpriteSheetRow") were removed, a real
    // globally-shared lookback would have made the second instance's first
    // row be misread as a continuation of the first instance's last row.
    auto pathA = writeTempCsv("reentrancyA",
        "a.bmp;g;1;1;1;0;0;10;10;;;1\n"
        "a.bmp;g;2;1;2;0;0;20;0;;;2\n"
        "skipskip\n");
    {
        SpriteSheet sheetA(pathA);
        auto rowsA = sheetA.getSpriteSheetRows("a.bmp");
        SP_CHECK_EQ(rowsA.size(), size_t{2});
    }

    auto pathB = writeTempCsv("reentrancyB",
        "a.bmp;g2;1;1;1;0;0;99;99;;;1\n"
        "skipskip\n");

    SP_CHECK_NO_THROW(SpriteSheet(pathB));

    SpriteSheet sheetB(pathB);
    auto rowsB = sheetB.getSpriteSheetRows("a.bmp");
    SP_CHECK_EQ(rowsB.size(), size_t{1});
    SP_CHECK_EQ(rowsB[0].x, 0);
    SP_CHECK_EQ(rowsB[0].height, 99);
    SP_CHECK_EQ(rowsB[0].numberPerSheet, 1);
}
