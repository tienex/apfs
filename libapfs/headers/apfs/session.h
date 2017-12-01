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

#ifndef __apfs_session_h
#define __apfs_session_h

#include "apfs/base.h"

namespace apfs {

class volume;

class session {
private:
    nx::context                *_context;
    nx::container              *_container;
    bool                        _free_context;

    std::map<size_t, volume *>  _volumes;

public:
    session(nx::context *context = nullptr);
    ~session();

public:
    bool start_at(uint64_t xid = INVALID_XID);
    bool start(bool last_xid = true);
    void stop();

public:
    inline void set_logger(nx::logger *logger)
    { _context->set_logger(logger); }
    inline nx::logger *get_logger() const
    { return _context->get_logger(); }

public:
    inline void set_main_device(nx::device *device)
    { _context->set_main_device(device); }
    inline nx::device *get_main_device() const
    { return _context->get_main_device(); }

public:
    inline void set_tier2_device(nx::device *device)
    { _context->set_tier2_device(device); }
    inline nx::device *get_tier2_device() const
    { return _context->get_tier2_device(); }

public:
    inline nx::container *get_container() const
    { return _container; }

public:
    volume *open(size_t volid);
};

}

#endif  // !__apfs_session_h
