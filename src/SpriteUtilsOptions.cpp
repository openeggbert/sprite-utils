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

Color SpriteUtilsOptions::getColorArgument(const std::string& arg, const Color& default_) const
{
    std::optional<string> value = spriteUtilsArgs.getArgumentOptional(arg);
    if (!value.has_value() || value->empty())
    {
        return default_;
    }
    auto array = Utils::split(value.value());

    if (array.size() != 3)
    {
        throw SpriteUtilsException("Invalid format of " + arg + " option: " + value.value());
    }
    return {
        stoi(array[0]),
        stoi(array[1]),
        stoi(array[2])
    };
}

Color SpriteUtilsOptions::getRectangleColor() const
{
    return getColorArgument("--rectangle-color", Color::RED);
}

string SpriteUtilsOptions::getSpriteSheetPath() const
{
    return spriteUtilsArgs.getArgumentOptional("--sprite-sheet-path").value_or(
        getWorkingDirectory() + "/spritesheet.csv");
}

string SpriteUtilsOptions::getExtractOutputDirectory() const
{
    return spriteUtilsArgs.getArgumentOptional("--out-dir").value_or(
        getWorkingDirectory() + "/extracted");
}

string SpriteUtilsOptions::getGifsOutputDirectory() const
{
    return spriteUtilsArgs.getArgumentOptional("--out-dir").value_or(
        getWorkingDirectory() + "/gifs");
}

int SpriteUtilsOptions::getGifFrameDelayMs() const
{
    auto arg = spriteUtilsArgs.getArgumentOptional("--frame-delay-ms");
    if (!arg.has_value())
    {
        return 100;
    }
    try
    {
        return std::stoi(*arg);
    }
    catch (...)
    {
        throw SpriteUtilsException("Invalid frame-delay-ms option: " + *arg);
    }
}

Color SpriteUtilsOptions::getGifBackgroundColor() const
{
    return getColorArgument("--gif-background-color", Color(255, 255, 255));
}

std::optional<string> SpriteUtilsOptions::getGroup() const
{
    return spriteUtilsArgs.getArgumentOptional("--group");
}

int SpriteUtilsOptions::getScale() const
{
    auto arg = spriteUtilsArgs.getArgumentOptional("--scale");
    if (!arg.has_value())
    {
        return 1;
    }
    int value;
    try
    {
        value = std::stoi(*arg);
    }
    catch (...)
    {
        throw SpriteUtilsException("Invalid scale option (must be a positive integer): " + *arg);
    }
    if (value < 1)
    {
        throw SpriteUtilsException("Invalid scale option (must be a positive integer): " + *arg);
    }
    return value;
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
