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
scavenge(void *opaque, uint64_t blockno, nx_object_t const *object)
{
    nx_dumper_t *dumper = (nx_dumper_t *)opaque;

    if (!verbose && nx_dumper_is_text(dumper)) {
        printf("Candidate Block %" PRIu64 " (%#" PRIx64 "): "
                "Checksum = 0x%016" PRIx64 " "
                "Object ID = %" PRIu64 " (%#" PRIx64 ") "
                "Checkpoint ID = %" PRIu64 " (%#" PRIx64 ") "
                "Type = %u.%u [%s]\n",
                blockno, blockno,
                nx::swap(object->o_checksum),
                nx::swap(object->o_oid), nx::swap(object->o_oid),
                nx::swap(object->o_xid), nx::swap(object->o_xid),
                NX_OBJECT_GET_TYPE(nx::swap(object->o_type)),
                nx::swap(object->o_subtype),
                nx_object_name(object));
    } else {
        nx_dumper_set_indent(dumper, 0);

        if (nx_dumper_is_xml(dumper)) {
            nx_dumper_attr(dumper, "number", "%" PRIu64, blockno);
        }
        nx_dumper_open_container(dumper, "block");

        if (!nx_dumper_is_xml(dumper)) {
            nx_dumper_emit(dumper, 0,
                    "{d:/Candidate Block #%" PRIu64 "}"
                    "{e:number/%" PRIu64 "}\n",
                    blockno, blockno);
        }

        nx_dumper_set_indent(dumper, 4);
        nx_object_dump(dumper, object);

        nx_dumper_close(dumper);
    }

    return true;
}

static void
usage(char const *progname)
{
    fprintf(stderr, "usage: %s [-v] device [block]\n", progname);
}

int
main_nx_scavenge(nx::context &context, int argc, char **argv)
{
    char const *progname = *argv;

    int c;
    while ((c = getopt(argc, argv, "v")) != EOF) {
        switch (c) {
            case 'v':
                verbose = true;
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
                argv[0],strerror(errno));
        exit(EXIT_FAILURE);
    }

    uint64_t bno = argc > 1 ? strtoull(argv[1], nullptr, 0) :
        static_cast <uint64_t> (-1);

    fprintf(stderr, "[device] block size = %zu block count = %" PRIu64 "\n",
            device.get_block_size(), device.get_block_count());

    auto container = new nx::container(&context);

    auto dumper = nx_dumper_new(stdout, NX_DUMPER_STYLE_TEXT,
            NX_DUMPER_FLAG_PRETTY);

    nx_dumper_open_container(dumper, "nx-scavenge");

    container->scavenge(scavenge, dumper, bno);

    nx_dumper_close(dumper);

    nx_dumper_dispose(dumper);

    delete container;

    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
