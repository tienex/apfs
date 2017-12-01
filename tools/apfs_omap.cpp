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

#include "nx/container.h"
#include "nx/enumerator.h"
#include "nx/volume.h"

#include "nxcompat/nxcompat.h"

#include <cstdio>
#include <cstdlib>

static bool
omap_traverser(uint32_t max_level, uint32_t level, uint32_t index,
        nx_omap_key_t const &key, nx_omap_value_t const &value)
{
    printf("%*slevel=%u node=%u key={oid=0x%016" PRIx64
            " xid=%016" PRIx64 "} ",
            static_cast<int>(max_level - level), "",
            level, index,
            nx::swap(key.ok_oid),
            nx::swap(key.ok_xid));
    if (level != 0) {
        printf("btnoid={0x%016" PRIx64 "}",
                nx::swap(value.ov_paddr));
    } else {
        printf("refoid={flags=0x%016" PRIx64 " oid=%016" PRIx64 "}",
                nx::swap(value.ov_flags),
                nx::swap(value.ov_oid));
    }
    printf("\n");

    return true;
}

static void
usage(char const *progname)
{
    fprintf(stderr, "usage: %s [-f|-x xid] device [volume]\n", progname);
}

#define INVALID_XID (static_cast<uint64_t>(-1))

int
main_apfs_omap(nx::context &context, int argc, char **argv)
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

    int volid = argc > 1 ? atoi(argv[1]) : 0;
    if (volid < 0) {
        volid = 0;
    }

    auto volume = container->open_volume(volid);
    if (volume == nullptr) {
        fprintf(stderr, "error: cannot open volume #%d\n", volid);
        exit(EXIT_FAILURE);
    }

    printf("volume=%s [case %s]\n", volume->get_name(),
            volume->is_case_sensitive() ? "sensitive" : "insensitive");

    volume->traverse_omap(omap_traverser);

    delete volume;
    delete container;

    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
