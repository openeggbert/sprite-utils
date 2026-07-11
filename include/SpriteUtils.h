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

#ifndef SPRITEUTILS_H
#define SPRITEUTILS_H

#include <unordered_map>
#include <memory>
#include <string>

#include "SpriteUtilsArgs.h"
#include "Command.h"
#include "DrawCommand.h"
#include "ExtractCommand.h"
#include "HelpCommand.h"
#include "RestoreCommand.h"
#include "VersionCommand.h"
#include "SpriteUtilsException.h"

class SpriteUtils {
private:
    // command name -> command instance
    std::unordered_map<std::string, std::unique_ptr<Command>> commandImplementations;

public:
    SpriteUtils();

    void run(const std::vector<std::string>& args);
    void run(const SpriteUtilsArgs& spriteUtilsArgs);
};
#endif // SPRITEUTILS_H
