
#ifndef __TPL1FL_HPP__
#define __TPL1FL_HPP__ 1

#include <stdexcept>
#include <zlib.h>

template<typename T>
struct TPL1Fl {
public:
#if defined(__R2D9_RAW_HEAP_MEMORY_HPP__) && (__R2D9_RAW_HEAP_MEMORY_HPP__ > 0)
    r2d9::RawHeapMemory memorize(const Bytef * inBuff, const T inSize, const T outSize) {
        r2d9::RawHeapMemory v;
        v.resize(outSize);
        uLong destLen = outSize;
        if ((uncompress(static_cast<Bytef *>(v), &destLen, inBuff, inSize) != Z_OK) || (destLen != outSize)) {
            throw std::runtime_error("");
        }
        return v;
    }
#else
    std::vector<uint8_t> vectorize(const Bytef * inBuff, const T inSize, const T outSize) {
        std::vector<uint8_t> v;
        v.resize(outSize);
        uLong destLen = outSize;
        if ((uncompress(v.data(), &destLen, inBuff, inSize) != Z_OK) || (destLen != outSize)) {
            throw std::runtime_error("");
        }
        return v;
    }
#endif
};

#endif
