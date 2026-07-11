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

#include "GifsCommand.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "GifWriter.h"
#include "SpriteSheet.h"
#include "SpriteSheetRow.h"
#include "SpriteUtilsException.h"
#include "Utils.h"

namespace {

// The color-key background isn't a single uniform color across an entire
// sheet, and isn't even reliably the majority color *within* one frame's
// own crop - some icons (trees, spheres, ...) nearly fill their whole
// bounding box, leaving only slivers of true background peeking through.
// A statistical guess (corner pixel, sheet-wide or per-crop histogram) gets
// those wrong in both directions: it can miss real background, or - worse -
// mistake a large solid patch of icon interior for background and erase
// part of the sprite.
//
// Flood-filling in from the frame's own border is safer: it only ever
// removes background pixels *connected to the edge*, so an icon that fills
// its whole cell (no reachable border strip of background) is correctly
// left untouched instead of being cut into. Sampling multiple points
// around the border (not just the corners) means a single corner sitting
// on top of icon content doesn't stop the whole fill from finding the
// background elsewhere along the edge.
void keyOutBackground(cv::Mat& frame, const cv::Vec3b& background) {
    const int w = frame.cols;
    const int h = frame.rows;
    if (w <= 0 || h <= 0)
        return;

    const cv::Scalar fill(background[0], background[1], background[2]);
    // Small per-step tolerance: floodFill's default (non-fixed-range) mode
    // compares each candidate pixel to its already-filled neighbor, so a
    // gently varying background still gets fully connected while a sharp
    // jump into icon content still stops the fill.
    const cv::Scalar tolerance(20, 20, 20);

    const std::vector<cv::Point> seeds = {
        {0, 0}, {w - 1, 0}, {0, h - 1}, {w - 1, h - 1},
        {w / 2, 0}, {w / 2, h - 1}, {0, h / 2}, {w - 1, h / 2},
    };
    for (const auto& seed : seeds) {
        cv::floodFill(frame, seed, fill, nullptr, tolerance, tolerance, 4);
    }
}

} // namespace

cv::Vec3b GifsCommand::toVec3b(const Color& c) {
    // OpenCV uses BGR order, not RGB (see DrawCommand::toScalar).
    return cv::Vec3b(static_cast<uint8_t>(c.b), static_cast<uint8_t>(c.g), static_cast<uint8_t>(c.r));
}

std::string GifsCommand::run(const SpriteUtilsArgs& args) {
    SpriteUtilsOptions opt(args);

    const std::filesystem::path workingDir(opt.getWorkingDirectory());
    const std::filesystem::path outDir(opt.getGifsOutputDirectory());
    const int delayMs = opt.getGifFrameDelayMs();
    const std::optional<std::string> groupFilter = opt.getGroup();
    const cv::Vec3b background = toVec3b(opt.getGifBackgroundColor());

    std::cout << "Going to build animated GIFs from images in directory: " << workingDir
               << " into: " << outDir << "\n";

    std::filesystem::create_directories(outDir);

    // Load the spritesheet CSV
    SpriteSheet spriteSheet(std::filesystem::path(opt.getSpriteSheetPath()), opt.getScale());

    int gifCount = 0;

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

        // group rows by Group, keeping each group's Number in Group order
        std::map<std::string, std::vector<SpriteSheetRow>> byGroup;
        for (const auto& row : rows) {
            if (groupFilter.has_value() && groupFilter.value() != row.group)
                continue;
            byGroup[row.group].push_back(row);
        }

        if (byGroup.empty()) {
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

        const cv::Rect bounds(0, 0, img.cols, img.rows);
        const std::filesystem::path fileOutDir = outDir / imageFile.stem();

        for (auto& groupEntry : byGroup) {
            const std::string& groupName = groupEntry.first;
            std::vector<SpriteSheetRow>& groupRows = groupEntry.second;

            // stable: rows with an unlabeled ("?") group all share the same
            // placeholder Number in Group, so a plain sort would leave their
            // relative (sheet reading) order unspecified instead of stable
            std::stable_sort(groupRows.begin(), groupRows.end(),
                [](const SpriteSheetRow& a, const SpriteSheetRow& b) {
                    return a.numberInGroup < b.numberInGroup;
                });

            std::vector<cv::Mat> frames;
            frames.reserve(groupRows.size());
            for (const auto& row : groupRows) {
                const cv::Rect rc(row.x, row.y, std::max(1, row.width), std::max(1, row.height));
                const cv::Rect clipped = rc & bounds;
                if (clipped.width <= 0 || clipped.height <= 0) {
                    std::cerr << "Warning: skipping sprite with an out-of-bounds rectangle: "
                               << row.createId() << "\n";
                    continue;
                }
                cv::Mat frame = img(clipped).clone();
                keyOutBackground(frame, background);
                frames.push_back(frame);
            }

            if (frames.empty())
                continue;

            std::filesystem::create_directories(fileOutDir);
            const std::filesystem::path outFile = fileOutDir / (Utils::sanitizeForFilename(groupName) + ".gif");
            GifWriter::writeAnimatedGif(outFile, frames, delayMs, background);
            std::cout << "Wrote " << outFile << " (" << frames.size() << " frame(s))\n";
            ++gifCount;
        }

        if (opt.getFileName().has_value() &&
            opt.getFileName().value() == imageFileName)
        {
            break;
        }
    }

    std::cout << "Wrote " << gifCount << " GIF(s).\n";
    return "";
}
