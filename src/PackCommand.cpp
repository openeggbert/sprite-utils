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

#include "PackCommand.h"

#include <filesystem>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <set>
#include <sstream>
#include <vector>

#include "SpriteSheet.h"
#include "SpriteSheetRow.h"
#include "SpriteUtilsException.h"
#include "SpriteUtilsOptions.h"
#include "Utils.h"

namespace {

// Same naming ExtractCommand writes, so "extract" -> (re-render each file
// externally, keeping filenames) -> "pack" round-trips.
std::string spriteFileName(const SpriteSheetRow& row) {
    std::ostringstream name;
    name << row.numberPerSheet << "__" << Utils::sanitizeForFilename(row.group)
         << "__" << row.numberInGroup << ".png";
    return name.str();
}

} // namespace

std::string PackCommand::run(const SpriteUtilsArgs& args) {
    SpriteUtilsOptions opt(args);

    const std::filesystem::path workingDir(opt.getWorkingDirectory());
    const std::filesystem::path outDir(opt.getPackOutputDirectory());
    const Color background = opt.getPackBackgroundColor();
    const bool explicitBackground = opt.hasExplicitPackBackgroundColor();

    std::cout << "Going to pack individual sprite images from directory: " << workingDir
               << " into: " << outDir << "\n";

    std::filesystem::create_directories(outDir);

    // Load the spritesheet CSV
    SpriteSheet spriteSheet(std::filesystem::path(opt.getSpriteSheetPath()), opt.getScale());

    // Distinct File values, in first-seen (CSV) order.
    std::vector<std::string> fileOrder;
    std::set<std::string> seen;
    for (const auto& row : spriteSheet.getSpriteSheetRows()) {
        if (seen.insert(row.file).second)
            fileOrder.push_back(row.file);
    }

    int packedCount = 0;
    for (const auto& fileName : fileOrder) {
        if (opt.getFileName().has_value() && opt.getFileName().value() != fileName)
            continue;

        auto rows = spriteSheet.getSpriteSheetRows(fileName);
        if (rows.empty())
            continue;

        const std::filesystem::path spriteDir = workingDir / std::filesystem::path(fileName).stem();
        if (!std::filesystem::is_directory(spriteDir)) {
            std::cerr << "Warning: no sprite directory found for " << fileName
                       << " (expected " << spriteDir << "), skipping\n";
            if (opt.getFileName().has_value())
                break;
            continue;
        }

        // Canvas size comes entirely from the (already --scale'd) CSV
        // geometry, not from any existing image - there may not be one yet.
        int canvasWidth = 0;
        int canvasHeight = 0;
        for (const auto& row : rows) {
            canvasWidth = std::max(canvasWidth, row.x + std::max(1, row.width));
            canvasHeight = std::max(canvasHeight, row.y + std::max(1, row.height));
        }

        std::vector<std::pair<const SpriteSheetRow*, cv::Mat>> loaded;
        loaded.reserve(rows.size());
        bool useAlpha = false;
        int missing = 0;
        for (const auto& row : rows) {
            const std::filesystem::path spriteFile = spriteDir / spriteFileName(row);
            if (!std::filesystem::exists(spriteFile)) {
                std::cerr << "Warning: missing sprite image, leaving background: " << spriteFile << "\n";
                ++missing;
                loaded.emplace_back(&row, cv::Mat());
                continue;
            }
            cv::Mat img = cv::imread(spriteFile.string(), cv::IMREAD_UNCHANGED);
            if (img.empty())
                throw SpriteUtilsException("Reading packed sprite image failed: " + spriteFile.string());
            if (img.channels() == 4)
                useAlpha = true;
            loaded.emplace_back(&row, img);
        }

        cv::Mat canvas;
        if (useAlpha) {
            // Transparent by default (a proper modern sprite atlas), unless
            // the caller explicitly asked for an opaque background.
            const int alpha = explicitBackground ? 255 : 0;
            canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC4,
                              cv::Scalar(background.b, background.g, background.r, alpha));
        } else {
            canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC3,
                              cv::Scalar(background.b, background.g, background.r));
        }

        const cv::Rect canvasBounds(0, 0, canvas.cols, canvas.rows);
        int placed = 0;
        for (auto& [rowPtr, img] : loaded) {
            if (img.empty())
                continue;

            if (img.channels() == 1) {
                cv::cvtColor(img, img, useAlpha ? cv::COLOR_GRAY2BGRA : cv::COLOR_GRAY2BGR);
            } else if (img.channels() == 3 && useAlpha) {
                cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);
            }

            const int targetWidth = std::max(1, rowPtr->width);
            const int targetHeight = std::max(1, rowPtr->height);
            if (img.cols != targetWidth || img.rows != targetHeight) {
                std::cerr << "Warning: resizing " << rowPtr->createId() << " from "
                           << img.cols << "x" << img.rows << " to " << targetWidth << "x"
                           << targetHeight << " to fit the sprite-sheet CSV rectangle\n";
                const int interp = (img.cols < targetWidth || img.rows < targetHeight)
                                        ? cv::INTER_LINEAR
                                        : cv::INTER_AREA;
                cv::resize(img, img, cv::Size(targetWidth, targetHeight), 0, 0, interp);
            }

            const cv::Rect dest(rowPtr->x, rowPtr->y, targetWidth, targetHeight);
            const cv::Rect clipped = dest & canvasBounds;
            if (clipped.width <= 0 || clipped.height <= 0) {
                std::cerr << "Warning: skipping sprite with an out-of-bounds rectangle: "
                           << rowPtr->createId() << "\n";
                continue;
            }
            const cv::Rect srcRect(clipped.x - dest.x, clipped.y - dest.y, clipped.width, clipped.height);
            img(srcRect).copyTo(canvas(clipped));
            ++placed;
        }

        const std::filesystem::path outFile = outDir / (std::filesystem::path(fileName).stem().string() + ".png");
        if (!cv::imwrite(outFile.string(), canvas))
            throw SpriteUtilsException("Writing packed sheet failed: " + outFile.string());

        std::cout << "Wrote " << outFile << " (" << canvasWidth << "x" << canvasHeight << ", "
                   << placed << "/" << rows.size() << " sprite(s) placed, " << missing << " missing)\n";
        ++packedCount;

        if (opt.getFileName().has_value() && opt.getFileName().value() == fileName)
            break;
    }

    std::cout << "Packed " << packedCount << " sheet(s).\n";
    return "";
}
