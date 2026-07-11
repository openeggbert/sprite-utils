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
#include "SpriteUtilsArgs.h"

SP_TEST_CASE(spriteUtilsArgs_defaultsToDrawWhenEmpty) {
    std::vector<std::string> args;
    SpriteUtilsArgs parsed(args);
    SP_CHECK_EQ(parsed.get_command(), std::string("draw"));
}

SP_TEST_CASE(spriteUtilsArgs_commandIsFirstArgument) {
    std::vector<std::string> args{"extract"};
    SpriteUtilsArgs parsed(args);
    SP_CHECK_EQ(parsed.get_command(), std::string("extract"));
}

SP_TEST_CASE(spriteUtilsArgs_parsesKeyValueOptions) {
    std::vector<std::string> args{"draw", "--dir", "/tmp/sprites"};
    SpriteUtilsArgs parsed(args);
    SP_CHECK(parsed.hasArgument("--dir"));
    SP_CHECK_EQ(parsed.getArgument("--dir"), std::string("/tmp/sprites"));
    SP_CHECK(!parsed.hasArgument("--missing"));
    SP_CHECK(!parsed.getArgumentOptional("--missing").has_value());
}

SP_TEST_CASE(spriteUtilsArgs_optionNotStartingWithDashDashThrows) {
    std::vector<std::string> args{"draw", "dir", "/tmp"};
    SP_CHECK_THROWS(SpriteUtilsArgs(args));
}

SP_TEST_CASE(spriteUtilsArgs_missingValueThrows) {
    std::vector<std::string> args{"draw", "--dir"};
    SP_CHECK_THROWS(SpriteUtilsArgs(args));
}

SP_TEST_CASE(spriteUtilsArgs_valueStartingWithDashDashThrows) {
    std::vector<std::string> args{"draw", "--dir", "--sprite-sheet-path"};
    SP_CHECK_THROWS(SpriteUtilsArgs(args));
}

SP_TEST_CASE(spriteUtilsArgs_booleanArgumentDefaultsAndParses) {
    std::vector<std::string> args{"draw", "--flag", "true"};
    SpriteUtilsArgs parsed(args);
    SP_CHECK_EQ(parsed.getBooleanArgument("--flag"), true);
    SP_CHECK_EQ(parsed.getBooleanArgument("--other", true), true);
    SP_CHECK_EQ(parsed.getBooleanArgument("--other", false), false);
}
