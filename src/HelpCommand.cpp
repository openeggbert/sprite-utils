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
                        --scale={positive integer}
                            Optional. Default=1. Multiplies every X/Y/Width/
                            Height from the CSV by this factor before use -
                            for running the same 1x-authored CSV against a
                            2x/4x/8x re-rendered source image.
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
                        --scale={positive integer}
                            Optional. Default=1. See "draw" above.
                        --out-dir={directory to write extracted sprites to}
                            Optional. Default={--dir}/extracted

    restore     Restore every "<file>.backup" in the working directory back
                over its original file, in one step.
                    OPTIONS
                        --dir={working directory}
                            Optional. Default=. (current directory)
                        --file-name={single file name to restore}
                            Optional. Default=(all backed-up files in --dir).

    gifs        Build one animated GIF per (file, Group) found in the
                sprite-sheet CSV: every sprite sharing a Group, in
                "Number in Group" order, becomes one frame, written to
                --out-dir/<source-file-stem>/<group>.gif.
                    OPTIONS
                        --dir={working directory}
                            Optional. Default=. (current directory)
                        --sprite-sheet-path={path to the sprite-sheet CSV}
                            Optional. Default={--dir}/spritesheet.csv
                        --file-name={single image file name to process}
                            Optional. Default=(all image files in --dir).
                        --group={single group name to build}
                            Optional. Default=(every group found).
                        --scale={positive integer}
                            Optional. Default=1. See "draw" above.
                        --out-dir={directory to write GIFs to}
                            Optional. Default={--dir}/gifs
                        --frame-delay-ms={milliseconds per frame}
                            Optional. Default=100
                        --gif-background-color={R,G,B}
                            Optional. Default=255,255,255 (used to pad
                            frames up to the animation's largest frame size)

    pack        The inverse of "extract": pastes individual sprite images
                back together into one full sheet image per source file,
                using the sprite-sheet CSV's rectangles for placement. Reads
                images from --dir/<source-file-stem>/<file name>, where
                <file name> is "extract"'s own naming
                ("<numberPerSheet>__<group>__<numberInGroup>.png") - meant
                for round-tripping through an external re-render step (e.g.
                re-rendering each extracted sprite from a 3D model at a
                higher resolution, then packing the results back into one
                sheet with --scale set to match). The output canvas is
                sized from the CSV alone, not any existing image. Written
                to --out-dir/<source-file-stem>.png. Missing sprite images
                are skipped with a warning (leaving that area as
                background); a loaded image that doesn't already match its
                CSV rectangle's size is resized to fit, with a warning.
                    OPTIONS
                        --dir={working directory}
                            Optional. Default=. (current directory)
                        --sprite-sheet-path={path to the sprite-sheet CSV}
                            Optional. Default={--dir}/spritesheet.csv
                        --file-name={single sheet file name to pack}
                            Optional. Default=(every file found in the CSV).
                        --scale={positive integer}
                            Optional. Default=1. See "draw" above.
                        --out-dir={directory to write packed sheets to}
                            Optional. Default={--dir}/packed
                        --background-color={R,G,B}
                            Optional. Default=255,255,255, opaque - unless
                            any input sprite has an alpha channel, in which
                            case the canvas defaults to fully transparent
                            instead (a proper modern sprite atlas), and
                            this option's R,G,B is only used, opaquely, if
                            explicitly given.

    help        Display help information
    version     Display version information
)";

    std::cout << str << std::endl;
    return str;
}
