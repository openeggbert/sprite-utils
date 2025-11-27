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

#include "SpriteUtilsArgs.h"

SpriteUtilsArgs::SpriteUtilsArgs(std::vector<std::string>& args)
{
    command = args.empty() ? "draw" : args[0];

    if (args.size() > 1) {
        for (size_t i = 1; i < args.size(); i++) {
            std::string key = args[i];

            if (!key.starts_with("--")) {
                throw SpriteUtilsException("Invalid option (does not start with --): " + key);
            }

            if (args.size() < (i + 2)) {
                throw SpriteUtilsException("Missing value for option: " + key);
            }

            ++i;
            const std::string& value = args[i];

            if (value.starts_with("--")) {
                throw SpriteUtilsException("Invalid value (starts with --): " + key);
            }

            internalMap[key] = value;
            std::cout << "Found arg: " << key << "=" << internalMap[key] << std::endl;
        }

        for (const auto& e : internalMap) {
            std::cout << "Found argument: " << e.first << " = " << e.second << "\n";
        }
    }
}

bool SpriteUtilsArgs::hasArgument(const std::string& arg) const
{
#if __cpp_lib_contains // C++20 way
    return internalMap.contains(arg);
#else
    return internalMap.find(arg) != internalMap.end();
#endif
}

void SpriteUtilsArgs::addArgument(const std::string& arg, const std::string& value)
{
    internalMap[arg] = value;
}

std::string& SpriteUtilsArgs::getArgument(const std::string& arg)
{
    return internalMap[arg];
}

std::optional<std::string> SpriteUtilsArgs::getArgumentOptional(const std::string& arg) const
{
    auto it = internalMap.find(arg);
    if (it != internalMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool SpriteUtilsArgs::getBooleanArgument(const std::string& arg, bool default_) const
{
    return hasArgument(arg) ? (internalMap.at(arg) == "true") : default_;
}

bool SpriteUtilsArgs::isVerboseLoggingEnabled()
{
    return hasArgument("verbose") && getArgument("verbose") == "true";
}

const std::string& SpriteUtilsArgs::get_command() const
{
    return command;
}
