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

#ifndef __nx_stack_h
#define __nx_stack_h

#include <functional>
#include <vector>

namespace nx {

template <typename T>
class stack {
private:
    T                       _top;
    std::vector <T>         _saved;
    std::function<void(T&)> _free;

public:
    stack(std::function<void(T&)> const &free = std::function<void(T&)>())
        : _top (T())
        , _free(free)
    { }

    ~stack()
    { clear(); }

public:
    inline void push(T o)
    {
        _saved.push_back(_top);
        _top = o;
    }

    inline void pop(bool free = true)
    {
        auto o = _saved.back();
        _saved.pop_back();
        if (free && _free) {
            _free(_top);
        }
        _top = o;
    }

    inline void clear()
    {
        while (!empty()) {
            pop();
        }
    }

    inline bool empty() const
    { return _saved.empty(); }

    inline size_t depth() const
    { return _saved.size(); }

    inline T const &top() const
    { return _top; }

    inline T &top()
    { return _top; }
};

}

#endif  // !__nx_stack_h
