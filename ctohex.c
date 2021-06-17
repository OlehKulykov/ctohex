
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
            format = "0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x";
        } else {
            format = ",\n0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x";
        }
        first = 0;
        const int strLen = sprintf(str1, format,
                                   buff[0],buff[1],buff[2],buff[3],buff[4],buff[5],buff[6],buff[7],
                                   buff[8],buff[9],buff[10],buff[11],buff[12],buff[13],buff[14],buff[15]);
        fwrite(str1, strLen, 1, f);
        written += 16;
        buff += 16;
    }
    if (written > 0) {
        fwrite(",\n", 2, 1, f);
    }
    first = 1;
    while (written < buffSize) {
        const char * format;
        if (first) {
            format = "0x%02x";
        } else {
            format = ",0x%02x";
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
    uLong dstSize = buffSize;
    int res = compress2(dstBuff, &dstSize, _in_file_buff, _in_file_size, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        free(dstBuff);
        return 2;
    }
    
    char str1[1024];
    char str2[1024];
    char str3[128];
    char str4[128];
    sprintf(str1, "file__%s_1.h", _lower_case_file_name);
    FILE * outFile = fopen(str1, "w+b");
    
    sprintf(str3, "FILE__%s_SIZE", _lower_case_file_name);
    sprintf(str4, "FILE__%s_C_SIZE", _lower_case_file_name);
    
    sprintf(str2, "// %s\n", _original_file_name);
    write_str(outFile, str2);
    
    sprintf(str1, "#ifndef %s\n", str3);
    write_str(outFile, str1);
    
    sprintf(str1, "#define %s %lli\n", str3, _in_file_size);
    write_str(outFile, str1);
    sprintf(str1, "#define %s %llu\n", str4, (unsigned long long)dstSize);
    write_str(outFile, str1);
    
    write_str(outFile, "#if defined(__cplusplus)\n");
    
    sprintf(str2, "FILE__%s", _lower_case_file_name);
    sprintf(str1, "extern \"C\" unsigned char %s[%s];\n", str2, str4);
    write_str(outFile, str1);
    
    //sprintf(str1, "extern \"C\" bool %s_write_to_path(const char *);\n", str2);
    //write_str(outFile, str1);
    
    write_str(outFile, "#else\n");
    
    sprintf(str2, "FILE__%s", _lower_case_file_name);
    sprintf(str1, "#include <stdbool.h>\nunsigned char %s[%s];\n", str2, str4);
    write_str(outFile, str1);

//    sprintf(str1, "extern bool %s_copy_to_buff(const char *);\n", str2);
//    write_str(outFile, str1);
    
    write_str(outFile, "#endif\n");
    
    write_str(outFile, "#else\n");
    sprintf(str1, "unsigned char %s[%s]={\n", str2, str4);
    write_str(outFile, str1);
    
    write_file_buff(outFile, dstBuff, dstSize);
    
//    sprintf(str2, "FILE__%s", _lower_case_file_name);
//    sprintf(str1, "};\nunsigned char * %s_PTR=%s;\n", str2, str2);
    write_str(outFile, "};\n#endif\n\n");
    
//    sprintf(str1, "bool %s_write_to_path(const char * path) {\n", str2);
//    write_str(outFile, str1);
//
//    write_str(outFile, "    FILE * f = fopen(path, \"w+b\");\n    if (f) {\n");
//
//    sprintf(str1, "        const size_t w = fwrite(%s_PTR, 1, %s_SIZE, f);\n", str2, str2);
//    write_str(outFile, str1);
//
//    write_str(outFile, "        fclose(f);\n");
//
//    sprintf(str1, "        return w == %s_SIZE;\n", str2);
//    write_str(outFile, str1);
//
//    write_str(outFile, "    }\n    return false;\n}\n#endif\n\n");
    
    write_str(outFile, "\n\n");
    
    fclose(outFile);
    
    free(dstBuff);
    return 0;
}

int write_as_zstd(void) {
    ZSTD_CCtx * cctx = ZSTD_createCCtx();
    size_t buffSize = ZSTD_compressBound(_in_file_size);
    buffSize += 256 * 1024;
    uint8_t * dstBuff = (uint8_t *)malloc(buffSize);
    size_t dstSize = ZSTD_compressCCtx(cctx,
                                       dstBuff, buffSize,
                                       _in_file_buff, _in_file_size,
                                       19);
    if (ZSTD_isError(dstSize)) {
        ZSTD_freeCCtx(cctx);
        return 3;
    }
    
    char str1[1024];
    char str2[1024];
    char str3[128];
    char str4[128];
    sprintf(str1, "file__%s_2.h", _lower_case_file_name);
    FILE * outFile = fopen(str1, "w+b");
    
    sprintf(str3, "FILE__%s_SIZE", _lower_case_file_name);
    sprintf(str4, "FILE__%s_C_SIZE", _lower_case_file_name);
    
    sprintf(str2, "// %s\n", _original_file_name);
    write_str(outFile, str2);
    
    sprintf(str1, "#ifndef %s\n", str3);
    write_str(outFile, str1);
    
    sprintf(str1, "#define %s %lli\n", str3, _in_file_size);
    write_str(outFile, str1);
    sprintf(str1, "#define %s %llu\n", str4, (unsigned long long)dstSize);
    write_str(outFile, str1);
    
    write_str(outFile, "#if defined(__cplusplus)\n");
    
    sprintf(str2, "FILE__%s", _lower_case_file_name);
    sprintf(str1, "extern \"C\" unsigned char %s[%s];\n", str2, str4);
    write_str(outFile, str1);
    
    //sprintf(str1, "extern \"C\" bool %s_write_to_path(const char *);\n", str2);
    //write_str(outFile, str1);
    
    write_str(outFile, "#else\n");
    
    sprintf(str2, "FILE__%s", _lower_case_file_name);
    sprintf(str1, "#include <stdbool.h>\nunsigned char %s[%s];\n", str2, str4);
    write_str(outFile, str1);

//    sprintf(str1, "extern bool %s_copy_to_buff(const char *);\n", str2);
//    write_str(outFile, str1);
    
    write_str(outFile, "#endif\n");
    
    write_str(outFile, "#else\n");
    sprintf(str1, "unsigned char %s[%s]={\n", str2, str4);
    write_str(outFile, str1);
    
    write_file_buff(outFile, dstBuff, dstSize);
    
//    sprintf(str2, "FILE__%s", _lower_case_file_name);
//    sprintf(str1, "};\nunsigned char * %s_PTR=%s;\n", str2, str2);
    write_str(outFile, "};\n#endif\n\n");
    
//    sprintf(str1, "bool %s_write_to_path(const char * path) {\n", str2);
//    write_str(outFile, str1);
//
//    write_str(outFile, "    FILE * f = fopen(path, \"w+b\");\n    if (f) {\n");
//
//    sprintf(str1, "        const size_t w = fwrite(%s_PTR, 1, %s_SIZE, f);\n", str2, str2);
//    write_str(outFile, str1);
//
//    write_str(outFile, "        fclose(f);\n");
//
//    sprintf(str1, "        return w == %s_SIZE;\n", str2);
//    write_str(outFile, str1);
//
//    write_str(outFile, "    }\n    return false;\n}\n#endif\n\n");
    
    write_str(outFile, "\n\n");
    
    
    fclose(outFile);
    free(dstBuff);
    ZSTD_freeCCtx(cctx);
    return 0;
}

int write_as_is(void) {
    char str1[1024];
    char str2[1024];
    char str3[128];
    sprintf(str1, "file__%s.h", _lower_case_file_name);
    FILE * outFile = fopen(str1, "w+b");
    
    sprintf(str3, "FILE__%s_SIZE", _lower_case_file_name);
    
    sprintf(str2, "// %s\n", _original_file_name);
    write_str(outFile, str2);
    
    sprintf(str1, "#ifndef %s\n", str3);
    write_str(outFile, str1);
    
    sprintf(str1, "#define %s %lli\n", str3, _in_file_size);
    write_str(outFile, str1);
    
    write_str(outFile, "#if defined(__cplusplus)\n");
    
    sprintf(str2, "FILE__%s", _lower_case_file_name);
    sprintf(str1, "extern \"C\" unsigned char %s[%s];\nextern \"C\" unsigned char * %s_PTR;\n", str2, str3, str2);
    write_str(outFile, str1);
    
    sprintf(str1, "extern \"C\" bool %s_write_to_path(const char *);\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "#else\n");
    
    sprintf(str2, "FILE__%s", _lower_case_file_name);
    sprintf(str1, "#include <stdbool.h>\nextern unsigned char %s[%s];\nextern unsigned char * %s_PTR;\n", str2, str3, str2);
    write_str(outFile, str1);

    sprintf(str1, "extern bool %s_write_to_path(const char *);\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "#endif\n");
    
    write_str(outFile, "#else\n");
    sprintf(str1, "unsigned char %s[%s]={\n", str2, str3);
    write_str(outFile, str1);
    
    write_file_buff(outFile, _in_file_buff, _in_file_size);
    
    sprintf(str2, "FILE__%s", _lower_case_file_name);
    sprintf(str1, "};\nunsigned char * %s_PTR=%s;\n", str2, str2);
    write_str(outFile, str1);
    
    sprintf(str1, "bool %s_write_to_path(const char * path) {\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "    FILE * f = fopen(path, \"w+b\");\n    if (f) {\n");
    
    sprintf(str1, "        const size_t w = fwrite(%s_PTR, 1, %s_SIZE, f);\n", str2, str2);
    write_str(outFile, str1);
    
    write_str(outFile, "        fclose(f);\n");
    
    sprintf(str1, "        return w == %s_SIZE;\n", str2);
    write_str(outFile, str1);
    
    write_str(outFile, "    }\n    return false;\n}\n#endif\n\n");
    
    fclose(outFile);
    return 0;
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
    
    FILE * inFile = fopen(_original_file_name, "rb");
    fseek(inFile, 0, SEEK_END);
    _in_file_size = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    _in_file_buff = (uint8_t *)malloc(_in_file_size);
    size_t srcSize = fread(_in_file_buff, 1, _in_file_size, inFile);
    fclose(inFile);
    if (srcSize != _in_file_size) {
        printf("Can't read file.\n");
        return 1;
    }
    
    int res;
    if ( (res = write_as_is()) != 0) {
        return res;
    }
    
    if ( (res = write_as_zlib()) != 0) {
        return res;
    }
    
    if ( (res = write_as_zstd()) != 0) {
        return res;
    }
    
    printf("Done\n");
    return 0;
}
