// MIT License
//
// Copyright (c) Oleh Kulykov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#ifndef __TPL1FL_HPP__
#define __TPL1FL_HPP__ 1

#include <stdexcept>
#include <zlib.h>

template<typename T>
struct TPL1Fl {
public:
#if defined(__R2D9_RAW_HEAP_MEMORY_HPP__) && (__R2D9_RAW_HEAP_MEMORY_HPP__ > 0)
    r2d9::RawHeapMemory memorize(const void * inBuff, const T inSize, const T outSize) {
        r2d9::RawHeapMemory v;
        v.resize(outSize);
        uLong destLen = outSize;
        if ((::uncompress(static_cast<Bytef *>(v), &destLen, static_cast<const Bytef *>(inBuff), inSize) != Z_OK) || (outSize != destLen)) {
            throw std::runtime_error("");
        }
        return v;
    }
#else
    std::vector<uint8_t> vectorize(const void * inBuff, const T inSize, const T outSize) {
        std::vector<uint8_t> v;
        v.resize(outSize);
        uLong destLen = outSize;
        if ((::uncompress(v.data(), &destLen, static_cast<const Bytef *>(inBuff), inSize) != Z_OK) || (outSize != destLen)) {
            throw std::runtime_error("");
        }
        return v;
    }
#endif
};

#endif // !__TPL1FL_HPP__
