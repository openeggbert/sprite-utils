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

#include "RestoreCommand.h"

#include <filesystem>
#include <iostream>

#include "SpriteUtilsOptions.h"
#include "Utils.h"

namespace {
constexpr const char* BACKUP_SUFFIX = ".backup";
}

std::string RestoreCommand::run(const SpriteUtilsArgs& args) {
    SpriteUtilsOptions opt(args);

    const std::filesystem::path workingDir(opt.getWorkingDirectory());
    std::cout << "Restoring images from backups in directory: " << workingDir << "\n";

    int restoredCount = 0;

    for (const auto& entry : std::filesystem::directory_iterator(workingDir)) {
        if (!entry.is_regular_file())
            continue;

        const std::filesystem::path backupFile = entry.path();
        const std::string backupFileName = backupFile.filename().string();

        if (!backupFileName.ends_with(BACKUP_SUFFIX))
            continue;

        std::filesystem::path originalFile = backupFile;
        originalFile.replace_filename(
            backupFileName.substr(0, backupFileName.size() - std::string(BACKUP_SUFFIX).size()));

        if (opt.getFileName().has_value() &&
            opt.getFileName().value() != originalFile.filename().string())
        {
            continue;
        }

        std::cout << "Restoring " << originalFile.filename().string()
                   << " from " << backupFileName << "\n";

        if (std::filesystem::exists(originalFile))
            std::filesystem::remove(originalFile);
        Utils::copyFile(backupFile, originalFile);

        ++restoredCount;
    }

    std::cout << "Restored " << restoredCount << " file(s).\n";
    return "";
}
