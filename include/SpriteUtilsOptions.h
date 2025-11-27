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

#ifndef SPRITEUTILSOPTIONS_H
#define SPRITEUTILSOPTIONS_H

#include "SpriteUtilsArgs.h"
#include <string>
#include <optional>

/**
 *
 * @author robertvokac
 */
using std::string;

struct Color
{
    int r{}, g{}, b{}, a{};
    constexpr Color() = default;

    constexpr Color(int r_, int g_, int b_, int a_ = 0) : r(r_), g(g_), b(b_), a(a_)
    {
    };
    static const Color RED;
};

class SpriteUtilsOptions
{
private:
    SpriteUtilsArgs spriteUtilsArgs;

public:
    SpriteUtilsOptions(const SpriteUtilsArgs& spriteUtilsArgsIn);

    string getWorkingDirectory() const;

    bool isDrawNumberEnabled() const;

    bool isDrawNumberBackgroundEnabled() const;

    bool isNumberDoubleSized() const;

    Color getRectangleColor() const;

    string getSpriteSheetPath() const;

    std::optional<string> getFileName() const;

    std::optional<int> getRow() const;
};
#endif // SPRITEUTILSOPTIONS_H
