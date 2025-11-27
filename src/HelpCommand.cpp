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


#include "HelpCommand.h"

std::string HelpCommand::run(const SpriteUtilsArgs& /*args*/)
{
    std::string str = R"(
NAME
    spriteutils - " Sprite Utils"

SYNOPSIS
    spriteutils [command] [options]

DESCRIPTION
    Tools used to work with sprites.

COMMAND
    draw        draw rectangles for sprites
                    OPTIONS
                        color={rgb value of the rectangle border}
                            Optional. Default=255,0,0
                        files={comma separated list of BMP files in the working directory}
                            Optional. Default=(all BMP files in the working directory).
                        groups={comma separated list of sprite groups}
                            Optional. Default=(all sprite groups).
                        positon={row starting with 0, height starting with 0}
                            Optional. Default=(all sprites).
                        number-per-group={row starting with 0, height starting with 0}
                            Optional. Default=(all sprites).

    help        Display help information
    version     Display version information
)";

    std::cout << str << std::endl;
    return str;
}
