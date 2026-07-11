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

#include "ExtractCommand.h"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "SpriteSheet.h"
#include "SpriteSheetRow.h"
#include "SpriteUtilsException.h"
#include "SpriteUtilsOptions.h"

std::string ExtractCommand::sanitizeForFilename(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_') {
            out += c;
        } else {
            out += '_';
        }
    }
    return out.empty() ? "_" : out;
}

std::string ExtractCommand::run(const SpriteUtilsArgs& args) {
    SpriteUtilsOptions opt(args);

    const std::filesystem::path workingDir(opt.getWorkingDirectory());
    const std::filesystem::path outDir(opt.getExtractOutputDirectory());

    std::cout << "Going to extract sprites from images in directory: " << workingDir
               << " into: " << outDir << "\n";

    std::filesystem::create_directories(outDir);

    // Load the spritesheet CSV
    SpriteSheet spriteSheet(std::filesystem::path(opt.getSpriteSheetPath()));

    int extractedCount = 0;

    for (const auto& entry : std::filesystem::directory_iterator(workingDir)) {
        if (!entry.is_regular_file())
            continue;

        auto imageFile = entry.path();
        const std::string imageFileName = imageFile.filename().string();

        if (opt.getFileName().has_value() &&
            opt.getFileName().value() != imageFileName)
        {
            continue;
        }

        if (imageFileName.ends_with(".backup"))
            continue;

        auto rows = spriteSheet.getSpriteSheetRows(imageFileName);
        if (rows.empty()) {
            if (opt.getFileName().has_value() &&
                opt.getFileName().value() == imageFileName)
            {
                break;
            }
            continue;
        }

        cv::Mat img = cv::imread(imageFile.string(), cv::IMREAD_UNCHANGED);
        if (img.empty())
            throw SpriteUtilsException("Reading image failed: " + imageFile.string());

        if (img.channels() == 1)
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);

        const std::filesystem::path fileOutDir = outDir / imageFile.stem();
        std::filesystem::create_directories(fileOutDir);

        const cv::Rect bounds(0, 0, img.cols, img.rows);

        for (const auto& row : rows) {
            const cv::Rect rc(row.x, row.y, std::max(1, row.width), std::max(1, row.height));
            const cv::Rect clipped = rc & bounds;

            if (clipped.width <= 0 || clipped.height <= 0) {
                std::cerr << "Warning: skipping sprite with an out-of-bounds rectangle: "
                           << row.createId() << "\n";
                continue;
            }

            cv::Mat sprite = img(clipped).clone();

            std::ostringstream name;
            name << row.numberPerSheet << "__" << sanitizeForFilename(row.group)
                 << "__" << row.numberInGroup << ".png";

            const std::filesystem::path outFile = fileOutDir / name.str();
            if (!cv::imwrite(outFile.string(), sprite))
                throw SpriteUtilsException("Writing extracted sprite failed: " + outFile.string());

            ++extractedCount;
        }

        if (opt.getFileName().has_value() &&
            opt.getFileName().value() == imageFileName)
        {
            break;
        }
    }

    std::cout << "Extracted " << extractedCount << " sprite(s).\n";
    return "";
}
