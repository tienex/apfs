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

#include "nx/nx.h"

#include "nxcompat/nxcompat.h"

#include <cstdio>
#include <cstdlib>

#include <memory>
#include <string>
#include <vector>

#define INVALID_XID (static_cast<uint64_t>(-1))

static void
usage(char const *progname)
{
    fprintf(stderr, "usage: %s [-f|-x xid] [-kpq] device\n", progname);
}

enum action {
    ACTION_PRINT_NAME,
    ACTION_PRINT_UUID,
    ACTION_CHECK
};

int
main(int argc, char **argv)
{
    char const *progname = *argv;
    bool first_xid = false;
    uint64_t xid = INVALID_XID;
    enum action action = ACTION_PRINT_NAME;

    int c;
    while ((c = getopt(argc, argv, "fkpqx:")) != EOF) {
        switch (c) {
            case 'p':
                action = ACTION_PRINT_NAME;
                break;

            case 'k':
                action = ACTION_PRINT_UUID;
                break;

            case 'q':
                action = ACTION_CHECK;
                break;

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

    std::string devname = argv[0];

    //
    // Is this a real path?
    //
    if (access(devname.c_str(), R_OK) != 0 && errno == ENOENT) {
        std::string devname2 = "/dev/" + devname;
        if (access(devname2.c_str(), R_OK) != 0) {
            fprintf(stderr, "error: cannot find '%s' for reading: %s\n",
                    devname.c_str(), strerror(errno));
            exit(EXIT_FAILURE);
        }
        devname = devname2;
    }

    nx::context context;
    nx::device device;
    context.set_main_device(&device);
    if (!device.open(devname.c_str())) {
        fprintf(stderr, "error: cannot open '%s' for reading: %s\n",
                devname.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    auto container = new nx::container(&context);

    bool opened;

    if (xid != INVALID_XID) {
        opened = container->open_at(xid);
    } else {
        opened = container->open(!first_xid);
    }

    if (!opened) {
        exit(EXIT_FAILURE);
    }

    for (size_t volid = 0;; volid++) {
        auto volume = container->open_volume(volid);
        if (volume == nullptr)
            break;

        if (action == ACTION_PRINT_UUID) {
            char buf[128];
            auto uuid = volume->get_uuid();
            nx_uuid_format(&uuid, buf, sizeof(buf));
            printf("%s\n", buf);
        } else if (action == ACTION_PRINT_NAME) {
            printf("%s\n", volume->get_name());
        }
        delete volume;
    }

    delete container;

    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
