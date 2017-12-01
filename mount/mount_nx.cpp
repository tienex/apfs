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

#include "nx_volume.h"

#include "nxtools/stderr_logger.h"
#include "nxtools/syslog_logger.h"

#include <sys/stat.h>
#ifdef __APPLE__
#include <unistd.h>
#endif

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void
usage(char const *progname)
{
    fprintf(stderr, "usage: %s [-F|-x xid] [fuse options] "
            "device [volume] mountpoint\n", progname);
}

int
main(int argc, char **argv)
{
    std::vector<char const *> args;
    char const *progname = *argv;
    bool first_xid = false;
    uint64_t xid = apfs::INVALID_XID;
    char const *devname = nullptr;
    bool next_arg_is_for_fuse = false;
    bool foreground = false;
#ifdef __APPLE__
    bool automounting = false;
#endif

    args.push_back(progname);

#ifdef __APPLE__
    // Stupid assumptions, if root then it's via diskarbitrationd.
    automounting = (getuid() == 0);
#endif

    for (int n = 1; n < argc; n++) {
        bool next_arg_is_fuse = false;

        if (strcmp(argv[n], "-h") == 0 || strcmp(argv[n], "--help") == 0) {
            usage(progname);
            args.push_back(argv[n]);
            return apfs_fuse::main(args);
        }

        if (!next_arg_is_for_fuse) {
            if (argv[n][0] == '-') {
                if (argv[n][1] == 'f' && argv[n][2] == '\0') {
                    foreground = true;
                } else if (argv[n][1] == 'x' && argv[n][2] == '\0') {
                    if (++n >= argc) {
                        usage(progname);
                        exit(EXIT_FAILURE);
                    }

                    xid = strtoull(argv[n], nullptr, 0);
                    continue;
                } else if (argv[n][1] == 'F' && argv[n][2] == '\0') {
                    first_xid = true;
                    continue;
                } else if (argv[n][1] == 'o' && argv[n][2] == '\0') {
                    if (argv[n + 1] == nullptr) {
                        usage(progname);
                        exit(EXIT_FAILURE);
                    }
#ifdef __APPLE__
                    if (strcmp(argv[n + 1], "noowners") == 0) {
                        // Skip diskarbitration passed arguments.
                        n++;
                        continue;
                    }
#endif
                    if (strcmp(argv[n + 1], "rsrcfork") == 0) {
                        apfs_fuse::expose_resource_fork = true;
                        n++;
                        continue;
                    }
                    if (strcmp(argv[n + 1], "xattrdir") == 0) {
                        apfs_fuse::expose_xattr_directory = true;
                        n++;
                        continue;
                    }
                    next_arg_is_fuse = true;
                }
            } else {
                if (!next_arg_is_for_fuse) {
                    if (devname == nullptr) {
                        devname = argv[n];
                        continue;
                    }
                }
            }
        }

        args.push_back(argv[n]);
        next_arg_is_for_fuse = next_arg_is_fuse;
    }

    if (devname == nullptr && args.size() == 1) {
        usage(progname);
        exit(EXIT_FAILURE);
    }

    //
    // Setup the session
    //
    nx::device device;
    apfs::session session;
    nxtools::stderr_logger logger;

    session.set_logger(&logger);
    session.set_main_device(&device);

    //
    // Open the device
    //
    if (!device.open(devname)) {
        fprintf(stderr, "error: cannot open '%s' for reading: %s\n",
                devname, ::strerror(errno));
        exit(EXIT_FAILURE);
    }

    //
    // Start the session
    //
    bool opened;

    if (xid != apfs::INVALID_XID) {
        opened = session.start_at(xid);
    } else {
        opened = session.start(!first_xid);
    }

    if (!opened) {
        exit(EXIT_FAILURE);
    }

    //
    // Wrap the session for fuse
    //
    apfs_fuse::the_volume = new apfs_fuse::nx_volume(&session);

#ifdef __APPLE__
    //
    // Obtain the volume name, and pass it to fuse.
    //
    char *volname;
    if (!(asprintf(&volname, "volname=%s",
                    apfs_fuse::the_volume->get_name()) < 0)) {
        args.insert(args.end() - 1, "-o");
        args.insert(args.end() - 1, volname);
    }
    args.insert(args.end() - 1, "-o");
    args.insert(args.end() - 1, "kill_on_unmount");
    args.insert(args.end() - 1, "-o");
    args.insert(args.end() - 1, "extended_security");
    args.insert(args.end() - 1, "-o");
    args.insert(args.end() - 1, "rdonly");
    if (automounting) {
        args.insert(args.end() - 1, "-o");
        args.insert(args.end() - 1, "allow_other");
        args.insert(args.end() - 1, "-o");
        args.insert(args.end() - 1, "local");
    }
#endif
    char *fsname;

    if (!(asprintf(&fsname, "fsname=%s", devname) < 0)) {
        args.insert(args.end() - 1, "-o");
        args.insert(args.end() - 1, fsname);
    }
    if (!apfs_fuse::expose_resource_fork &&
            !apfs_fuse::expose_xattr_directory &&
            sizeof(ino_t) == sizeof(nx_ino_t)) {
        args.insert(args.end() - 1, "-o");
        args.insert(args.end() - 1, "use_ino");
#if 0
        args.insert(args.end() - 1, "-o");
        args.insert(args.end() - 1, "readdir_ino");
#endif
    }

    if (!foreground) {
        // Move to syslog logging
        // TODO: Make this a runtime flag.
        session.set_logger(new nxtools::syslog_logger(progname));
    }

#ifdef __APPLE__
    args.insert(args.end() - 1, "-o");
    args.insert(args.end() - 1, "novolicon");
    apfs_fuse::novolicon_unsupported = false;
#endif

    int rc = apfs_fuse::main(args);

#ifdef __APPLE__
    if (rc != EXIT_SUCCESS) {
        //
        // Maybe novolicon is not supported because this osxfuse is not patched,
        // extract the volume icon, if any, and put it into a temporary file
        // and use it.
        //
        apfs_fuse::novolicon_unsupported = true;
        args.erase(args.end() - 3, args.end() - 1);

        if (auto path = apfs_fuse::extract_volume_icon("mount_nx")) {
            char *iconopt;
            if (!(asprintf(&iconopt, "volicon=%s", path) < 0)) {
                args.insert(args.end() - 1, "-o");
                args.insert(args.end() - 1, iconopt);
            }
        }

        rc = apfs_fuse::main(args);
    }
#endif

    exit(rc);
    return rc;
}
