
#ifndef __TPL1FL_HPP__
#define __TPL1FL_HPP__ 1

#include <stdexcept>
#include <zlib.h>

template<typename T>
struct TPL1Fl {
public:
    std::vector<uint8_t> vectorize(const Bytef * inBuff, const T inSize, const T outSize) {
        std::vector<uint8_t> v;
        v.resize(outSize);
        uLong destLen = outSize;
        if (uncompress(v.data(), &destLen, inBuff, inSize) != Z_OK) {
            throw std::runtime_error("");
        }
        return v;
    }
};

#endif
