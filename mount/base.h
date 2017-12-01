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

#ifndef __apfs_fuse_base_h
#define __apfs_fuse_base_h

#include "apfs/apfs.h"

#include <cstring>

namespace apfs_fuse {

class volume;
typedef std::map<std::string, volume *> nx_volume_map;

// The mounted volume.
extern volume *the_volume;

extern bool novolicon_unsupported;
extern bool expose_resource_fork;
extern bool expose_xattr_directory;

#ifdef _WIN32
#define XATTR_DIRECTORY          "$$XATTR"
#define XATTR_DIRECTORY_LEN      7
#define XATTR_ROOT_DIRECTORY     "$$ROOT"
#define XATTR_ROOT_DIRECTORY_LEN 6
#else
#define XATTR_DIRECTORY          "..xattr"
#define XATTR_DIRECTORY_LEN      7
#define XATTR_ROOT_DIRECTORY     "..root"
#define XATTR_ROOT_DIRECTORY_LEN 6
#endif

static inline bool has_rsrc_prefix(std::string const &name)
{ return strncmp(name.c_str(), "._", 2) == 0; }
static inline std::string prefix_rsrc_name(std::string const &name)
{ return "._" + name; }
static inline bool unprefix_rsrc_name(std::string &name)
{
    if (has_rsrc_prefix(name)) {
        name = name.substr(2);
        return true;
    }
    return false;
}

char const *extract_volume_icon(char const *progname);
int main(std::vector<char const *> const &args);

}

#endif  // !__apfs_fuse_base_h
