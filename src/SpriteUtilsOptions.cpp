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


#include "SpriteUtilsOptions.h"

#include <sstream>
#include <vector>

#include "Utils.h"

/**
 *
 * @author robertvokac
 */

const Color Color::RED{255, 0, 0, 0};

SpriteUtilsOptions::SpriteUtilsOptions(const SpriteUtilsArgs& spriteUtilsArgsIn) : spriteUtilsArgs(spriteUtilsArgsIn)
{
}

string SpriteUtilsOptions::getWorkingDirectory() const
{
    return spriteUtilsArgs.getArgumentOptional("--dir").value_or(".");
}

bool SpriteUtilsOptions::isDrawNumberEnabled() const
{
    return spriteUtilsArgs.getBooleanArgument("--draw-number", true);
}

bool SpriteUtilsOptions::isDrawNumberBackgroundEnabled() const
{
    return spriteUtilsArgs.getBooleanArgument("--draw-number-background", true);
}

bool SpriteUtilsOptions::isNumberDoubleSized() const
{
    return spriteUtilsArgs.getBooleanArgument("--double-sized-number");
}

Color SpriteUtilsOptions::getRectangleColor() const
{
    std::optional<string> arg = spriteUtilsArgs.getArgumentOptional("--rectangle-color");
    if (!arg.has_value() || arg->empty())
    {
        return Color::RED;
    }
    auto array = Utils::split(arg.value());

    if (array.size() != 3)
    {
        throw SpriteUtilsException("Invalid format of rectangle-color option: " + arg.value());
    }
    return {
        stoi(array[0]),
        stoi(array[1]),
        stoi(array[2])
    };
}

string SpriteUtilsOptions::getSpriteSheetPath() const
{
    return spriteUtilsArgs.getArgumentOptional("--sprite-sheet-path").value_or(
        getWorkingDirectory() + "/spritesheet.csv");
}


std::optional<string> SpriteUtilsOptions::getFileName() const
{
    return spriteUtilsArgs.getArgumentOptional("--file-name");
}

std::optional<int> SpriteUtilsOptions::getRow() const
{
    auto arg = spriteUtilsArgs.getArgumentOptional("--row");
    if (!arg.has_value())
    {
        return std::nullopt;
    }
    try
    {
        return std::stoi(*arg);
    }
    catch (...)
    {
        throw std::invalid_argument("Invalid argument");
    }
};
