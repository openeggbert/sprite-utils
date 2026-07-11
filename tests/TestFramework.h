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

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Minimal, dependency-free test harness (no external test framework is
// vendored in this project). Each TEST_CASE registers a runner function;
// run_all_tests() executes them all and returns the number of failures.

namespace sptest {

inline int& failureCount() {
    static int count = 0;
    return count;
}

inline int& checkCount() {
    static int count = 0;
    return count;
}

using TestFn = void (*)();

struct TestCase {
    const char* name;
    TestFn fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

struct Registrar {
    Registrar(const char* name, TestFn fn) {
        registry().push_back({name, fn});
    }
};

inline void reportFailure(const std::string& expr, const char* file, int line) {
    ++failureCount();
    std::cerr << "  FAIL: " << expr << " (" << file << ":" << line << ")\n";
}

inline int runAll() {
    int total = static_cast<int>(registry().size());
    int i = 0;
    for (const auto& tc : registry()) {
        ++i;
        std::cout << "[" << i << "/" << total << "] " << tc.name << "\n";
        tc.fn();
    }
    std::cout << "\n" << checkCount() << " check(s), " << failureCount() << " failure(s)\n";
    return failureCount();
}

} // namespace sptest

#define SP_TEST_CASE(name) \
    static void name(); \
    static sptest::Registrar registrar_##name(#name, &name); \
    static void name()

#define SP_CHECK(cond) \
    do { \
        ++sptest::checkCount(); \
        if (!(cond)) { \
            sptest::reportFailure(#cond, __FILE__, __LINE__); \
        } \
    } while (0)

#define SP_CHECK_EQ(a, b) \
    do { \
        ++sptest::checkCount(); \
        auto _a = (a); auto _b = (b); \
        if (!(_a == _b)) { \
            std::ostringstream _oss; \
            _oss << #a << " == " << #b << "  (got " << _a << " vs " << _b << ")"; \
            sptest::reportFailure(_oss.str(), __FILE__, __LINE__); \
        } \
    } while (0)

#define SP_CHECK_THROWS(expr) \
    do { \
        ++sptest::checkCount(); \
        bool _threw = false; \
        try { (expr); } catch (...) { _threw = true; } \
        if (!_threw) { \
            sptest::reportFailure(#expr " was expected to throw", __FILE__, __LINE__); \
        } \
    } while (0)

#define SP_CHECK_NO_THROW(expr) \
    do { \
        ++sptest::checkCount(); \
        try { (expr); } catch (const std::exception& e) { \
            std::ostringstream _oss; \
            _oss << #expr << " was not expected to throw, but threw: " << e.what(); \
            sptest::reportFailure(_oss.str(), __FILE__, __LINE__); \
        } catch (...) { \
            sptest::reportFailure(#expr " was not expected to throw", __FILE__, __LINE__); \
        } \
    } while (0)
