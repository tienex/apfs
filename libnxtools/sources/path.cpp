/*
 * Copyright (c) 2017-present Orlando Bassotto
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nxtools/path.h"
#include "nxtools/string.h"

#include <cstring>

#ifdef _WIN32
static inline bool
is_device_name(std::string const &s, char const *devname)
{
    return (strlen(devname) == s.length() &&
            stricmp(s.c_str(), devname) == 0);
}

std::string nxtools::
fix_windows_filename(std::string const &name)
{
    std::string result;
    for (size_t n = 0; n < name.length(); n++) {
        switch (name[n]) {
            case ':':
            case '\\':
            case '?':
            case '<':
            case '>':
            case '"':
            case '/':
            case '|':
                result += '_';
                break;
            default:
                result += name[n];
        }
    }

    if (is_device_name(result, "con") ||
        is_device_name(result, "prn") ||
        is_device_name(result, "aux") ||
        is_device_name(result, "nul") ||
        is_device_name(result, "com1") ||
        is_device_name(result, "com2") ||
        is_device_name(result, "com3") ||
        is_device_name(result, "com4") ||
        is_device_name(result, "com5") ||
        is_device_name(result, "com6") ||
        is_device_name(result, "com7") ||
        is_device_name(result, "com8") ||
        is_device_name(result, "com9") ||
        is_device_name(result, "lpt1") ||
        is_device_name(result, "lpt2") ||
        is_device_name(result, "lpt3") ||
        is_device_name(result, "lpt4") ||
        is_device_name(result, "lpt5") ||
        is_device_name(result, "lpt6") ||
        is_device_name(result, "lpt7") ||
        is_device_name(result, "lpt8") ||
        is_device_name(result, "lpt9")) {
            result += "_";
    }

    return result;
}
#endif

std::string nxtools::
normalize_path(std::string const &path)
{
    if (path.empty())
        return "/";

    auto tokens = split(path, '/');
    if (tokens.empty())
        return "/";

    std::vector <std::string> result;
    for (auto &token : tokens) {
        if (token.empty() || token == ".")
            continue;
        if (token == "..") {
            if (!result.empty()) {
                result.pop_back();
            }
            continue;
        }
        result.push_back(token);
    }

    auto normalized = join(result, "/");
    if (normalized.empty())
        return "/";

    return normalized;
}
