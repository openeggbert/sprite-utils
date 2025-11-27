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

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <filesystem>

class Utils {
private:
    Utils() = delete; // Not meant to be instantiated.

    static void listAllFilesInDirInternal(
        const std::filesystem::path& dir,
        std::vector<std::filesystem::path>& files);

public:

    static std::vector<std::filesystem::path> listAllFilesInDir(const std::filesystem::path& dir);

    static void copyFile(const std::filesystem::path& original, const std::filesystem::path& copied);

    static void writeTextToFile(const std::string& text, const std::filesystem::path& file);

    static std::string readTextFromFile(const std::filesystem::path& file);

    static std::string readFromInputStream(std::istream& input);

    static std::vector<std::string> split(const std::string& input, const std::string& delim = ",");
};
#endif // UTILS_H
