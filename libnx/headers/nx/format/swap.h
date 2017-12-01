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

#ifndef __nx_format_swap_h
#define __nx_format_swap_h

#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || \
    (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define nx_swap16(x) (x)
#define nx_swap32(x) (x)
#define nx_swap64(x) (x)
#elif defined(__GNUC__)
#define nx_swap16(x) __builtin_bswap16(x)
#define nx_swap32(x) __builtin_bswap32(x)
#define nx_swap64(x) __builtin_bswap64(x)
#elif defined(_MSC_VER)
#ifdef __cplusplus
extern "C" {
#endif
unsigned short __cdecl _byteswap_ushort(unsigned short);
unsigned long  __cdecl _byteswap_ulong(unsigned long);
unsigned __int64 __cdecl _byteswap_uint64(unsigned __int64);
#ifdef __cplusplus
}
#endif
#pragma intrinsic(_byteswap_ushort)
#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_byteswap_uint64)
#define nx_swap16(x) _byteswap_ushort(x)
#define nx_swap32(x) _byteswap_ulong(x)
#define nx_swap64(x) _byteswap_uint64(x)
#else
#error "Don't know how to swap bytes"
#endif

#endif  /* !__nx_format_swap_h */
