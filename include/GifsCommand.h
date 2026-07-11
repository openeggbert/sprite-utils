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

#ifndef GIFSCOMMAND_H
#define GIFSCOMMAND_H

#include <string>
#include <opencv2/core.hpp>

#include "Command.h"
#include "SpriteUtilsArgs.h"
#include "SpriteUtilsOptions.h"

/**
 * Builds one animated GIF per (file, group) found in the sprite-sheet CSV:
 * every sprite sharing a Group, in Number in Group order, becomes one frame.
 *
 * @author robertvokac
 */
class GifsCommand : public Command {
public:
    static constexpr const char* NAME = "gifs";

    GifsCommand() = default;
    ~GifsCommand() override = default;

    std::string getName() const override { return NAME; }
    std::string run(const SpriteUtilsArgs& args) override;

private:
    static cv::Vec3b toVec3b(const Color& c);
};
#endif // GIFSCOMMAND_H
