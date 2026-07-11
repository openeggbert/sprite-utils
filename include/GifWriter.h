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

#ifndef GIFWRITER_H
#define GIFWRITER_H

#include <cstdint>
#include <filesystem>
#include <vector>

#include <opencv2/core.hpp>

/**
 * A small, self-contained animated GIF (GIF89a) encoder: palette
 * quantization (exact when a frame has <=256 colors, median-cut otherwise)
 * plus the GIF-flavoured variable-width LZW compressor. Written by hand
 * because this project's OpenCV build has no animated GIF/WebP write
 * support (cv::Animation/imwriteanimation is not present in every OpenCV
 * 4.x packaging).
 *
 * @author robertvokac
 */
class GifWriter {
public:
    GifWriter() = delete; // Not meant to be instantiated.

    // Writes an animated, looping GIF built from `framesBgr` (8-bit, 3-channel
    // BGR images as produced by cv::imread/cropping). Frames may differ in
    // size: each is placed at the top-left corner of a shared canvas sized to
    // the largest frame and padded with `backgroundColorBgr`.
    // `frameDelayMs` is the per-frame display time; GIF stores delay in
    // hundredths of a second, so values are rounded to the nearest 10ms
    // (minimum 10ms/1 tick).
    static void writeAnimatedGif(
        const std::filesystem::path& outFile,
        const std::vector<cv::Mat>& framesBgr,
        int frameDelayMs,
        const cv::Vec3b& backgroundColorBgr);

private:
    // Quantizes one BGR image to a palette of up to 256 colors (exact if it
    // already has <=256 distinct colors, median-cut reduced otherwise).
    // Writes the per-pixel palette index (row-major, top-to-bottom) into
    // `indexOut` and returns the palette in RGB order (GIF's native order).
    static std::vector<cv::Vec3b> quantizeToRgbPalette(const cv::Mat& bgr, std::vector<uint8_t>& indexOut);

    // GIF-flavoured LZW encoder over a stream of palette indices.
    static std::vector<uint8_t> lzwEncode(const std::vector<uint8_t>& indices, int minCodeSize);

    static int bitsNeededFor(int paletteSize);
};
#endif // GIFWRITER_H
