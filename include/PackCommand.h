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

#ifndef PACKCOMMAND_H
#define PACKCOMMAND_H

#include <string>

#include "Command.h"
#include "SpriteUtilsArgs.h"

/**
 * The inverse of "extract": given a directory of individual sprite images
 * (one per sprite-sheet row, named the same way "extract" names its own
 * output - "<numberPerSheet>__<group>__<numberInGroup>.png" under
 * "<dir>/<source-file-stem>/"), pastes each one back at its sprite-sheet
 * CSV rectangle to rebuild one full sheet image per source file, written
 * to "<out-dir>/<source-file-stem>.png".
 *
 * Meant for round-tripping through an external re-render step - e.g.
 * "extract" a sheet, re-render each individual sprite from a 3D model at a
 * higher resolution, then "pack" the results back into a single sheet
 * image using the same (optionally "--scale"d) CSV geometry. The sheet
 * canvas itself is sized from the CSV, not read from any existing image,
 * since there may not be one yet at the new resolution.
 *
 * @author robertvokac
 */
class PackCommand : public Command {
public:
    static constexpr const char* NAME = "pack";

    PackCommand() = default;
    ~PackCommand() override = default;

    std::string getName() const override { return NAME; }
    std::string run(const SpriteUtilsArgs& args) override;
};
#endif // PACKCOMMAND_H
