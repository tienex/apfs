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

#ifndef __nx_context_h
#define __nx_context_h

#include "nx/device.h"
#include "nx/logger.h"

namespace nx {

class context {
private:
    logger *_logger;
    device *_main_device;
    device *_tier2_device;

public:
    context()
        : _logger      (nullptr)
        , _main_device (nullptr)
        , _tier2_device(nullptr)
    { }

public:
    inline void set_logger(logger *logger)
    { _logger = logger; }
    inline logger *get_logger() const
    { return _logger; }

public:
    inline void set_main_device(device *device)
    { _main_device = device; }
    inline device *get_main_device() const
    { return _main_device; }

public:
    inline void set_tier2_device(device *device)
    { _tier2_device = device; }
    inline device *get_tier2_device() const
    { return _tier2_device; }

protected:
    friend class btree_traverser;
    friend class container;
    friend class enumerator;
    friend class object;
    friend class volume;
    void log(severity::value const &severity, char const *format, ...);
};

}

#endif  // !__nx_context_h
