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

static bool verbose = false;

static bool
root_traverser(void *opaque, uint32_t max_level, uint32_t level, uint32_t index,
        nx::object::sized_value_type const &key,
        nx::object::sized_value_type const &value)
{
    nx_dumper_t *dumper = (nx_dumper_t *)opaque;
    size_t       indent = (max_level - level) * 4;

    uint64_t obj_id = nx::swap(*reinterpret_cast<uint64_t const *>(key.first));
    printf("%*slevel=%u node=%u "
            "key={ID=0x%015" PRIx64 " type=%#x len=%#" PRIxSIZE "} ",
            static_cast<int>(indent), "",
            level, index,
            APFS_OBJECT_ID_ID(obj_id),
            APFS_OBJECT_ID_TYPE(obj_id),
            key.second);

    if (level != 0) {
        printf("btnoid={0x%016" PRIx64 "}\n",
                nx::swap(*(uint64_t const *)value.first));
    } else {
        printf("value={len=%#" PRIxSIZE "}\n", value.second);
        if (verbose) {
            nx_dumper_set_indent(dumper, indent + 4);
            apfs_object_dump(dumper, key.first, key.second,
                    value.first, value.second);
            nx_dumper_flush(dumper);
        }
    }

    return true;
}

static void
usage(char const *progname)
{
    fprintf(stderr, "usage: %s [-f|-x xid] [-v] device [volume]\n", progname);
}

#define INVALID_XID (static_cast<uint64_t>(-1))

int
main_apfs_traverse(nx::context &context, int argc, char **argv)
{
    char const *progname = *argv;
    bool first_xid = false;
    uint64_t xid = INVALID_XID;

    int c;
    while ((c = getopt(argc, argv, "fvx:")) != EOF) {
        switch (c) {
            case 'f':
                first_xid = true;
                break;

            case 'v':
                verbose = true;
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

    auto dumper = nx_dumper_new(stdout, NX_DUMPER_STYLE_TEXT,
            NX_DUMPER_FLAG_PRETTY);

    volume->traverse_root(root_traverser, dumper);

    nx_dumper_dispose(dumper);

    delete volume;
    delete container;

    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
