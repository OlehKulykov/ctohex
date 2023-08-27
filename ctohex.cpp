
#include <exception>
#include <string>
#include <vector>

#include "tpl1fl.hpp"
#include "tpl2fl.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include <zstd.h>

char _original_file_name[1024] = { 0 };
char _lower_case_file_name[1024] = { 0 };
uint8_t * _in_file_buff = NULL;
long long _in_file_size = 0;

void write_str(FILE * f, const char * str) {
    fwrite(str, strlen(str), 1, f);
}

void write_file_buff(FILE * f, const uint8_t * buff, long long buffSize) {
    char str1[1024];
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
        const int strLen = sprintf(str1, format,
                                   buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],
                                   buff[8],buff[9],buff[10],buff[11],buff[12],buff[13],buff[14],buff[15]);
        fwrite(str1, strLen, 1, f);
        written += 16;
        buff += 16;
    }
    if (written > 0 && written < buffSize) {
        fwrite(",\n", 2, 1, f);
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
        const int strLen = sprintf(str1, format, *buff);
        fwrite(str1, strLen, 1, f);
        buff++;
        written++;
    }
}

int write_as_zlib(void) {
    uLong buffSize = compressBound((uLong)_in_file_size);
    buffSize += 256 * 1024;
    uint8_t * dstBuff = (uint8_t *)malloc(buffSize);
    if (!dstBuff) {
        printf("Can't allocate memory.\n");
        return 1;
    }
    uLong dstSize = buffSize;
    int res = compress2(dstBuff, &dstSize, _in_file_buff, _in_file_size, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        free(dstBuff);
        return 1;
    }
    
    TPL1Fl<size_t> tpl;
    std::vector<uint8_t> test = tpl.vectorize(dstBuff, dstSize, _in_file_size);
    if (test.size() != _in_file_size || memcmp(test.data(), _in_file_buff, _in_file_size)) {
        printf("ERROR: Test zlib result.\n");
        return 1;
    }
    test.clear();
    
    char str1[4096];
    sprintf(str1, "file__%s_1.h", _lower_case_file_name);
    FILE * outFile = fopen(str1, "w+b");
    if (!outFile) {
        free(dstBuff);
        printf("Can't open output file.\n");
        return 1;
    }
    
    sprintf(str1, "//1 %s\n", _original_file_name);
    write_str(outFile, str1);
    
    sprintf(str1, "#ifndef FILE__%s_SIZE\n#define FILE__%s_SIZE %lli\n", _lower_case_file_name, _lower_case_file_name, _in_file_size);
    write_str(outFile, str1);
    
    sprintf(str1, "#define FILE__%s_SIZE_C %llu\n", _lower_case_file_name, (unsigned long long)dstSize);
    write_str(outFile, str1);
    
    sprintf(str1, "static unsigned char FILE__%s[%llu]={\n", _lower_case_file_name, (unsigned long long)dstSize);
    write_str(outFile, str1);
    
    write_file_buff(outFile, dstBuff, dstSize);
    
    write_str(outFile, "};\n#endif\n");

//    printf("--- zlib:\n");
//    for (uLong i = 0; i < dstSize; i++) { printf("%c", (char)dstBuff[i]); }
//    printf("\n");
    
    fclose(outFile);
    free(dstBuff);
    return 0;
}

int write_as_zstd(void) {
    ZSTD_CCtx * cctx = ZSTD_createCCtx();
    size_t buffSize = ZSTD_compressBound(_in_file_size);
    buffSize += 256 * 1024;
    uint8_t * dstBuff = (uint8_t *)malloc(buffSize);
    if (!dstBuff) {
        printf("Can't allocate memory.\n");
        return 1;
    }
    size_t dstSize = ZSTD_compressCCtx(cctx,
                                       dstBuff, buffSize,
                                       _in_file_buff, _in_file_size,
                                       19);
    if (ZSTD_isError(dstSize)) {
        free(dstBuff);
        ZSTD_freeCCtx(cctx);
        return 1;
    }
    
    TPL2Fl<size_t> tpl;
    std::vector<uint8_t> test = tpl.vectorize(dstBuff, dstSize, _in_file_size);
    if (test.size() != _in_file_size || memcmp(test.data(), _in_file_buff, _in_file_size)) {
        printf("ERROR: Test zstd result.\n");
        return 1;
    }
    test.clear();
    
    char str1[4096];
    sprintf(str1, "file__%s_2.h", _lower_case_file_name);
    FILE * outFile = fopen(str1, "w+b");
    if (!outFile) {
        free(dstBuff);
        ZSTD_freeCCtx(cctx);
        printf("Can't open output file.\n");
        return 1;
    }
    
    sprintf(str1, "//2 %s\n", _original_file_name);
    write_str(outFile, str1);
    
    sprintf(str1, "#ifndef FILE__%s_SIZE\n#define FILE__%s_SIZE %lli\n", _lower_case_file_name, _lower_case_file_name, _in_file_size);
    write_str(outFile, str1);
    
    sprintf(str1, "#define FILE__%s_SIZE_C %llu\n", _lower_case_file_name, (unsigned long long)dstSize);
    write_str(outFile, str1);
    
    sprintf(str1, "static unsigned char FILE__%s[%llu]={\n", _lower_case_file_name, (unsigned long long)dstSize);
    write_str(outFile, str1);
    
    write_file_buff(outFile, dstBuff, dstSize);
    
    write_str(outFile, "};\n#endif\n");
    
//    printf("--- zstd:\n");
//    for (size_t i = 0; i < dstSize; i++) { printf("%c", (char)dstBuff[i]); }
//    printf("\n");
    
    fclose(outFile);
    free(dstBuff);
    ZSTD_freeCCtx(cctx);
    return 0;
}

int write_as_is(void) {
    char str1[4096];
    sprintf(str1, "file__%s.h", _lower_case_file_name);
    FILE * outFile = fopen(str1, "w+b");
    if (!outFile) {
        printf("Can't open output file.\n");
        return 1;
    }
    
    sprintf(str1, "//0 %s\n", _original_file_name);
    write_str(outFile, str1);
    
    sprintf(str1, "#ifndef FILE__%s_SIZE\n#define FILE__%s_SIZE %lli\n", _lower_case_file_name, _lower_case_file_name, _in_file_size);
    write_str(outFile, str1);
    
    sprintf(str1, "static unsigned char FILE__%s[%lli]={\n", _lower_case_file_name, _in_file_size);
    write_str(outFile, str1);
    
    write_file_buff(outFile, _in_file_buff, _in_file_size);
    
    write_str(outFile, "};\n#endif\n");
    
    fclose(outFile);
    return 0;
}

void cleanup(void) {
    if (_in_file_buff) {
        free(_in_file_buff);
        _in_file_buff = NULL;
    }
}

int main(int argc, const char * argv[]) {
    for (int i = 0; i < argc; i++) {
        switch (i) {
            case 1: {
                const char * s1 = argv[i];
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
                            *s2++ = (char)tolower(*s1);
                            break;
                    }
                    *s3++ = *s1++;
                }
                break;
            }
            
            default:
                break;
        }
    }
    
    if (strlen(_original_file_name) == 0 || strlen(_lower_case_file_name) == 0) {
        cleanup();
        printf("No input file.\n");
        return 0;
    }
    
    //sprintf(_original_file_name, "");
    //sprintf(_lower_case_file_name, "");
    
    FILE * inFile = fopen(_original_file_name, "rb");
    if (!inFile) {
        cleanup();
        printf("Can't open file for reading.\n");
        return 1;
    }
    fseek(inFile, 0, SEEK_END);
    _in_file_size = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    _in_file_buff = (uint8_t *)malloc(_in_file_size);
    size_t srcSize = fread(_in_file_buff, 1, _in_file_size, inFile);
    fclose(inFile);
    if (srcSize != _in_file_size) {
        cleanup();
        printf("Can't read file.\n");
        return 1;
    }
    
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
    
    cleanup();
    printf("Done\n");
    return 0;
}
