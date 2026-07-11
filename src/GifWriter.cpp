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

#include "GifWriter.h"

#include <algorithm>
#include <climits>
#include <fstream>
#include <unordered_map>

#include "SpriteUtilsException.h"

namespace {

uint32_t packColor(const cv::Vec3b& bgr) {
    return (static_cast<uint32_t>(bgr[2]) << 16) // R
         | (static_cast<uint32_t>(bgr[1]) << 8)  // G
         | static_cast<uint32_t>(bgr[0]);        // B
}

cv::Vec3b unpackColor(uint32_t rgb) {
    return cv::Vec3b(
        static_cast<uint8_t>(rgb & 0xFF),         // B
        static_cast<uint8_t>((rgb >> 8) & 0xFF),  // G
        static_cast<uint8_t>((rgb >> 16) & 0xFF)  // R
    );
}

// One median-cut box: the unique colors (as packed RGB) it currently owns,
// each with a pixel-frequency weight.
struct MedianCutBox {
    std::vector<std::pair<uint32_t, int>> colors; // (packedRgb, weight)

    int channelRange(int channel) const {
        int lo = 255, hi = 0;
        for (const auto& c : colors) {
            int v = (channel == 0) ? (int)((c.first >> 16) & 0xFF)
                  : (channel == 1) ? (int)((c.first >> 8) & 0xFF)
                                   : (int)(c.first & 0xFF);
            lo = std::min(lo, v);
            hi = std::max(hi, v);
        }
        return hi - lo;
    }

    int widestChannel() const {
        int rR = channelRange(0), rG = channelRange(1), rB = channelRange(2);
        if (rR >= rG && rR >= rB) return 0;
        if (rG >= rB) return 1;
        return 2;
    }

    cv::Vec3b weightedAverage() const {
        long long sr = 0, sg = 0, sb = 0, sw = 0;
        for (const auto& c : colors) {
            cv::Vec3b bgr = unpackColor(c.first);
            sr += (long long)bgr[2] * c.second;
            sg += (long long)bgr[1] * c.second;
            sb += (long long)bgr[0] * c.second;
            sw += c.second;
        }
        if (sw == 0) sw = 1;
        return cv::Vec3b(
            static_cast<uint8_t>(sb / sw),
            static_cast<uint8_t>(sg / sw),
            static_cast<uint8_t>(sr / sw));
    }
};

// Reduces `uniqueColors` (packedRgb -> pixel count) to at most `maxColors`
// representative RGB colors via median-cut.
std::vector<cv::Vec3b> medianCut(const std::unordered_map<uint32_t, int>& uniqueColors, int maxColors) {
    MedianCutBox initial;
    initial.colors.reserve(uniqueColors.size());
    for (const auto& kv : uniqueColors) {
        initial.colors.emplace_back(kv.first, kv.second);
    }

    std::vector<MedianCutBox> boxes{initial};

    while (static_cast<int>(boxes.size()) < maxColors) {
        // split the box with the most unique colors (simple, effective heuristic)
        int splitIdx = -1;
        size_t bestSize = 1; // boxes with a single color can't be usefully split
        for (size_t i = 0; i < boxes.size(); ++i) {
            if (boxes[i].colors.size() > bestSize) {
                bestSize = boxes[i].colors.size();
                splitIdx = static_cast<int>(i);
            }
        }
        if (splitIdx < 0) break; // nothing left worth splitting

        MedianCutBox& box = boxes[splitIdx];
        int channel = box.widestChannel();
        std::sort(box.colors.begin(), box.colors.end(),
            [channel](const auto& a, const auto& b) {
                auto extract = [channel](uint32_t c) {
                    return (channel == 0) ? (int)((c >> 16) & 0xFF)
                         : (channel == 1) ? (int)((c >> 8) & 0xFF)
                                          : (int)(c & 0xFF);
                };
                return extract(a.first) < extract(b.first);
            });

        size_t mid = box.colors.size() / 2;
        MedianCutBox lower, upper;
        lower.colors.assign(box.colors.begin(), box.colors.begin() + mid);
        upper.colors.assign(box.colors.begin() + mid, box.colors.end());

        boxes[splitIdx] = lower;
        boxes.push_back(upper);
    }

    std::vector<cv::Vec3b> palette;
    palette.reserve(boxes.size());
    for (const auto& box : boxes) {
        palette.push_back(box.weightedAverage());
    }
    return palette;
}

int nearestPaletteEntry(const cv::Vec3b& color, const std::vector<cv::Vec3b>& palette) {
    int best = 0;
    int bestDist = INT_MAX;
    for (size_t i = 0; i < palette.size(); ++i) {
        int db = (int)color[0] - (int)palette[i][0];
        int dg = (int)color[1] - (int)palette[i][1];
        int dr = (int)color[2] - (int)palette[i][2];
        int dist = db * db + dg * dg + dr * dr;
        if (dist < bestDist) {
            bestDist = dist;
            best = static_cast<int>(i);
        }
    }
    return best;
}

void writeUint16LE(std::ofstream& out, uint16_t v) {
    out.put(static_cast<char>(v & 0xFF));
    out.put(static_cast<char>((v >> 8) & 0xFF));
}

void writeSubBlocks(std::ofstream& out, const std::vector<uint8_t>& data) {
    size_t offset = 0;
    while (offset < data.size()) {
        size_t chunk = std::min<size_t>(255, data.size() - offset);
        out.put(static_cast<char>(chunk));
        out.write(reinterpret_cast<const char*>(data.data() + offset), static_cast<std::streamsize>(chunk));
        offset += chunk;
    }
    out.put(static_cast<char>(0)); // block terminator
}

} // namespace

int GifWriter::bitsNeededFor(int paletteSize) {
    int bits = 1;
    while ((1 << bits) < paletteSize) {
        ++bits;
    }
    return std::max(2, bits);
}

std::vector<cv::Vec3b> GifWriter::quantizeToRgbPalette(const cv::Mat& bgr, std::vector<uint8_t>& indexOut) {
    std::unordered_map<uint32_t, int> uniqueColors;
    uniqueColors.reserve(256);

    for (int y = 0; y < bgr.rows; ++y) {
        const cv::Vec3b* row = bgr.ptr<cv::Vec3b>(y);
        for (int x = 0; x < bgr.cols; ++x) {
            ++uniqueColors[packColor(row[x])];
        }
    }

    std::vector<cv::Vec3b> palette;
    std::unordered_map<uint32_t, int> colorToPaletteIndex;

    if (static_cast<int>(uniqueColors.size()) <= 256) {
        palette.reserve(uniqueColors.size());
        colorToPaletteIndex.reserve(uniqueColors.size());
        for (const auto& kv : uniqueColors) {
            colorToPaletteIndex[kv.first] = static_cast<int>(palette.size());
            palette.push_back(unpackColor(kv.first));
        }
    } else {
        palette = medianCut(uniqueColors, 256);
        colorToPaletteIndex.reserve(uniqueColors.size());
        for (const auto& kv : uniqueColors) {
            cv::Vec3b rgb = unpackColor(kv.first);
            colorToPaletteIndex[kv.first] = nearestPaletteEntry(rgb, palette);
        }
    }

    indexOut.resize(static_cast<size_t>(bgr.rows) * static_cast<size_t>(bgr.cols));
    size_t i = 0;
    for (int y = 0; y < bgr.rows; ++y) {
        const cv::Vec3b* row = bgr.ptr<cv::Vec3b>(y);
        for (int x = 0; x < bgr.cols; ++x) {
            indexOut[i++] = static_cast<uint8_t>(colorToPaletteIndex.at(packColor(row[x])));
        }
    }

    // `palette` was built via unpackColor()/weightedAverage(), both of which
    // return BGR-ordered Vec3b (matching OpenCV's native pixel layout, same
    // as the `bgr` input). GIF color tables are stored as literal R,G,B byte
    // triplets, so convert order here, once, at the boundary - the rest of
    // this function (packColor/unpackColor/nearestPaletteEntry) only needs
    // internal consistency, not any particular channel order.
    std::vector<cv::Vec3b> rgbPalette;
    rgbPalette.reserve(palette.size());
    for (const auto& bgrColor : palette) {
        rgbPalette.emplace_back(bgrColor[2], bgrColor[1], bgrColor[0]);
    }
    return rgbPalette;
}

std::vector<uint8_t> GifWriter::lzwEncode(const std::vector<uint8_t>& indices, int minCodeSize) {
    const int clearCode = 1 << minCodeSize;
    const int endCode = clearCode + 1;
    const int maxCode = 4096;

    std::vector<uint8_t> out;
    uint32_t bitBuffer = 0;
    int bitCount = 0;

    auto emit = [&](int code, int codeSize) {
        bitBuffer |= (static_cast<uint32_t>(code) << bitCount);
        bitCount += codeSize;
        while (bitCount >= 8) {
            out.push_back(static_cast<uint8_t>(bitBuffer & 0xFF));
            bitBuffer >>= 8;
            bitCount -= 8;
        }
    };

    std::unordered_map<uint32_t, int> dict; // (prefixCode<<8 | nextByte) -> code
    int nextCode = endCode + 1;
    int codeSize = minCodeSize + 1;

    auto resetDict = [&]() {
        dict.clear();
        nextCode = endCode + 1;
        codeSize = minCodeSize + 1;
    };

    emit(clearCode, codeSize);

    if (!indices.empty()) {
        int prefixCode = indices[0]; // single-symbol code == its own index value

        for (size_t i = 1; i < indices.size(); ++i) {
            int k = indices[i];
            uint32_t key = (static_cast<uint32_t>(prefixCode) << 8) | static_cast<uint32_t>(k);
            auto it = dict.find(key);
            if (it != dict.end()) {
                prefixCode = it->second;
                continue;
            }

            emit(prefixCode, codeSize);

            if (nextCode < maxCode) {
                const int assignedCode = nextCode;
                dict[key] = assignedCode;
                ++nextCode;
                // Grow the code width exactly when the code we just handed
                // out is the first one that no longer fits in `codeSize`
                // bits (i.e. assignedCode == 2^codeSize) - not one entry
                // earlier, or a standards-compliant decoder reads codes at
                // the wrong bit boundaries from here on.
                if (assignedCode == (1 << codeSize) && codeSize < 12) {
                    ++codeSize;
                }
            } else {
                emit(clearCode, codeSize);
                resetDict();
            }
            prefixCode = k;
        }

        emit(prefixCode, codeSize);
    }

    emit(endCode, codeSize);

    if (bitCount > 0) {
        out.push_back(static_cast<uint8_t>(bitBuffer & 0xFF));
    }

    return out;
}

void GifWriter::writeAnimatedGif(
    const std::filesystem::path& outFile,
    const std::vector<cv::Mat>& framesBgr,
    int frameDelayMs,
    const cv::Vec3b& backgroundColorBgr)
{
    if (framesBgr.empty())
        throw SpriteUtilsException("Cannot write an animated GIF with zero frames: " + outFile.string());

    int canvasWidth = 0, canvasHeight = 0;
    for (const auto& f : framesBgr) {
        canvasWidth = std::max(canvasWidth, f.cols);
        canvasHeight = std::max(canvasHeight, f.rows);
    }
    if (canvasWidth <= 0 || canvasHeight <= 0 || canvasWidth > 0xFFFF || canvasHeight > 0xFFFF)
        throw SpriteUtilsException("Invalid GIF canvas size for: " + outFile.string());

    // GIF delay is stored in hundredths of a second; round to nearest tick,
    // minimum 1 (0 is technically legal but renders as "as fast as possible"
    // in most viewers, which is rarely what's wanted for a sprite preview).
    int delayTicks = std::max(1, static_cast<int>((frameDelayMs + 5) / 10));

    std::ofstream out(outFile, std::ios::binary);
    if (!out.is_open())
        throw SpriteUtilsException("Cannot write GIF: " + outFile.string());

    // --- Header ---
    out.write("GIF89a", 6);

    // --- Logical Screen Descriptor (no global color table) ---
    writeUint16LE(out, static_cast<uint16_t>(canvasWidth));
    writeUint16LE(out, static_cast<uint16_t>(canvasHeight));
    out.put(static_cast<char>(0x00)); // packed: no GCT, color resolution/sort bits unused
    out.put(static_cast<char>(0x00)); // background color index (no GCT, so unused)
    out.put(static_cast<char>(0x00)); // pixel aspect ratio

    // --- Application Extension: NETSCAPE2.0, loop forever ---
    out.put(static_cast<char>(0x21)); // extension introducer
    out.put(static_cast<char>(0xFF)); // application extension label
    out.put(static_cast<char>(0x0B)); // block size (11)
    out.write("NETSCAPE2.0", 11);
    out.put(static_cast<char>(0x03)); // sub-block size
    out.put(static_cast<char>(0x01)); // sub-block ID
    writeUint16LE(out, 0);            // loop count: 0 = infinite
    out.put(static_cast<char>(0x00)); // block terminator

    for (const auto& frame : framesBgr) {
        cv::Mat canvas(canvasHeight, canvasWidth, CV_8UC3,
            cv::Scalar(backgroundColorBgr[0], backgroundColorBgr[1], backgroundColorBgr[2]));
        frame.copyTo(canvas(cv::Rect(0, 0, frame.cols, frame.rows)));

        std::vector<uint8_t> indices;
        std::vector<cv::Vec3b> paletteRgb = quantizeToRgbPalette(canvas, indices);

        const int minCodeSize = bitsNeededFor(static_cast<int>(paletteRgb.size()));
        const int tableSize = 1 << minCodeSize;

        // --- Graphic Control Extension ---
        out.put(static_cast<char>(0x21));
        out.put(static_cast<char>(0xF9));
        out.put(static_cast<char>(0x04)); // block size
        out.put(static_cast<char>(0x00)); // disposal: unspecified, no transparency
        writeUint16LE(out, static_cast<uint16_t>(delayTicks));
        out.put(static_cast<char>(0x00)); // transparent color index (unused)
        out.put(static_cast<char>(0x00)); // block terminator

        // --- Image Descriptor ---
        out.put(static_cast<char>(0x2C)); // image separator
        writeUint16LE(out, 0); // left
        writeUint16LE(out, 0); // top
        writeUint16LE(out, static_cast<uint16_t>(canvasWidth));
        writeUint16LE(out, static_cast<uint16_t>(canvasHeight));
        // packed: local color table present, size = minCodeSize-1
        out.put(static_cast<char>(0x80 | (minCodeSize - 1)));

        // --- Local Color Table (padded to a power-of-two size with black) ---
        for (int i = 0; i < tableSize; ++i) {
            if (i < static_cast<int>(paletteRgb.size())) {
                out.put(static_cast<char>(paletteRgb[i][0]));
                out.put(static_cast<char>(paletteRgb[i][1]));
                out.put(static_cast<char>(paletteRgb[i][2]));
            } else {
                out.put(static_cast<char>(0));
                out.put(static_cast<char>(0));
                out.put(static_cast<char>(0));
            }
        }

        // --- Image Data ---
        out.put(static_cast<char>(minCodeSize));
        std::vector<uint8_t> compressed = lzwEncode(indices, minCodeSize);
        writeSubBlocks(out, compressed);
    }

    // --- Trailer ---
    out.put(static_cast<char>(0x3B));

    if (!out)
        throw SpriteUtilsException("Writing GIF failed: " + outFile.string());
}
