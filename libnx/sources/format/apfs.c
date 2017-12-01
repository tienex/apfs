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

#include "nx/format/apfs.h"

#include "nxcompat/nxcompat.h"

#include <ctype.h>
#include <stdlib.h>

char *
apfs_format_time(uint64_t timestamp, char *buf, size_t bufsiz, bool iso)
{
    time_t t;
    size_t len;
    uint64_t seconds;
    uint32_t nseconds;

    seconds = timestamp / 1000000000;
    nseconds = timestamp % 1000000000;

    t = (time_t)seconds;
    strftime(buf, bufsiz, "%Y-%m-%d %H:%M:%S",
            iso ? gmtime(&t) : localtime(&t));
    len = strlen(buf);
    snprintf(buf + len, bufsiz - len, ".%09u%s", nseconds, iso ? "Z" : "");
    if (iso) {
        *strchr(buf, ' ') = 'T';
    }

    return buf;
}

/* hacker's delight */
#define CRC32C_POLY 0x82F63B78U /* CRC32-C */

static uint32_t *
utf8_to_utf32(char const *s, size_t len, size_t *ulen)
{
    UTF8 const *source;
    UTF8 const *sourceEnd;
    UTF32      *target;
    UTF32      *targetEnd;
    UTF32      *result;

    result = (UTF32 *)calloc((len + 1), sizeof(UTF32));
    if (result == NULL)
        return NULL;

    source    = (UTF8 const *)s;
    sourceEnd = (UTF8 const *)(s + len);
    target    = result;
    targetEnd = result + len;

    if (ConvertUTF8toUTF32(&source, sourceEnd, &target, targetEnd,
                lenientConversion) != conversionOK) {
        free(result);
        result = NULL;
    } else {
        *ulen = target - result;
    }

    return (uint32_t *)result;
}

static inline int
tolower_if_insensitive(int c, bool insensitive)
{
    return insensitive ? tolower(c) : c;
}

static uint32_t
crc32c(uint32_t crc, void const *data, size_t length, bool insensitive)
{
    uint8_t const *bytes = (uint8_t const *)data;

    while (length-- != 0) {
        size_t   bits = 8;
        uint32_t byte = tolower_if_insensitive(*bytes++, insensitive);

        crc = crc ^ byte;

        while (bits-- != 0) {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (CRC32C_POLY & mask);
        }
    }

    return crc;
}

uint32_t
apfs_hash_name(char const *name, size_t namelen, bool insensitive)
{
    size_t    utf32_length;
    uint32_t *utf32_name = utf8_to_utf32(name, namelen, &utf32_length);
    uint32_t  hash       = ~0U;

    if (utf32_name != NULL) {
        hash = crc32c(hash, utf32_name, utf32_length * sizeof(uint32_t),
                insensitive);
        free(utf32_name);
    }

    return hash & 0x3fffff;
}
