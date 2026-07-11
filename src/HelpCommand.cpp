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
    sprite_utils - "Sprite Utils"

SYNOPSIS
    sprite_utils [command] [--option value]...

DESCRIPTION
    Tools used to work with sprite sheets, described by a semicolon-delimited
    CSV file (see --sprite-sheet-path below).

COMMAND
    draw        Draw a dashed rectangle (and, optionally, the sprite's
                computed sequence number) over every sprite described by the
                sprite-sheet CSV, for every matching image file in the
                working directory. A ".backup" copy of each image is made
                (or restored from, on the next run) before drawing.
                    OPTIONS
                        --dir={working directory}
                            Optional. Default=. (current directory)
                        --sprite-sheet-path={path to the sprite-sheet CSV}
                            Optional. Default={--dir}/spritesheet.csv
                        --file-name={single image file name to process}
                            Optional. Default=(all image files in --dir).
                        --row={row number to draw, starting at 1}
                            Optional. Default=(all rows).
                        --rectangle-color={R,G,B}
                            Optional. Default=255,0,0
                        --draw-number={true|false}
                            Optional. Default=true
                        --draw-number-background={true|false}
                            Optional. Default=true
                        --double-sized-number={true|false}
                            Optional. Default=false

    extract     Cut every sprite rectangle described by the sprite-sheet CSV
                out of its source image into its own PNG file, under
                --out-dir/<source-file-stem>/.
                    OPTIONS
                        --dir={working directory}
                            Optional. Default=. (current directory)
                        --sprite-sheet-path={path to the sprite-sheet CSV}
                            Optional. Default={--dir}/spritesheet.csv
                        --file-name={single image file name to process}
                            Optional. Default=(all image files in --dir).
                        --out-dir={directory to write extracted sprites to}
                            Optional. Default={--dir}/extracted

    restore     Restore every "<file>.backup" in the working directory back
                over its original file, in one step.
                    OPTIONS
                        --dir={working directory}
                            Optional. Default=. (current directory)
                        --file-name={single file name to restore}
                            Optional. Default=(all backed-up files in --dir).

    help        Display help information
    version     Display version information
)";

    std::cout << str << std::endl;
    return str;
}
