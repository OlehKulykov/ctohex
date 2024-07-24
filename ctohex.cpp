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


#include <exception>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "TPL1Fl.hpp"
#include "TPL2Fl.hpp"
#include "TPL3Fl.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include <zstd.h>
#include <lzma.h>

#define _original_file_name_size 1024
char _original_file_name[_original_file_name_size] = { 0 };

#define _lower_case_file_name_size 1024
char _lower_case_file_name[_lower_case_file_name_size] = { 0 };

uint8_t * _in_file_buff = NULL;
long long _in_file_size = 0;

bool _print_output0 = false;
bool _print_output1 = false;
bool _print_output2 = false;
bool _print_output3 = false;
bool _generate_double_include_header = false;
bool _add_write_to_file_function = false;
bool _add_clang_gcc_nullability = false;
bool _add_pointer = false;

struct ResultInfo {
    size_t index;
    size_t size;
    size_t sizeLen;
    std::string path;
    std::string algo;
    
    ResultInfo(const size_t aIndex, const size_t aSize, const char * aPath, const char * aAlgo) :
        index(aIndex),
        size(aSize),
        sizeLen(0),
        path(aPath),
        algo(aAlgo) {
            char buff[32];
            const int len = snprintf(buff, 32, "%llu", static_cast<unsigned long long>(aSize));
            if (len > 0) {
                sizeLen = len;
            }
        }
};

std::vector<ResultInfo> _results;

void print_output_buff(const uint8_t * buff, const size_t buffSize) {
    std::flush(std::cout);
    const char * str = (const char *)buff;
    for (size_t i = 0; i < buffSize; i++) {
        const char c = str[i];
        if (::isprint(c) || ::iscntrl(c)) {
            std::cout << c;
        } else {
            std::cout << ' ';
        }
    }
    std::flush(std::cout);
}

void write_str(FILE * f, const char * str) {
    ::fwrite(str, ::strlen(str), 1, f);
    ::fflush(f);
}

void write_file_buff(FILE * f, const uint8_t * buff, long long buffSize) {
    char str1[1024];
    const size_t str1Size = 1024;
    
    int first = 1;
    long long written = 0;
    for (long long j = 0; j < (buffSize / 16); j++) {
        const char * format;
        if (first) {
            format = "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X";
        } else {
            format = ",\n0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X";
        }
        first = 0;
        const int strLen = ::snprintf(str1, str1Size, format,
                                      buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],
                                      buff[8],buff[9],buff[10],buff[11],buff[12],buff[13],buff[14],buff[15]);
        ::fwrite(str1, strLen, 1, f);
        written += 16;
        buff += 16;
    }
    if (written > 0 && written < buffSize) {
        ::fwrite(",\n", 2, 1, f);
    }
    first = 1;
    while (written < buffSize) {
        const char * format;
        if (first) {
            format = "0x%02X";
        } else {
            format = ",0x%02X";
        }
        first = 0;
        const int strLen = ::snprintf(str1, str1Size, format, *buff);
        ::fwrite(str1, strLen, 1, f);
        buff++;
        written++;
    }
}

int write_output_result(const int index, const char * algo, const uint8_t * dstBuff, const size_t dstSize) {
    char outFileName[4096] = { 0 };
    ::snprintf(outFileName, 4096, "file__%s_%i.h", _lower_case_file_name, index);
    
    FILE * outFile = ::fopen(outFileName, "w+b");
    if (!outFile) {
        std::flush(std::cerr) << "Can't open output file." << std::endl;
        return __LINE__;
    }
    
    char str1[4096];
    const size_t str1Size = 4096;
    
    char nonullPtr[32] = { 0 };
    if (_add_clang_gcc_nullability) {
        ::strcpy(nonullPtr, "* _Nonnull");
    } else {
        ::strcpy(nonullPtr, "*");
    }
    
    ::snprintf(str1, str1Size, "//%i %s\n", index, _original_file_name);
    write_str(outFile, str1);
    
    ::snprintf(str1, str1Size, "#ifndef FILE__%s_SIZE\n#define FILE__%s_SIZE_SRC %lli\n", _lower_case_file_name, _lower_case_file_name, _in_file_size);
    write_str(outFile, str1);

    ::snprintf(str1, str1Size, "#define FILE__%s_SIZE %llu\n", _lower_case_file_name, static_cast<unsigned long long>(dstSize));
    write_str(outFile, str1);
    
    if (_add_write_to_file_function) {
        write_str(outFile, "#include <stdbool.h>\n");
    }
    
    if (_generate_double_include_header) {
        write_str(outFile, "#if defined(__cplusplus)\n");
        
        ::snprintf(str1, str1Size, "extern \"C\" unsigned char FILE__%s[%llu];\n", _lower_case_file_name, static_cast<unsigned long long>(dstSize));
        write_str(outFile, str1);
        
        if (_add_pointer) {
            ::snprintf(str1, str1Size, "extern \"C\" const unsigned char %s FILE__%s_PTR;\n", nonullPtr, _lower_case_file_name);
            write_str(outFile, str1);
        }
        if (_add_write_to_file_function) {
            ::snprintf(str1, str1Size, "extern \"C\" bool FILE__%s_write_to_file(const char %s);\n", _lower_case_file_name, nonullPtr);
            write_str(outFile, str1);
        }
        
        write_str(outFile, "#else\n");
        
        ::snprintf(str1, str1Size, "extern unsigned char FILE__%s[%llu];\n", _lower_case_file_name, static_cast<unsigned long long>(dstSize));
        write_str(outFile, str1);
        
        if (_add_pointer) {
            ::snprintf(str1, str1Size, "extern const unsigned char %s FILE__%s_PTR;\n", nonullPtr, _lower_case_file_name);
            write_str(outFile, str1);
        }
        if (_add_write_to_file_function) {
            ::snprintf(str1, str1Size, "extern bool FILE__%s_write_to_file(const char %s);\n", _lower_case_file_name, nonullPtr);
            write_str(outFile, str1);
        }
        
        write_str(outFile, "#endif\n");
        
        ::snprintf(str1, str1Size, "unsigned char FILE__%s[%llu] = {\n", _lower_case_file_name, static_cast<unsigned long long>(dstSize));
        write_str(outFile, str1);
        
        write_file_buff(outFile, dstBuff, dstSize);
        write_str(outFile, "};\n");
        
        if (_add_pointer) {
            ::snprintf(str1, str1Size, "const unsigned char %s FILE__%s_PTR = FILE__%s;\n", nonullPtr, _lower_case_file_name, _lower_case_file_name);
            write_str(outFile, str1);
        }
        if (_add_write_to_file_function) {
            ::snprintf(str1, str1Size, "bool FILE__%s_write_to_file(const char %s file_path) {\n", _lower_case_file_name, nonullPtr);
            write_str(outFile, str1);
            write_str(outFile, "  FILE * file = fopen(file_path, \"w+b\");\n");
            
            ::snprintf(str1, str1Size, "  if (file) {\n    const unsigned int written = (unsigned int)fwrite(FILE__%s, 1, %llu, file);\n", _lower_case_file_name, static_cast<unsigned long long>(dstSize));
            write_str(outFile, str1);
            
            ::snprintf(str1, str1Size, "    fclose(file);\n    return (written == %llu);\n  }\n  return false;\n}\n", static_cast<unsigned long long>(dstSize));
            write_str(outFile, str1);
        }
        
        write_str(outFile, "#endif\n");
    } else {
        ::snprintf(str1, str1Size, "static unsigned char FILE__%s[%llu] = {\n", _lower_case_file_name, static_cast<unsigned long long>(dstSize));
        write_str(outFile, str1);
        
        write_file_buff(outFile, dstBuff, dstSize);
        write_str(outFile, "};\n");
        
        if (_add_pointer) {
            ::snprintf(str1, str1Size, "static const unsigned char %s FILE__%s_PTR = FILE__%s;\n", nonullPtr, _lower_case_file_name, _lower_case_file_name);
            write_str(outFile, str1);
        }
        if (_add_write_to_file_function) {
            ::snprintf(str1, str1Size, "static bool FILE__%s_write_to_file(const char %s file_path) {\n", _lower_case_file_name, nonullPtr);
            write_str(outFile, str1);
            write_str(outFile, "  FILE * file = fopen(file_path, \"w+b\");\n");
            
            ::snprintf(str1, str1Size, "  if (file) {\n    const unsigned int written = (unsigned int)fwrite(FILE__%s, 1, %llu, file);\n", _lower_case_file_name, static_cast<unsigned long long>(dstSize));
            write_str(outFile, str1);
            
            ::snprintf(str1, str1Size, "    fclose(file);\n    return (written == %llu);\n  }\n  return false;\n}\n", static_cast<unsigned long long>(dstSize));
            write_str(outFile, str1);
        }
        
        write_str(outFile, "#endif\n");
    }
    
    ::fclose(outFile);
    
    bool printOutput = false;
    switch (index) {
        case 0: printOutput = _print_output0; break;
        case 1: printOutput = _print_output1; break;
        case 2: printOutput = _print_output2; break;
        case 3: printOutput = _print_output3; break;
        default: break;
    }
    
    if (printOutput) {
        std::flush(std::cout) << std::endl << "---- [" << index << "] '" << algo << "' buffer, " << _in_file_size << " -> " << dstSize << ", " << outFileName << ":" << std::endl;
        print_output_buff(dstBuff, dstSize);
    }
    
    _results.emplace_back(ResultInfo(index, dstSize, outFileName, algo));
  
    return 0;
}

int write_as_zlib(void) {
    uLong buffSize = compressBound((uLong)_in_file_size);
    buffSize += 256 * 1024;
    uint8_t * dstBuff = static_cast<uint8_t *>(::malloc(buffSize));
    if (!dstBuff) {
        std::flush(std::cout) << "Can't allocate memory." << std::endl;
        return __LINE__;
    }
    uLong dstSize = buffSize;
    int res = compress2(dstBuff, &dstSize, _in_file_buff, _in_file_size, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        ::free(dstBuff);
        return __LINE__;
    }
    
    {
        TPL1Fl<size_t> tpl;
        std::vector<uint8_t> test = tpl.vectorize(dstBuff, dstSize, _in_file_size);
        if (test.size() != _in_file_size || ::memcmp(test.data(), _in_file_buff, _in_file_size)) {
            std::flush(std::cerr) << "ERROR: Test zlib result." << std::endl;
            return __LINE__;
        }
    }
    
    write_output_result(1, "z", dstBuff, dstSize);
    
    ::free(dstBuff);
    return 0;
}

int write_as_zstd(void) {
    ZSTD_CCtx * cctx = ZSTD_createCCtx();
    size_t buffSize = ZSTD_compressBound(_in_file_size);
    buffSize += 256 * 1024;
    uint8_t * dstBuff = static_cast<uint8_t *>(::malloc(buffSize));
    if (!dstBuff) {
        std::flush(std::cerr) << "Can't allocate memory." << std::endl;
        return __LINE__;
    }
    size_t dstSize = ZSTD_compressCCtx(cctx,
                                       dstBuff, buffSize,
                                       _in_file_buff, _in_file_size,
                                       19);
    if (ZSTD_isError(dstSize)) {
        ::free(dstBuff);
        ZSTD_freeCCtx(cctx);
        return __LINE__;
    }
    
    {
        TPL2Fl<size_t> tpl;
        std::vector<uint8_t> test = tpl.vectorize(dstBuff, dstSize, _in_file_size);
        if (test.size() != _in_file_size || ::memcmp(test.data(), _in_file_buff, _in_file_size)) {
            std::flush(std::cerr) << "ERROR: Test zstd result." << std::endl;
            return __LINE__;
        }
    }
    
    write_output_result(2, "zstd", dstBuff, dstSize);
    
    ::free(dstBuff);
    ZSTD_freeCCtx(cctx);
    return 0;
}

int write_as_lzma(void) {
    lzma_stream strm = LZMA_STREAM_INIT;
    
    lzma_options_lzma opt_lzma2 = { 0 };
    if (lzma_lzma_preset(&opt_lzma2, LZMA_PRESET_EXTREME)) {
        lzma_end(&strm);
        return __LINE__;
    }
    
    lzma_filter filters[] = {
        { .id = LZMA_FILTER_LZMA2, .options = &opt_lzma2 },
        { .id = LZMA_VLI_UNKNOWN, .options = NULL },
    };
    
    lzma_ret ret = lzma_stream_encoder(&strm, filters, LZMA_CHECK_CRC32);
    if (ret != LZMA_OK) {
        lzma_end(&strm);
        return __LINE__;
    }
    
    size_t buffSize = _in_file_size + (1024 * 1024);
    uint8_t * dstBuff = static_cast<uint8_t *>(::malloc(buffSize));
    if (!dstBuff) {
        std::flush(std::cerr) << "Can't allocate memory." << std::endl;
        return __LINE__;
    }
    
    strm.next_in = _in_file_buff;
    strm.avail_in = _in_file_size;
    strm.next_out = dstBuff;
    strm.avail_out = buffSize;
    
    ret = lzma_code(&strm, LZMA_RUN);
    if (ret != LZMA_OK) {
        std::flush(std::cerr) << "Can't code stream." << std::endl;
        lzma_end(&strm);
        ::free(dstBuff);
        return __LINE__;
    }
    
    ret = lzma_code(&strm, LZMA_FINISH);
    
    if ((ret != LZMA_OK) && (ret != LZMA_STREAM_END)) {
        lzma_end(&strm);
        ::free(dstBuff);
        std::flush(std::cerr) << "Can't finish stream." << std::endl;
        return __LINE__;
    }
    
    const size_t dstSize = strm.total_out;
    lzma_end(&strm);
    
    {
        TPL3Fl<size_t> tpl;
        std::vector<uint8_t> test = tpl.vectorize(dstBuff, dstSize, _in_file_size);
        if (test.size() != _in_file_size || ::memcmp(test.data(), _in_file_buff, _in_file_size)) {
            std::flush(std::cerr) << "ERROR: Test lzma result." << std::endl;
            return __LINE__;
        }
    }
    
    write_output_result(3, "lzma", dstBuff, dstSize);
    
    ::free(dstBuff);
    return 0;
}

int write_as_is(void) {
    return write_output_result(0, "as_is", _in_file_buff, _in_file_size);
}

void cleanup(void) {
    if (_in_file_buff) {
        free(_in_file_buff);
        _in_file_buff = NULL;
    }
}

void print_help(void) {
    std::flush(std::cout) << std::endl << "Usage:" << std::endl;
    std::flush(std::cout) << "ctohex [OPTIONS] [INPUT FILE]" << std::endl;
    std::flush(std::cout) << "OPTIONS:" << std::endl;
    std::flush(std::cout) << " -p  Print original and output buffers" << std::endl;
    std::flush(std::cout) << " -d  Generate double include header" << std::endl;
    std::flush(std::cout) << " -w  Add simple 'write to file' function" << std::endl;
    std::flush(std::cout) << " -n  Add clang/gcc nullability" << std::endl;
    std::flush(std::cout) << " -r  Add pointer variable to output buffer" << std::endl;
    std::flush(std::cout) << std::endl;
}

bool sort_result_comparator(const ResultInfo & a, const ResultInfo & b) {
    return (a.size < b.size);
}

void process_and_print_results(void) {
    std::flush(std::cout);
    std::sort(_results.begin(), _results.end(), sort_result_comparator);
    
    size_t maxAlgoLen = 0, maxSizeLen = 0;
    for (size_t i = 0, n = _results.size(); i < n; i++) {
        const ResultInfo & info = _results[i];
        maxAlgoLen = std::max(maxAlgoLen, info.algo.size());
        maxSizeLen = std::max(maxSizeLen, info.sizeLen);
    }
    
    for (size_t i = 0, n = _results.size(); i < n; i++) {
        const ResultInfo & info = _results[i];
        std::flush(std::cout) << "[" << info.index << "] " << info.size;
        for (size_t j = info.sizeLen, m = maxSizeLen + 1; j < m; j++) {
            std::flush(std::cout) << " ";
        }
        std::flush(std::cout) << info.algo;
        for (size_t j = info.algo.size(), m = maxAlgoLen + 1; j < m; j++) {
            std::flush(std::cout) << " ";
        }
        std::flush(std::cout) << info.path << std::endl;
    }
}

int main(int argc, const char * argv[]) {
    for (int i = 1; i < argc; i++) {
        const char * arg = argv[i];
        
        if (::strcmp(arg, "--help") == 0) {
            print_help();
            return 0;
        }
        
        if (i == (argc - 1)) {
            const char * s1 = arg;
            //const char * s1 = "config.json";
            char * s2 = _lower_case_file_name, * s3 = _original_file_name;
            while (*s1) {
                switch (*s1) {
                    case '.':
                    case ' ':
                    case '/':
                    case '\\':
                    case '-':
                        *s2++ = '_';
                        break;
                    default:
                        *s2++ = (char)::tolower(*s1);
                        break;
                }
                *s3++ = *s1++;
            }
            continue;
        }
        
        const size_t argLen = ::strlen(arg);
        if ((argLen > 1) && (arg[0] == '-')) {
            for (size_t j = 1; j < argLen; j++) {
                const char option = arg[j];
                if (option == 'p') {
                    bool isSpecificOutput = false;
                    for (size_t k = j + 1; k < argLen; k++) {
                        const char numChar = arg[k];
                        if (numChar == '0') { isSpecificOutput = _print_output0 = true; }
                        else if (numChar == '1') { isSpecificOutput = _print_output1 = true; }
                        else if (numChar == '2') { isSpecificOutput = _print_output2 = true; }
                        else if (numChar == '3') { isSpecificOutput = _print_output3 = true; }
                        else { k = argLen; }
                    }
                    if (!isSpecificOutput) {
                        _print_output0 = _print_output1 = _print_output2 = _print_output3 = true;
                    }
                } else if (option == 'd') {
                    _generate_double_include_header = true;
                } else if (option == 'w') {
                    _add_write_to_file_function = true;
                } else if (option == 'n') {
                    _add_clang_gcc_nullability = true;
                } else if (option == 'r') {
                    _add_pointer = true;
                }
                
            }
            
            //continue;
        }
        
    }
    
    if (::strlen(_original_file_name) == 0 || ::strlen(_lower_case_file_name) == 0) {
        print_help();
        return 0;
    }
    
    //sprintf(_original_file_name, "");
    //sprintf(_lower_case_file_name, "");
    
    FILE * inFile = ::fopen(_original_file_name, "rb");
    if (!inFile) {
        cleanup();
        std::flush(std::cerr) << "Can't open file for reading." << std::endl;
        return 1;
    }
    ::fseek(inFile, 0, SEEK_END);
    _in_file_size = ::ftell(inFile);
    ::fseek(inFile, 0, SEEK_SET);
    _in_file_buff = static_cast<uint8_t *>(::malloc(_in_file_size));
    size_t srcSize = ::fread(_in_file_buff, 1, _in_file_size, inFile);
    ::fclose(inFile);
    if (srcSize != _in_file_size) {
        cleanup();
        std::flush(std::cerr) << "Can't read file." << std::endl;
        return 1;
    }
    
    _results.reserve(4);
    
    int res;
    if ( (res = write_as_is()) != 0) {
        cleanup();
        return res;
    }
    
    if ( (res = write_as_zlib()) != 0) {
        cleanup();
        return res;
    }
    
    if ( (res = write_as_zstd()) != 0) {
        cleanup();
        return res;
    }
    
    if ( (res = write_as_lzma()) != 0) {
        cleanup();
        return res;
    }
    
    cleanup();
    std::flush(std::cout) << std::endl << "Done" << std::endl;
    
    process_and_print_results();
    std::flush(std::cout) << std::endl;
    
    return 0;
}
