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


#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <map>
#include <vector>
#include <string>
#include <filesystem>

#include "SpriteSheetRow.h"
#include "SpriteUtilsException.h"

class SpriteSheet
{
private:
    std::map<std::string, std::vector<SpriteSheetRow>> map;

    static int lastX;
    static int lastWidth;
    static int lastHeight;
    std::optional<SpriteSheetRow> lastSpriteSheetRow = std::nullopt;

    void processLine(const std::string& line, std::vector<SpriteSheetRow>& rows);
    void updateSpriteSheetRow(SpriteSheetRow& spriteSheetRow);
    void updateMap(const SpriteSheetRow& spriteSheetRow);
    void validateRow(const SpriteSheetRow& spriteSheetRow);
    void validateFirstRow(SpriteSheetRow& spriteSheetRow);
    void validateSubsequentRow(SpriteSheetRow& spriteSheetRow);
    void saveComputedFile(const std::filesystem::path& file,
                          const std::string& originalText,
                          const std::vector<SpriteSheetRow>& rows);

public:
    explicit SpriteSheet(const std::filesystem::path& file);

    std::vector<SpriteSheetRow> getSpriteSheetRows(const std::string& file);
    std::vector<SpriteSheetRow> getSpriteSheetRows();
};
#endif // SPRITESHEET_H
