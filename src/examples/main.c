/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <thud/thud.h>

#include <clog/clog.h>
#include <clog/console.h>

clog_config g_clog;

static int readWholeFile(const char* filename, const uint8_t** outOctets, size_t* outSize)
{
    uint8_t* source = 0;
    FILE* fp = fopen(filename, "rb");
    if (fp == 0) {
        return -1;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        return -2;
    }

    long bufSize = ftell(fp);
    if (bufSize == -1) {
        return -3;
    }

    source = tc_malloc_type_count(uint8_t, bufSize);
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        return -4;
    }

    fread(source, sizeof(uint8_t), bufSize, fp);
    fclose(fp);

    *outOctets = source;
    *outSize = bufSize;

    CLOG_VERBOSE("read file '%s', octets: %ld", filename, bufSize);

    return 0;
}

int main(int argc, char* argv[])
{
    g_clog.log = clog_console;

    const uint8_t* opusOctets;
    size_t opusSize;

    int errorCode = readWholeFile("multiball.opus", &opusOctets, &opusSize);
    if (errorCode < 0) {
        CLOG_VERBOSE("can not find multiball");
        return errorCode;
    }

    ThudSample sample;
    errorCode = thudLoadSample(&sample, opusOctets, opusSize);
    if (errorCode < 0) {
        CLOG_VERBOSE("can not convert opus data");
        return errorCode;
    }

    return 0;
}
