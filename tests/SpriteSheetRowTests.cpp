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
#include "SpriteSheetRow.h"

SP_TEST_CASE(spriteSheetRow_tooFewColumnsThrows) {
    // Only 9 fields; MINIMUM_COLUMNS is 10.
    SP_CHECK_THROWS(SpriteSheetRow("a;b;1;1;1;;0;10;10"));
}

SP_TEST_CASE(spriteSheetRow_minimumColumnsParsesWithDefaults) {
    // Exactly 10 fields: notes present, tags/numberPerSheet default.
    SpriteSheetRow row("img.bmp;g;1;1;1;;0;10;10;somenote");
    SP_CHECK_EQ(row.file, std::string("img.bmp"));
    SP_CHECK_EQ(row.group, std::string("g"));
    SP_CHECK_EQ(row.numberInGroup, 1);
    SP_CHECK_EQ(row.row, 1);
    SP_CHECK_EQ(row.column, 1);
    SP_CHECK_EQ(row.x, -1); // empty X -> unresolved sentinel
    SP_CHECK_EQ(row.y, 0);
    SP_CHECK_EQ(row.width, 10);
    SP_CHECK_EQ(row.height, 10); // column==1, no negation
    SP_CHECK_EQ(row.notes, std::string("somenote"));
    SP_CHECK_EQ(row.tags, std::string(""));
    SP_CHECK_EQ(row.numberPerSheet, 0);
}

SP_TEST_CASE(spriteSheetRow_explicitXIsParsed) {
    SpriteSheetRow row("img.bmp;g;1;1;2;5;0;10;7;;;3");
    SP_CHECK_EQ(row.x, 5);
}

SP_TEST_CASE(spriteSheetRow_heightIsNegatedForColumnGreaterThanOne) {
    SpriteSheetRow row("img.bmp;g;1;1;2;5;0;10;7;;;3");
    SP_CHECK_EQ(row.column, 2);
    SP_CHECK_EQ(row.height, -7);
}

SP_TEST_CASE(spriteSheetRow_zeroHeightStaysZeroEvenWhenNegated) {
    SpriteSheetRow row("img.bmp;g;1;1;2;5;0;10;0;;;3");
    SP_CHECK_EQ(row.height, 0);
}

SP_TEST_CASE(spriteSheetRow_heightNotNegatedForFirstColumn) {
    SpriteSheetRow row("img.bmp;g;1;1;1;0;0;10;7;;;1");
    SP_CHECK_EQ(row.height, 7);
}

SP_TEST_CASE(spriteSheetRow_createIdFormat) {
    SpriteSheetRow row("img.bmp;walk;3;1;1;0;0;10;10;;;1");
    SP_CHECK_EQ(row.createId(), std::string("img.bmp__walk__3"));
}

SP_TEST_CASE(spriteSheetRow_toCsvLineRoundTripsAllFields) {
    SpriteSheetRow row("img.bmp;walk;3;2;1;0;5;10;20;a note;tag1,tag2;7");
    SP_CHECK_EQ(row.toCsvLine(),
        std::string("img.bmp;walk;3;2;1;0;5;10;20;a note;tag1,tag2;7"));
}
