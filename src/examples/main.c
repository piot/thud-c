/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <clog/console.h>
#include <imprint/memory.h>
#include <thud/synth.h>
#include <thud/thud.h>
#include <thunder/sound_module.h>
#include <unistd.h>

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

    thunder_sound_module soundModule;
    imprint_memory memory;
    imprint_memory_init(&memory, 256 * 1024, "thunder sound module");

    thunder_sound_module_init(&soundModule, &memory);

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

    ThudSynth synth;

    thudSynthInit(&synth);
    thudSynthSetVoice(&synth, 0, &sample);

    soundModule.compositor.nodes[0] = synth.stereo;
    soundModule.compositor.nodes[0].is_playing = 1;
    soundModule.compositor.nodes[0].channel_count = 2;
    soundModule.compositor.nodes[0].volume = 1.0;
    soundModule.compositor.nodes_count = 1;

    while (1) {
        thunder_sound_module_update(&soundModule);
        usleep(1000);
    }

    return 0;
}
