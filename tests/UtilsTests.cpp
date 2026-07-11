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
#include "Utils.h"

SP_TEST_CASE(split_defaultCommaDelimiter) {
    auto parts = Utils::split("a,b,c");
    SP_CHECK_EQ(parts.size(), size_t{3});
    SP_CHECK_EQ(parts[0], std::string("a"));
    SP_CHECK_EQ(parts[1], std::string("b"));
    SP_CHECK_EQ(parts[2], std::string("c"));
}

SP_TEST_CASE(split_customDelimiter) {
    auto parts = Utils::split("a;b;c", ";");
    SP_CHECK_EQ(parts.size(), size_t{3});
    SP_CHECK_EQ(parts[0], std::string("a"));
    SP_CHECK_EQ(parts[2], std::string("c"));
}

SP_TEST_CASE(split_preservesTrailingEmptyField) {
    // sprite-utils' CSV parser relies on this: a line ending in ";" must
    // produce a trailing empty element, not silently drop it.
    auto parts = Utils::split("a;b;", ";");
    SP_CHECK_EQ(parts.size(), size_t{3});
    SP_CHECK_EQ(parts[2], std::string(""));
}

SP_TEST_CASE(split_preservesEmptyMiddleField) {
    auto parts = Utils::split("a;;c", ";");
    SP_CHECK_EQ(parts.size(), size_t{3});
    SP_CHECK_EQ(parts[1], std::string(""));
}

SP_TEST_CASE(split_emptyInputYieldsSingleEmptyField) {
    auto parts = Utils::split("", ";");
    SP_CHECK_EQ(parts.size(), size_t{1});
    SP_CHECK_EQ(parts[0], std::string(""));
}

SP_TEST_CASE(split_noDelimiterYieldsWholeInput) {
    auto parts = Utils::split("abc", ";");
    SP_CHECK_EQ(parts.size(), size_t{1});
    SP_CHECK_EQ(parts[0], std::string("abc"));
}
