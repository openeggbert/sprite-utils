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


#include "DrawCommand.h"
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include "SpriteSheet.h"
#include "SpriteSheetRow.h"
#include "SpriteUtilsException.h"
#include "Utils.h"

// -------------------- Digit masks (3x5) --------------------
// A small fixed bitmap font.
// true = pixel on, false = pixel off
// Stored in row-major order (5 rows × 3 columns)
static const bool DIGIT_0[15] = {
    1,1,1,
    1,0,1,
    1,0,1,
    1,0,1,
    1,1,1
};
static const bool DIGIT_1[15] = {
    0,0,1,
    0,1,1,
    1,0,1,
    0,0,1,
    0,0,1
};
static const bool DIGIT_2[15] = {
    1,1,1,
    0,0,1,
    0,1,0,
    1,0,0,
    1,1,1
};
static const bool DIGIT_3[15] = {
    1,1,1,
    0,0,1,
    1,1,1,
    0,0,1,
    1,1,1
};
static const bool DIGIT_4[15] = {
    0,0,1,
    0,1,0,
    1,1,1,
    0,0,1,
    0,0,1
};
static const bool DIGIT_5[15] = {
    1,1,1,
    1,0,0,
    1,1,1,
    0,0,1,
    1,1,1
};
static const bool DIGIT_6[15] = {
    1,1,1,
    1,0,0,
    1,1,1,
    1,0,1,
    1,1,1
};
static const bool DIGIT_7[15] = {
    1,1,1,
    0,0,1,
    0,0,1,
    0,0,1,
    0,0,1
};
static const bool DIGIT_8[15] = {
    1,1,1,
    1,0,1,
    1,1,1,
    1,0,1,
    1,1,1
};
static const bool DIGIT_9[15] = {
    1,1,1,
    1,0,1,
    1,1,1,
    0,0,1,
    1,1,1
};

// -------------------- Double-sized digit masks (10x6) --------------------
// These are NOT scaled from the small 3x5 font.
// They are manually defined high-resolution bitmaps, same output as Java.
static const bool D0[60] = {
    0,1,1,1,1,0,
    1,1,1,1,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    0,1,1,1,1,0
};

static const bool D1[60] = {
    0,0,0,0,1,1,
    0,0,0,1,1,1,
    0,0,1,1,1,1,
    0,1,1,1,1,1,
    1,1,1,0,1,1,
    1,1,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1
};

static const bool D2[60] = {
    0,1,1,1,1,0,
    1,1,1,1,1,1,
    0,0,0,0,1,1,
    0,0,0,1,1,1,
    0,0,1,1,1,0,
    0,0,1,1,0,0,
    0,1,1,0,0,0,
    0,1,1,0,0,0,
    1,1,1,1,1,1,
    1,1,1,1,1,1
};

static const bool D3[60] = {
    0,1,1,1,1,0,
    1,1,1,1,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    1,1,1,1,1,0,
    1,1,1,1,1,0,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    1,1,1,1,1,1,
    0,1,1,1,1,0
};

static const bool D4[60] = {
    0,0,0,0,1,1,
    0,0,0,1,1,0,
    0,0,1,1,0,0,
    0,1,1,0,0,0,
    1,1,0,0,0,0,
    1,1,1,1,1,1,
    1,1,1,1,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1
};

static const bool D5[60] = {
    1,1,1,1,1,1,
    1,1,1,1,1,1,
    1,1,0,0,0,0,
    1,1,0,0,0,0,
    1,1,1,1,1,0,
    1,1,1,1,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    1,1,1,1,1,1,
    1,1,1,1,1,0
};

static const bool D6[60] = {
    0,1,1,1,1,0,
    1,1,1,1,1,1,
    1,1,0,0,0,0,
    1,1,0,0,0,0,
    1,1,1,1,1,0,
    1,1,1,1,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,1,1,1,1,
    0,1,1,1,1,0
};

static const bool D7[60] = {
    1,1,1,1,1,1,
    1,1,1,1,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    0,0,0,0,1,1
};

static const bool D8[60] = {
    0,1,1,1,1,0,
    1,1,1,1,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    0,1,1,1,1,0,
    0,1,1,1,1,0,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,1,1,1,1,
    0,1,1,1,1,0
};

static const bool D9[60] = {
    0,1,1,1,1,0,
    1,1,1,1,1,1,
    1,1,0,0,1,1,
    1,1,0,0,1,1,
    1,1,1,1,1,1,
    0,1,1,1,1,0,
    0,0,0,0,1,1,
    0,0,0,0,1,1,
    1,1,1,1,1,1,
    1,1,1,1,1,0
};

const bool* DrawCommand::digitMask(char ch) {
    switch (ch) {
        case '0': return DIGIT_0;
        case '1': return DIGIT_1;
        case '2': return DIGIT_2;
        case '3': return DIGIT_3;
        case '4': return DIGIT_4;
        case '5': return DIGIT_5;
        case '6': return DIGIT_6;
        case '7': return DIGIT_7;
        case '8': return DIGIT_8;
        case '9': return DIGIT_9;
        default:  return nullptr;
    }
}

static const bool* bigDigitMask(char ch) {
    switch (ch) {
        case '0': return D0;
        case '1': return D1;
        case '2': return D2;
        case '3': return D3;
        case '4': return D4;
        case '5': return D5;
        case '6': return D6;
        case '7': return D7;
        case '8': return D8;
        case '9': return D9;
        default:  return nullptr;
    }
}

// -------------------- Helpers --------------------

cv::Scalar DrawCommand::toScalar(const Color& c) {
    // OpenCV uses BGR order, not RGB
    return cv::Scalar(c.b, c.g, c.r);
}

void DrawCommand::drawDashedRect(cv::Mat& img, cv::Rect rc, const cv::Scalar& color) {
    // Imitates Java BasicStroke with pattern [1,3].
    // We draw short lines (1 pixel) separated by gaps.
    const int dash = 1;
    const int gap  = 3;

    // top side
    for (int x = rc.x; x < rc.x + rc.width; x += dash + gap) {
        int x2 = std::min(x + dash, rc.x + rc.width - 1);
        cv::line(img, {x, rc.y}, {x2, rc.y}, color, 1, cv::LINE_8);
    }
    // bottom side
    for (int x = rc.x; x < rc.x + rc.width; x += dash + gap) {
        int x2 = std::min(x + dash, rc.x + rc.width - 1);
        cv::line(img, {x, rc.y + rc.height - 1}, {x2, rc.y + rc.height - 1}, color, 1, cv::LINE_8);
    }
    // left side
    for (int y = rc.y; y < rc.y + rc.height; y += dash + gap) {
        int y2 = std::min(y + dash, rc.y + rc.height - 1);
        cv::line(img, {rc.x, y}, {rc.x, y2}, color, 1, cv::LINE_8);
    }
    // right side
    for (int y = rc.y; y < rc.y + rc.height; y += dash + gap) {
        int y2 = std::min(y + dash, rc.y + rc.height - 1);
        cv::line(img,
                 {rc.x + rc.width - 1, y},
                 {rc.x + rc.width - 1, y2},
                 color, 1, cv::LINE_8);
    }
}

void DrawCommand::drawDigitBlock(cv::Mat& img, int digit, int startX, int startY, int scale,
                                 bool fillBackground, bool backgroundWhite, bool foregroundYellow)
{
    const bool* mask = digitMask(char('0' + digit));
    if (!mask)
        throw SpriteUtilsException("Character is not supported: " + std::to_string(digit));

    // Optional background rectangle behind the digit
    if (fillBackground) {
        // Java always used white background
        cv::Scalar bg = backgroundWhite ? cv::Scalar(255,255,255) : cv::Scalar(255,255,255);
        cv::rectangle(img,
            cv::Rect(startX - 1, startY - 1, 3*scale + 2, 5*scale + 2),
            bg, cv::FILLED);
    }

    // Foreground color depends on background mode:
    // black if background is drawn,
    // yellow if digits are standalone
    cv::Scalar fg = foregroundYellow ? cv::Scalar(0,255,255)
                                     : cv::Scalar(0,0,0);

    int idx = 0;
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 3; ++x, ++idx) {
            if (mask[idx]) {
                cv::rectangle(img,
                    cv::Rect(startX + x*scale, startY + y*scale, scale, scale),
                    fg, cv::FILLED);
            }
        }
    }
}

void drawBigDigit(cv::Mat& img, int digit, int startX, int startY, const SpriteUtilsOptions& opt) {
    const bool* mask = bigDigitMask(char('0' + digit));
    if (!mask)
        throw SpriteUtilsException("Unsupported digit");

    if (opt.isDrawNumberBackgroundEnabled()) {
        cv::rectangle(img,
            cv::Rect(startX - 1, startY - 1, 6 + 2, 10 + 2),
            cv::Scalar(255,255,255), cv::FILLED);
    }

    cv::Scalar fg = opt.isDrawNumberBackgroundEnabled()
                    ? cv::Scalar(0,0,0)      // black
                    : cv::Scalar(0,255,255); // yellow

    int idx = 0;

    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 6; ++x, ++idx) {
            if (mask[idx]) {
                cv::rectangle(img,
                    cv::Rect(startX + x, startY + y, 1, 1),
                    fg, cv::FILLED);
            }
        }
    }
}



void DrawCommand::drawNumber(cv::Mat& img, int number, int endX, int endY, bool doubleSize, const SpriteUtilsOptions& opt) {
    std::string s = std::to_string(number);
    int dx = 0;

    for (int i = (int)s.size() - 1; i >= 0; --i) {
        int digit = s[i] - '0';

        if (doubleSize) {

            int scale = 1;
            // big font (10x6)
            int startX = endX - dx - (2*scale + 2*scale) - 1;
            int startY = endY - 10;
            drawBigDigit(img, digit, startX, startY, opt);

            dx += 7;    // spacing between big digits
        } else {
            int scale = 1;
            int startX = endX - dx - (2*scale + 2*scale) + 2;
            int startY = endY - (4 *scale + scale);

            drawDigitBlock(img, digit, startX, startY, scale,
                           opt.isDrawNumberBackgroundEnabled(),
                           true,
                           !opt.isDrawNumberBackgroundEnabled());

            dx += 4;    // spacing between small digits
        }
    }
}


// -------------------- BMP bit-depth helpers --------------------

// Read original bit-depth directly from BMP header (offset 28)
uint16_t DrawCommand::readBmpBpp(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in)
        throw SpriteUtilsException("Failed to open BMP for bit-depth check: " + file.string());

    unsigned char header[54] = {0};
    in.read(reinterpret_cast<char*>(header), 54);
    if (in.gcount() < 54)
        throw SpriteUtilsException("Invalid BMP header: " + file.string());

    if (header[0] != 'B' || header[1] != 'M')
        throw SpriteUtilsException("Not a BMP file: " + file.string());

    // biBitCount at offset 28 (little-endian)
    uint16_t bpp = *reinterpret_cast<uint16_t*>(&header[28]);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    bpp = (bpp >> 8) | (bpp << 8);
#endif
    return bpp;
}

// Convert BGR8 → 16bpp (RGB565) and write BMP with manual headers
void DrawCommand::writeBmp16BGR565(const std::filesystem::path& out, const cv::Mat& bgr8) {
    if (bgr8.empty() || bgr8.type() != CV_8UC3)
        throw SpriteUtilsException("writeBmp16BGR565 expects CV_8UC3 image");

    const int width  = bgr8.cols;
    const int height = bgr8.rows;
    const int rowStrideOut = ((width * 2 + 3) / 4) * 4; // 2 bytes/pixel, 4-byte aligned per BMP spec

    std::vector<uint8_t> pixelData(rowStrideOut * height);

    // BMP stores rows bottom-up
    for (int y = 0; y < height; ++y) {
        const cv::Vec3b* src = bgr8.ptr<cv::Vec3b>(height - 1 - y);
        uint16_t* dst = reinterpret_cast<uint16_t*>(&pixelData[y * rowStrideOut]);
        for (int x = 0; x < width; ++x) {
            uint8_t B = src[x][0];
            uint8_t G = src[x][1];
            uint8_t R = src[x][2];
            uint16_t b5 = (B >> 3) & 0x1F;
            uint16_t g6 = (G >> 2) & 0x3F;
            uint16_t r5 = (R >> 3) & 0x1F;
            uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
            dst[x] = rgb565;
        }
    }

    const uint32_t fileHeaderSize   = 14;
    const uint32_t infoHeaderSize   = 40;
    const uint32_t bitfieldsSize    = 12;
    const uint32_t pixelDataOffset  = fileHeaderSize + infoHeaderSize + bitfieldsSize;
    const uint32_t fileSize         = pixelDataOffset + (uint32_t)pixelData.size();

    std::ofstream f(out, std::ios::binary);
    if (!f) throw SpriteUtilsException("Cannot write BMP: " + out.string());

    // BITMAPFILEHEADER
    f.put('B'); f.put('M');
    uint32_t u32 = fileSize; f.write(reinterpret_cast<char*>(&u32), 4);
    uint16_t u16 = 0;        f.write(reinterpret_cast<char*>(&u16), 2);
    u16 = 0;                 f.write(reinterpret_cast<char*>(&u16), 2);
    u32 = pixelDataOffset;   f.write(reinterpret_cast<char*>(&u32), 4);

    // BITMAPINFOHEADER (40 B)
    u32 = infoHeaderSize;    f.write(reinterpret_cast<char*>(&u32), 4);
    u32 = width;             f.write(reinterpret_cast<char*>(&u32), 4);
    u32 = height;            f.write(reinterpret_cast<char*>(&u32), 4);
    u16 = 1;                 f.write(reinterpret_cast<char*>(&u16), 2); // planes
    u16 = 16;                f.write(reinterpret_cast<char*>(&u16), 2); // bpp
    u32 = 3;                 f.write(reinterpret_cast<char*>(&u32), 4); // BI_BITFIELDS  (!!!)
    u32 = (uint32_t)pixelData.size(); f.write(reinterpret_cast<char*>(&u32), 4);
    u32 = 2835;              f.write(reinterpret_cast<char*>(&u32), 4); // ppm X
    u32 = 2835;              f.write(reinterpret_cast<char*>(&u32), 4); // ppm Y
    u32 = 0;                 f.write(reinterpret_cast<char*>(&u32), 4); // colors used
    u32 = 0;                 f.write(reinterpret_cast<char*>(&u32), 4); // important colors

    uint32_t rMask = 0xF800;
    uint32_t gMask = 0x07E0;
    uint32_t bMask = 0x001F;
    f.write(reinterpret_cast<char*>(&rMask), 4);
    f.write(reinterpret_cast<char*>(&gMask), 4);
    f.write(reinterpret_cast<char*>(&bMask), 4);

    // pixel data
    f.write(reinterpret_cast<const char*>(pixelData.data()), pixelData.size());
}

// -------------------- run() --------------------

std::string DrawCommand::run(const SpriteUtilsArgs& args) {
    SpriteUtilsOptions opt(args);

    const std::filesystem::path workingDir(opt.getWorkingDirectory());
    std::cout << "Going to process images in directory: " << workingDir << "\n";

    // Load the spritesheet CSV
    SpriteSheet spriteSheet(std::filesystem::path(opt.getSpriteSheetPath()));

    // Iterate all files in the working directory
    for (const auto& entry : std::filesystem::directory_iterator(workingDir)) {
        if (!entry.is_regular_file())
            continue;

        auto imageFile = entry.path();
        if (opt.getFileName().has_value() &&
            opt.getFileName().value() != imageFile.filename().string())
        {
            continue;
        }

        if (imageFile.filename().string().ends_with(".backup"))
            continue;

        // Backup logic identical to Java:
        std::filesystem::path backup = imageFile; backup += ".backup";
        try {
            if (std::filesystem::exists(backup)) {
                std::filesystem::remove(imageFile);
                Utils::copyFile(backup, imageFile);
            } else {
                Utils::copyFile(imageFile, backup);
            }
        } catch (const SpriteUtilsException& e) {
            throw SpriteUtilsException(
                std::string("Error managing backup files: ") + e.what());
        }

        // Detect the original bit-depth
        const uint16_t bpp = readBmpBpp(imageFile);

        // Load the image into OpenCV (as BGR8)
        cv::Mat img = cv::imread(imageFile.string(), cv::IMREAD_UNCHANGED);
        if (img.empty())
            throw SpriteUtilsException("Reading image failed: " + imageFile.string());

        // if grayscale (1 channel), convert to BGR
        if (img.channels() == 1)
            cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);


        // Get the rows belonging to this image
        auto rows = spriteSheet.getSpriteSheetRows(imageFile.filename().string());
        if (rows.empty()) {
            if (opt.getFileName().has_value() &&
                opt.getFileName().value() == imageFile.filename().string())
            {
                break;
            }
            continue;
        }

        // Filter by --row if specified
        std::optional<int> onlyRow = opt.getRow();
        std::vector<SpriteSheetRow> todo;
        todo.reserve(rows.size());
        for (const auto& r : rows) {
            if (!onlyRow.has_value() || onlyRow.value() == r.row) {
                todo.push_back(r);
            }
        }

        // Draw all required rows
        for (const auto& row : todo) {
            bool doubleSize = opt.isNumberDoubleSized();
            if (doubleSize && row.width < 23) {
                doubleSize = false;
            }

            if (opt.isDrawNumberEnabled()) {
                int endX = row.x + row.width  - 1;
                int endY = row.y + row.height - 1;
                drawNumber(img, row.numberPerSheet, endX - 2, endY - 1, doubleSize, opt);
            }

            // Dashed rectangle
            cv::Scalar col = toScalar(opt.getRectangleColor());
            cv::Rect rc(row.x, row.y, std::max(1, row.width), std::max(1, row.height));
            drawDashedRect(img, rc, col);
        }

        // Save back preserving original BPP
        try {
            std::string ext = imageFile.extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            // Always write BLP as RGB565, same as original Java
            if (ext == ".blp") {
                writeBmp16BGR565(imageFile, img);
            }
            // BMP 16-bit → keep RGB565
            else if (bpp == 16) {
                writeBmp16BGR565(imageFile, img);
            }
            else if (bpp == 8) {
                if (!cv::imwrite(imageFile.string(), img)) {
                    throw SpriteUtilsException("Could not write 24bpp BMP: " + imageFile.string());
                }
            }

            // Any other image type → let OpenCV handle it
            else {
                if (!cv::imwrite(imageFile.string(), img)) {
                    throw SpriteUtilsException("OpenCV could not write this type: " + imageFile.string());
                }
            }

        } catch (const std::exception& e) {
            throw SpriteUtilsException(
                std::string("Writing image failed: ") + e.what());
        }

        // If only one file was requested, stop after finishing it
        if (opt.getFileName().has_value() &&
            opt.getFileName().value() == imageFile.filename().string())
        {
            break;
        }
    }

    return "";
}
