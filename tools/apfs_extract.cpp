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

#include "apfs/apfs.h"

#include "nxcompat/nxcompat.h"
#include "nxtools/path.h"
#include "nxtools/time.h"

#ifdef __APPLE__
#include <sys/acl.h>
#endif

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#define fix_file_name(x) fix_windows_filename(x)
#else
#define fix_file_name(x) x
#endif

#define COPY_BLOCK_SIZE (16 * 1024 * 1024)

static void
update_status(double r, std::string const &name, bool end = false)
{
    static char const *fill  = "####################";
    static char const *empty = "....................";
    static size_t len = 0;
    size_t off;

    if (len == 0) {
        len = strlen(fill);
    }

    off = static_cast<size_t>(r * len);
    printf("\rExtracting '%s' [%.*s%.*s] %.2f%%%s", name.c_str(),
            static_cast<int>(off), fill,
            static_cast<int>(len - off), empty,
            r * 100.0, end ? "\n" : "");
    fflush(stdout);
}

typedef std::vector<uint8_t> byte_vector;

static bool
xwrite(std::string const &filename, byte_vector const &content, int mode)
{
    int fd = open(filename.c_str(), O_CREAT|O_WRONLY|O_BINARY, mode & ~0111);
    if (fd < 0)
        return -1;

    bool success = write(fd, &content[0], content.size()) == content.size();
    close(fd);

    return success;
}

static bool
read_xattr(apfs::object const *o, std::string xname, byte_vector &content)
{
    content.resize(o->get_xattr_size(xname));
    return (o->read_xattr(xname, &content[0], content.size(), 0) ==
            content.size());
}

static void
apply_xattrs(int fd, apfs::object const *o, std::string const &file_name,
        uint16_t mode)
{
    byte_vector         content;
    apfs::string_vector xattrs;

    o->get_xattrs(xattrs);

    for (auto const &xname : xattrs) {
        bool noxattr = false;
        bool success = false;

        printf("Applying xattr '%s'...", xname.c_str());
        fflush(stdout);

        if (!read_xattr(o, xname, content)) {
            printf("error reading extended attribute: %s\n", strerror(errno));
            continue;
        }

#ifdef __APPLE__
        if (xname == APFS_XATTR_NAME_SECURITY) {
                //
                // Set the ACLs
                //
                auto acl = acl_copy_int(&content[0]);
                if (acl != nullptr) {
                    noxattr = true;
                    success = (acl_set_fd(fd, acl) == 0);
                    acl_free(acl);
                }
            }
#endif

        if (!noxattr) {
            success = (fsetxattr_nx(fd, xname.c_str(),
                        &content[0], content.size(), XATTR_CREATE) == 0);
            if (!success) {
                if (errno == EEXIST) {
                    success = (fsetxattr_nx(fd, xname.c_str(), &content[0],
                                content.size(), XATTR_REPLACE) == 0);
                } else if (errno == ENOTSUP || errno == E2BIG) {
                    //
                    // When xattrs are not supported or too big,
                    // fall back to writing them to the filesystem.
                    //
                    std::string xname_path;

                    if (xname == APFS_XATTR_NAME_RESOURCEFORK) {
                        xname_path = "._" + file_name;
                    } else {
#ifdef _WIN32
                        xname_path = "$XATTR";
                        mkdir(xname_path.c_str());
                        xname_path += "\\" + file_name;
                        mkdir(xname_path.c_str());
                        xname_path += "\\" + xname;
#else
                        xname_path = "..xattr";
                        mkdir(xname_path.c_str(), mode & 0777);
                        xname_path += "/" + file_name;
                        mkdir(xname_path.c_str(), mode & 0777);
                        xname_path += "/" + xname;
#endif
                    }

                    printf("writing to '%s'... ", xname_path.c_str());
                    success = xwrite(xname_path, content, mode);
                }
            }
        }

        if (!success) {
            printf("error setting extended attribute: %s\n", strerror(errno));
        } else {
            printf("success.\n");
        }
    }
}

static bool
extract(apfs::object const *o)
{
    apfs::object::info  info;
    std::string         file_name;
    int                 fd    = -1;
    uint8_t            *block = nullptr;

    o->get_info(info);

    struct timespec tv[3] = {
        nxtools::nx_timespec_to_timespec(info.atim),
        nxtools::nx_timespec_to_timespec(info.mtim),
        // this last one is really for windows compat layer.
        nxtools::nx_timespec_to_timespec(info.btim)
    };

    file_name = fix_file_name(o->get_name());

    if (o->is_symbolic_link()) {
        std::string target;

        o->read_symbolic_link(target);
        target = fix_file_name(target);

        update_status(0.0, file_name, false);

        if (symlink_nx(target.c_str(), file_name.c_str()) < 0) {
            fprintf(stderr, "error: cannot create symbolic link '%s': %s\n",
                    file_name.c_str(), strerror(errno));
            return false;
        }

        update_status(1.0, file_name, true);
        return true;
    }

    fd = open(file_name.c_str(), O_CREAT|O_WRONLY|O_BINARY, info.mode & 0777);
    if (fd < 0) {
        fprintf(stderr, "error: cannot open '%s' for writing: %s\n",
                file_name.c_str(), strerror(errno));
        return false;
    }

    update_status(0.0, file_name, false);

    if (info.blocks > 0) {
        nx_off_t  offset = 0;
        nx_off_t  size   = info.size;

        block = new (std::nothrow) uint8_t[COPY_BLOCK_SIZE];
        if (block == nullptr) {
            fprintf(stderr, "error: failed allocating copying blocks\n");
            return false;
        }

        while (size != 0) {
            update_status(static_cast<double>(offset) /
                    static_cast<double>(info.size), file_name, false);

            nx_off_t copysize = COPY_BLOCK_SIZE;
            if (copysize > size) {
                copysize = size;
            }

            auto nread = o->read(block, copysize, offset);
            if (nread < 0) {
                fprintf(stderr, "\nerror: an error occurred reading at offset "
                            "%#" PRIx64 ": %s\n", offset, strerror(errno));
                goto fail;
            }

            auto nwritten = pwrite(fd, block, nread, offset);
            if (nwritten < 0) {
                fprintf(stderr, "\nerror: an error occurred writing at offset "
                            "%#" PRIx64 ": %s\n", offset, strerror(errno));
                goto fail;
            }

            size -= nwritten, offset += nwritten;
        }

        delete[] block;
    }

    update_status(1.0, file_name, true);

    //
    // Apply xattrs.
    //
    apply_xattrs(fd, o, file_name, info.mode);

    //
    // Now set owner and flags.
    //
#ifndef _WIN32
    if (fchown(fd, info.uid, info.gid) < 0) {
        fprintf(stderr, "warning: cannot set uid %u / gid %u: %s\n",
                info.uid, info.gid, strerror(errno));
    }
#endif

    if (fchflags_nx(fd, info.flags) < 0) {
        fprintf(stderr, "warning: cannot set bsd flags %#x: %s\n",
                info.flags, strerror(errno));
    }

    //
    // Update times.
    //
    if (futimens_nx(fd, tv) < 0) {
        fprintf(stderr, "warning: cannot set access/modification "
                "time: %s\n", strerror(errno));
    }
    close(fd);

    return true;

fail:
    delete[] block;
    close(fd);
    unlink(file_name.c_str());
    return false;
}

static void
usage(char const *progname)
{
    fprintf(stderr, "usage: %s [-f|-x xid] device [volume [fileid]]\n", progname);
}

#define INVALID_XID (static_cast<uint64_t>(-1))

int
main_apfs_extract(nx::context &context, int argc, char **argv)
{
    char const *progname = *argv;
    bool first_xid = false;
    uint64_t xid = INVALID_XID;

    int c;
    while ((c = getopt(argc, argv, "fx:")) != EOF) {
        switch (c) {
            case 'f':
                first_xid = true;
                break;

            case 'x':
                xid = strtoull(optarg, nullptr, 0);
                break;

            default:
                usage(progname);
                exit(EXIT_FAILURE);
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 1) {
        usage(progname);
        exit(EXIT_FAILURE);
    }

    nx::device device;
    context.set_main_device(&device);
    if (!device.open(argv[0])) {
        fprintf(stderr, "error: cannot open '%s' for reading: %s\n",
                argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "[device] block size = %zu block count = %" PRIu64 "\n",
            device.get_block_size(), device.get_block_count());

    auto session = new apfs::session(&context);

    bool opened;

    if (xid != INVALID_XID) {
        opened = session->start_at(xid);
    } else {
        opened = session->start(!first_xid);
    }

    if (!opened) {
        exit(EXIT_FAILURE);
    }

    int volid = argc > 1 ? atoi(argv[1]) : 0;
    if (volid < 0) {
        volid = 0;
    }

    uint64_t fileid = argc > 2 ? strtoull(argv[2], nullptr, 0) : 0;

    auto volume = session->open(volid);
    if (volume == nullptr) {
        fprintf(stderr, "error: cannot open volume #%d\n", volid);
        exit(EXIT_FAILURE);
    }

    printf("volume=%s [case %s]\n", volume->get_name(),
            volume->is_case_sensitive() ? "sensitive" : "insensitive");

    apfs::object *o;
    if (fileid == 0) {
        o = volume->open_root();
    } else {
        o = volume->open(fileid);
    }

    bool success = false;
    if (o != nullptr) {
        if (o->is_regular() || o->is_symbolic_link()) {
            success = extract(o);
        } else {
            fprintf(stderr, "error: cannot extract a non regular file\n");
        }

        o->release();
    }

    delete session;

    exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
