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

// These tests check the GIF container's structural markers (magic bytes,
// loop extension, image separators, trailer) rather than fully decoding
// pixels back out -- this project has no GIF *reader*. Full pixel-level
// verification of GifWriter's output was done once, out of band, by
// decoding real generated files with Python/Pillow; see plan.md.

#include "TestFramework.h"
#include "GifWriter.h"
#include "Utils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {

std::vector<uint8_t> readAllBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

size_t countByte(const std::vector<uint8_t>& bytes, uint8_t value) {
    size_t count = 0;
    for (uint8_t b : bytes) {
        if (b == value) ++count;
    }
    return count;
}

bool containsBytes(const std::vector<uint8_t>& haystack, const std::string& needle) {
    if (needle.size() > haystack.size()) return false;
    for (size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
        if (std::equal(needle.begin(), needle.end(), haystack.begin() + static_cast<long>(i))) {
            return true;
        }
    }
    return false;
}

std::filesystem::path tempGifPath(const std::string& name) {
    return std::filesystem::temp_directory_path() / ("sprite_utils_test_" + name + ".gif");
}

} // namespace

SP_TEST_CASE(gifWriter_zeroFramesThrows) {
    auto path = tempGifPath("zeroFrames");
    std::vector<cv::Mat> frames;
    SP_CHECK_THROWS(GifWriter::writeAnimatedGif(path, frames, 100, cv::Vec3b(0, 0, 0)));
}

SP_TEST_CASE(gifWriter_writesValidHeaderTrailerAndLoopExtension) {
    auto path = tempGifPath("headerTrailer");
    std::vector<cv::Mat> frames{
        cv::Mat(2, 2, CV_8UC3, cv::Scalar(255, 0, 0)),
        cv::Mat(2, 2, CV_8UC3, cv::Scalar(0, 255, 0)),
        cv::Mat(2, 2, CV_8UC3, cv::Scalar(0, 0, 255)),
    };

    SP_CHECK_NO_THROW(GifWriter::writeAnimatedGif(path, frames, 100, cv::Vec3b(0, 0, 0)));

    auto bytes = readAllBytes(path);
    SP_CHECK(bytes.size() > 20);
    SP_CHECK(bytes.size() >= 6);
    SP_CHECK(std::string(bytes.begin(), bytes.begin() + 6) == std::string("GIF89a"));
    SP_CHECK_EQ(bytes.back(), uint8_t{0x3B}); // trailer

    SP_CHECK(containsBytes(bytes, "NETSCAPE2.0"));

    // one Image Descriptor (0x2C) per frame
    SP_CHECK_EQ(countByte(bytes, 0x2C), size_t{3});
    // one Graphic Control Extension (0xF9) per frame
    SP_CHECK_EQ(countByte(bytes, 0xF9), size_t{3});
}

SP_TEST_CASE(gifWriter_handlesDifferentlySizedFrames) {
    auto path = tempGifPath("differentSizes");
    std::vector<cv::Mat> frames{
        cv::Mat(4, 6, CV_8UC3, cv::Scalar(10, 20, 30)),
        cv::Mat(8, 3, CV_8UC3, cv::Scalar(40, 50, 60)),
    };

    SP_CHECK_NO_THROW(GifWriter::writeAnimatedGif(path, frames, 50, cv::Vec3b(255, 255, 255)));

    auto bytes = readAllBytes(path);
    // Logical Screen Descriptor width/height (bytes 6-9, little-endian)
    // must be the max across frames: width=6, height=8.
    SP_CHECK_EQ(static_cast<int>(bytes[6]) | (static_cast<int>(bytes[7]) << 8), 6);
    SP_CHECK_EQ(static_cast<int>(bytes[8]) | (static_cast<int>(bytes[9]) << 8), 8);
}

SP_TEST_CASE(gifWriter_handlesManyUniqueColorsViaMedianCut) {
    // A gradient with far more than 256 unique colors forces the
    // median-cut quantization fallback path.
    cv::Mat gradient(32, 32, CV_8UC3);
    for (int y = 0; y < gradient.rows; ++y) {
        for (int x = 0; x < gradient.cols; ++x) {
            gradient.at<cv::Vec3b>(y, x) = cv::Vec3b(
                static_cast<uint8_t>(x * 8), static_cast<uint8_t>(y * 8), static_cast<uint8_t>((x + y) * 4));
        }
    }

    auto path = tempGifPath("manyColors");
    std::vector<cv::Mat> frames{gradient};

    SP_CHECK_NO_THROW(GifWriter::writeAnimatedGif(path, frames, 100, cv::Vec3b(0, 0, 0)));

    auto bytes = readAllBytes(path);
    SP_CHECK(bytes.size() > 6);
    SP_CHECK(std::string(bytes.begin(), bytes.begin() + 6) == std::string("GIF89a"));
    SP_CHECK_EQ(bytes.back(), uint8_t{0x3B});
}
