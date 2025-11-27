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


#ifndef DRAWCOMMAND_H
#define DRAWCOMMAND_H

#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>

#include "Command.h"
#include "SpriteUtilsArgs.h"
#include "SpriteUtilsOptions.h"

class DrawCommand : public Command {
public:
    static constexpr const char* NAME = "draw";

    DrawCommand() = default;
    ~DrawCommand() override = default;

    std::string getName() const override { return NAME; }
    std::string run(const SpriteUtilsArgs& args) override;

private:
    // --- BMP I/O helpers to preserve bit depth ---
    static uint16_t readBmpBpp(const std::filesystem::path& file);
    static void writeBmp16BGR565(const std::filesystem::path& out, const cv::Mat& bgr8);
    static void writeBmp8Gray(const std::filesystem::path& out, const cv::Mat& bgr8);

    // --- drawing helpers ---
    static cv::Scalar toScalar(const Color& c); // B,G,R
    static void  drawDashedRect(cv::Mat& img, cv::Rect rc, const cv::Scalar& color);
    static void  drawNumber(cv::Mat& img, int number, int endX, int endY, bool doubleSize, const SpriteUtilsOptions& opt);
    static void  drawDigitBlock(cv::Mat& img, int digit, int startX, int startY, int scale, bool fillBackground, bool backgroundWhite, bool foregroundYellow);

    // 3x5 font mask for digits '0'..'9'
    static const bool* digitMask(char ch);

    // utility
    static inline int clampi(int v, int lo, int hi) { return std::max(lo, std::min(hi, v)); }
};
#endif // DRAWCOMMAND_H
