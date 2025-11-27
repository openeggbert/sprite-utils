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


/**
 * Represents a row in a sprite sheet.
 * This struct is responsible for parsing and serializing sprite sheet rows from/to CSV format.
 * 
 * @author robertvokac
 */
#ifndef SPRITESHEETROW_H
#define SPRITESHEETROW_H
#include <optional>
#include <string>

class SpriteSheetRow
{
private:
    static constexpr const char* DELIMITER = ";";
    static constexpr const char* ID_DELIMITER = "__";
    static constexpr int MINIMUM_COLUMNS = 10;
    static std::optional<int> parseOptionalInt(const std::string& value);

public:
    std::string file;
    std::string group;
    int numberInGroup = 0;
    int row = 0;
    int column = 0;
    int x = -1;
    int y = 0;
    int width = 0;
    int height = 0;
    std::string notes;
    std::string tags;
    int numberPerSheet = 0;

    // parse from CSV line
    explicit SpriteSheetRow(const std::string& csvLine);

    std::string createId() const;
    std::string toCsvLine() const;
};

#endif // SPRITESHEETROW_H
