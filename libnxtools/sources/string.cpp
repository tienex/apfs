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

#include "nxtools/string.h"

#include <cctype>

#include <algorithm>
#ifdef HAVE_CODECVT
#include <codecvt>
#endif
#include <locale>
#include <numeric>

#ifndef HAVE_CODECVT
#include "nxcompat/nxcompat.h"
#endif

std::vector<std::string> nxtools::
split(std::string const &s, char sep)
{
    if (s.empty())
        return std::vector<std::string>();

    size_t start = 0, end = 0;
    std::vector<std::string> tokens;
    while ((end = s.find(sep, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

std::string nxtools::
join(std::vector<std::string> const &strings, std::string const &sep)
{
    return std::accumulate(strings.begin(), strings.end(), std::string(),
            [&sep](std::string const &result, std::string const &value)
            { return result + sep + value; });
}

std::string nxtools::
to_lower(std::string const &string)
{
    std::string result = string;
    std::transform(result.begin(), result.end(), result.begin(),
            [](char c) { return std::tolower(c); });
    return result;
}

#ifndef HAVE_CODECVT
namespace {

template <size_t WCharSize>
static wchar_t *widen_internal(UTF8 const *, size_t);

template <>
wchar_t *widen_internal<4>(UTF8 const *s, size_t len)
{
    UTF8 const *source;
    UTF8 const *sourceEnd;
    UTF32      *target;
    UTF32      *targetEnd;
    UTF32      *result;

    result = new UTF32[len + 1];

    source    = (UTF8 const *)s;
    sourceEnd = (UTF8 const *)(s + len);
    target    = result;
    targetEnd = result + len;

    if (ConvertUTF8toUTF32(&source, sourceEnd, &target, targetEnd,
                lenientConversion) != conversionOK) {
        delete[] result;
        result = nullptr;
    } else {
        *target = 0;
    }

    return reinterpret_cast<wchar_t *>(result);
}

template <>
wchar_t *widen_internal<2>(UTF8 const *s, size_t len)
{
    UTF8 const *source;
    UTF8 const *sourceEnd;
    UTF16      *target;
    UTF16      *targetEnd;
    UTF16      *result;

    result = new UTF16[len + 1];

    source    = (UTF8 const *)s;
    sourceEnd = (UTF8 const *)(s + len);
    target    = result;
    targetEnd = result + len;

    if (ConvertUTF8toUTF16(&source, sourceEnd, &target, targetEnd,
                lenientConversion) != conversionOK) {
        delete[] result;
        result = nullptr;
    } else {
        *target = 0;
    }

    return reinterpret_cast<wchar_t *>(result);
}

}
#endif

std::wstring nxtools::
widen(std::string const &string)
{
    if (string.empty())
        return std::wstring();

#ifdef HAVE_CODECVT
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(string);
#else
    auto wstr = widen_internal<sizeof(wchar_t)>(
            reinterpret_cast<UTF8 const *>(string.c_str()),
            string.length());
    if (wstr == nullptr)
        return std::wstring();

    std::wstring result(wstr);
    delete[] wstr;

    return result;
#endif
}
