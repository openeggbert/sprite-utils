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


#include "SpriteSheetRow.h"
#include <sstream>
#include <stdexcept>

#include "Utils.h"

std::optional<int> SpriteSheetRow::parseOptionalInt(const std::string& value) {
    if (value.empty()) {
        return std::nullopt;
    }
    try {
        return std::stoi(value);
    } catch (...) {
        throw std::invalid_argument("Invalid integer format: " + value);
    }
}

// --- constructor from CSV line ---

SpriteSheetRow::SpriteSheetRow(const std::string& csvLine)
{
    auto cols = Utils::split(csvLine, DELIMITER);

    if (cols.size() < MINIMUM_COLUMNS) {
        throw std::invalid_argument("CSV line does not contain enough columns: " + csvLine);
    }

    try {
        int i = 0;
        file = cols[i++];
        group = cols[i++];
        numberInGroup = std::stoi(cols[i++]);
        row = std::stoi(cols[i++]);
        column = std::stoi(cols[i++]);
        x = parseOptionalInt(cols[i++]).value_or(-1);
        y = std::stoi(cols[i++]);
        width = std::stoi(cols[i++]);

        std::string hstr = cols[i++];
        height = (hstr.empty() || hstr == "0") ? 0 : std::stoi(hstr);

        notes = i < cols.size() ? cols[i++] : "";
        tags = i < cols.size() ? cols[i++] : "";
        numberPerSheet = i < cols.size() ? std::stoi(cols[i]) : 0;

        if (column > 1) {
            height = -height;
        }
    }
    catch (...) {
        throw std::invalid_argument("CSV line contains invalid number format: " + csvLine);
    }
}

// --- createId ---
std::string SpriteSheetRow::createId() const {
    return file + ID_DELIMITER + group + ID_DELIMITER + std::to_string(numberInGroup);
}

// --- serialize back to CSV ---
std::string SpriteSheetRow::toCsvLine() const {
    std::ostringstream oss;
    oss << file << DELIMITER
        << group << DELIMITER
        << numberInGroup << DELIMITER
        << row << DELIMITER
        << column << DELIMITER
        << x << DELIMITER
        << y << DELIMITER
        << width << DELIMITER
        << height << DELIMITER
        << notes << DELIMITER
        << tags << DELIMITER
        << numberPerSheet;

    return oss.str();
}