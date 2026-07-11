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


#include "SpriteSheet.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <fstream>

SpriteSheet::SpriteSheet(const std::filesystem::path& file, int scaleIn) : scale(scaleIn)
{
    if (scale < 1)
        throw SpriteUtilsException("Invalid scale (must be a positive integer): " + std::to_string(scale));

    std::vector<SpriteSheetRow> rows;

    const std::string text = Utils::readTextFromFile(file);

    std::istringstream input(text);
    std::string line;

    // skip 1st line (header)
    std::getline(input, line);

    // process until "skipskip"
    while (std::getline(input, line))
    {
        if (line.find("skipskip") != std::string::npos)
            break;

        processLine(line, rows);
    }

    saveComputedFile(file, text, rows);
}


void SpriteSheet::processLine(const std::string& line, std::vector<SpriteSheetRow>& rows)
{
    SpriteSheetRow row(line);

    validateRow(row);

    SpriteSheetRow* previous = rows.empty() ? nullptr : &rows.back();

    if (row.column > 1
        && previous != nullptr
        && std::abs(row.height) >= std::abs(previous->height)
        && (row.column > 2 ? (previous->height != 0 && row.height != 0) : false))
    {
        row.height = -row.height;
    }

    updateSpriteSheetRow(row);

    rows.push_back(row);
    updateMap(row);

    std::cout << row.toCsvLine() << std::endl;
}


void SpriteSheet::updateSpriteSheetRow(SpriteSheetRow& spriteSheetRow)
{
    if (spriteSheetRow.x == -1) {
        if (spriteSheetRow.column == 1)
            spriteSheetRow.x = 0;
        else {
            if (!lastSpriteSheetRow.has_value())
                throw SpriteUtilsException("Could not compute X for " + spriteSheetRow.createId());
            spriteSheetRow.x = lastSpriteSheetRow->x + lastSpriteSheetRow->width;
        }
    }

    if (spriteSheetRow.height <= 0) {
        if (!lastSpriteSheetRow.has_value())
            throw SpriteUtilsException("Could not compute height for " + spriteSheetRow.createId());
        spriteSheetRow.height = lastSpriteSheetRow->height + (-spriteSheetRow.height);
    }

    lastSpriteSheetRow = spriteSheetRow;    // <- COPY, no pointer!
}


void SpriteSheet::updateMap(const SpriteSheetRow& spriteSheetRow)
{
    map[spriteSheetRow.file].push_back(spriteSheetRow);
}


void SpriteSheet::validateRow(const SpriteSheetRow& spriteSheetRow)
{
    if (lastSpriteSheetRow.has_value()) {
        if (spriteSheetRow.file != lastSpriteSheetRow->file) {
            lastSpriteSheetRow.reset();
        }
    }

    if (!lastSpriteSheetRow.has_value()) {
        validateFirstRow(const_cast<SpriteSheetRow&>(spriteSheetRow));
    } else {
        validateSubsequentRow(const_cast<SpriteSheetRow&>(spriteSheetRow));
    }
}


void SpriteSheet::validateFirstRow(SpriteSheetRow& spriteSheetRow)
{
    if (spriteSheetRow.row != 1 || spriteSheetRow.column != 1)
        throw SpriteUtilsException("Invalid initial row or column for file " + spriteSheetRow.file);

    spriteSheetRow.numberPerSheet = 1;
}


void SpriteSheet::validateSubsequentRow(SpriteSheetRow& spriteSheetRow)
{
    spriteSheetRow.numberPerSheet = lastSpriteSheetRow->numberPerSheet + 1;

    if (spriteSheetRow.row > lastSpriteSheetRow->row)
    {
        if (spriteSheetRow.row != (lastSpriteSheetRow->row + 1))
            throw SpriteUtilsException("Unexpected row for " + spriteSheetRow.createId());
    }
    else if (spriteSheetRow.row < lastSpriteSheetRow->row)
    {
        throw SpriteUtilsException("Unexpected row for " + spriteSheetRow.createId());
    }
    else if (spriteSheetRow.column != (lastSpriteSheetRow->column + 1))
    {
        throw SpriteUtilsException("Unexpected column for " + spriteSheetRow.createId());
    }
}


void SpriteSheet::saveComputedFile(const std::filesystem::path& file,
                                   const std::string& originalText,
                                   const std::vector<SpriteSheetRow>& rows)
{
    std::filesystem::path computed = file.string() + ".computed.csv";

    if (std::filesystem::exists(computed))
        std::filesystem::remove(computed);

    std::ofstream out(computed);

    if (!out.is_open())
        throw SpriteUtilsException("Cannot write computed CSV: " + computed.string());

    std::istringstream input(originalText);
    std::string header;
    std::getline(input, header);

    out << header << "\n";

    for (const auto& r : rows)
        out << r.toCsvLine() << "\n";
}


SpriteSheetRow SpriteSheet::applyScale(const SpriteSheetRow& row, int scale)
{
    if (scale == 1)
        return row;

    SpriteSheetRow scaled = row;
    scaled.x *= scale;
    scaled.y *= scale;
    scaled.width *= scale;
    scaled.height *= scale;
    return scaled;
}


std::vector<SpriteSheetRow> SpriteSheet::getSpriteSheetRows(const std::string& file)
{
    std::vector<SpriteSheetRow> result;
    for (const auto& row : map[file])
        result.push_back(applyScale(row, scale));
    return result;
}


std::vector<SpriteSheetRow> SpriteSheet::getSpriteSheetRows()
{
    std::vector<SpriteSheetRow> res;

    for (auto& p : map)
        for (const auto& row : p.second)
            res.push_back(applyScale(row, scale));

    return res;
}
