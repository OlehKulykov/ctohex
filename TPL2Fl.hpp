
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
        const size_t dstCapacity = ZSTD_getDecompressedSize(inBuff, inSize);
        if (dstCapacity > 0 && !ZSTD_isError(dstCapacity)) {
            v.resize(dstCapacity);
            const size_t dstSize = ZSTD_decompressDCtx(_dctx, static_cast<void *>(v), dstCapacity, inBuff, inSize);
            if (ZSTD_isError(dstSize) || (dstSize != outSize)) {
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
        const size_t dstCapacity = ZSTD_getDecompressedSize(inBuff, inSize);
        if (dstCapacity > 0 && !ZSTD_isError(dstCapacity)) {
            v.resize(dstCapacity);
            const size_t dstSize = ZSTD_decompressDCtx(_dctx, v.data(), dstCapacity, inBuff, inSize);
            if (ZSTD_isError(dstSize) || (dstSize != outSize)) {
                throw std::runtime_error("");
            }
            if (v.size() != dstSize) {
                v.resize(dstSize);
            }
        }
        return v;
    }
#endif
    
    TPL2Fl() {
        _dctx = ZSTD_createDCtx();
        if (!_dctx) {
            throw std::bad_alloc();
        }
    }
    
    ~TPL2Fl() {
        ZSTD_freeDCtx(_dctx);
    }
};

#endif
