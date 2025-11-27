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



#include "Utils.h"

#include "SpriteUtilsException.h"
#include <fstream>
#include <sstream>


void Utils::listAllFilesInDirInternal(
    const std::filesystem::path& dir,
    std::vector<std::filesystem::path>& files)
{
    files.push_back(dir);
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_directory())
        {
            listAllFilesInDirInternal(entry.path(), files);
        }
        else
        {
            files.push_back(entry.path());
        }
    }
}


std::vector<std::filesystem::path> Utils::listAllFilesInDir(const std::filesystem::path& dir)
{
    std::vector<std::filesystem::path> files;
    listAllFilesInDirInternal(dir, files);
    return files;
}

void Utils::copyFile(const std::filesystem::path& original, const std::filesystem::path& copied)
{
    try
    {
        std::filesystem::copy_file(
            original,
            copied,
            std::filesystem::copy_options::overwrite_existing
        );
    }
    catch (const std::exception& e)
    {
        throw SpriteUtilsException(
            "Copying file failed: " + original.string() + " " + e.what()
        );
    }
}

void Utils::writeTextToFile(const std::string& text, const std::filesystem::path& file)
{
    std::ofstream out(file);
    if (!out.is_open())
    {
        throw SpriteUtilsException("Writing to file failed: " + file.string());
    }
    out << text;
}

std::string Utils::readTextFromFile(const std::filesystem::path& file)
{
    if (!std::filesystem::exists(file))
    {
        return "";
    }

    std::ifstream in(file);
    if (!in.is_open())
    {
        throw SpriteUtilsException("Reading file failed: " + file.string());
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::string Utils::readFromInputStream(std::istream& input)
{
    std::stringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::vector<std::string> Utils::split(const std::string& input, const std::string& delim) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t dlen = delim.length();

    for (size_t pos = input.find(delim, start);
         pos != std::string::npos;
         pos = input.find(delim, start))
    {
        result.emplace_back(input.substr(start, pos - start));
        start = pos + dlen;
    }

    result.emplace_back(input.substr(start));
    return result;
}
