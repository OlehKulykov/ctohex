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


#ifndef __TPL2FL_HPP__
#define __TPL2FL_HPP__ 1

#include <stdexcept>
#include <zstd.h>

template<typename T>
struct TPL2Fl {
private:
    ZSTD_DCtx * _dctx;
    
public:
#if defined(__R2D9_RAW_HEAP_MEMORY_HPP__) && (__R2D9_RAW_HEAP_MEMORY_HPP__ > 0)
    r2d9::RawHeapMemory memorize(const void * inBuff, const T inSize, const T outSize) {
        r2d9::RawHeapMemory v;
        const size_t dstCapacity = ::ZSTD_getDecompressedSize(inBuff, inSize);
        if ((dstCapacity > 0) && !::ZSTD_isError(dstCapacity)) {
            v.resize(dstCapacity);
            const size_t dstSize = ::ZSTD_decompressDCtx(_dctx, static_cast<void *>(v), dstCapacity, inBuff, inSize);
            if (::ZSTD_isError(dstSize) || (outSize != dstSize)) {
                throw std::runtime_error("");
            }
            if (v.size() != dstSize) {
                v.resize(dstSize);
            }
        }
        return v;
    }
#else
    std::vector<uint8_t> vectorize(const void * inBuff, const T inSize, const T outSize) {
        std::vector<uint8_t> v;
        const size_t dstCapacity = ::ZSTD_getDecompressedSize(inBuff, inSize);
        if ((dstCapacity > 0) && !::ZSTD_isError(dstCapacity)) {
            v.resize(dstCapacity);
            const size_t dstSize = ::ZSTD_decompressDCtx(_dctx, v.data(), dstCapacity, inBuff, inSize);
            if (::ZSTD_isError(dstSize) || (outSize != dstSize)) {
                throw std::runtime_error("");
            }
            if (v.size() != dstSize) {
                v.resize(dstSize);
            }
        }
        return v;
    }
#endif
    
    TPL2Fl() : _dctx(NULL) { // nullptr C++11
        _dctx = ::ZSTD_createDCtx();
        if (!_dctx) {
            throw std::bad_alloc();
        }
    }
    
    ~TPL2Fl() {
        ::ZSTD_freeDCtx(_dctx);
    }
};

#endif // !__TPL2FL_HPP__
