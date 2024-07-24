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


#ifndef __TPL3Fl_HPP__
#define __TPL3Fl_HPP__ 1

#include <stdexcept>
#include <lzma.h>

template<typename T>
struct TPL3Fl {
public:
#if defined(__R2D9_RAW_HEAP_MEMORY_HPP__) && (__R2D9_RAW_HEAP_MEMORY_HPP__ == 1)
    r2d9::RawHeapMemory memorize(const Bytef * inBuff, const T inSize, const T outSize) {
        r2d9::RawHeapMemory v;
        v.resize(outSize);
        lzma_stream strm = LZMA_STREAM_INIT;
        strm.next_in = inBuff;
        strm.avail_in = inSize;
        strm.next_out = static_cast<uint8_t *>(v);
        strm.avail_out = outSize;
        for (size_t step = 0; step < 3; step++) {
            lzma_ret ret;
            switch (step) {
                case 0: ret = lzma_auto_decoder(&strm, UINT64_MAX, 0); break;
                case 1: ret = lzma_code(&strm, LZMA_RUN); break;
                case 2: ret = lzma_code(&strm, LZMA_FINISH); break;
                default: break;
            }
            if ((ret != LZMA_OK) && (ret != LZMA_STREAM_END)) {
                lzma_end(&strm);
                throw std::runtime_error("");
            }
        }
        lzma_end(&strm);
        return v;
    }
#else
    std::vector<uint8_t> vectorize(const uint8_t * inBuff, const T inSize, const T outSize) {
        std::vector<uint8_t> v;
        v.resize(outSize);
        lzma_stream strm = LZMA_STREAM_INIT;
        strm.next_in = inBuff;
        strm.avail_in = inSize;
        strm.next_out = v.data();
        strm.avail_out = outSize;
        for (size_t step = 0; step < 3; step++) {
            lzma_ret ret;
            switch (step) {
                case 0: ret = lzma_auto_decoder(&strm, UINT64_MAX, 0); break;
                case 1: ret = lzma_code(&strm, LZMA_RUN); break;
                case 2: ret = lzma_code(&strm, LZMA_FINISH); break;
                default: break;
            }
            if ((ret != LZMA_OK) && (ret != LZMA_STREAM_END)) {
                lzma_end(&strm);
                throw std::runtime_error("");
            }
        }
        lzma_end(&strm);
        return v;
    }
#endif
};

#endif // !__TPL3Fl_HPP__
