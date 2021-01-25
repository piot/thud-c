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

#include <fcntl.h>
#include <termios.h>

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

int readButton(int fd, unsigned char keys[8])
{
    unsigned char buf[16];

    fd_set set;
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&set);
    FD_SET(fd, &set);
    int res = select(fd + 1, &set, 0, 0, &tv);

    if (res > 0) {
        int n = read(fd, buf, sizeof(buf));
        for (int i = 0; i < n; i++) {
            unsigned char raw = buf[i];
            unsigned char key = raw & 0x7f;

            if (key == 96) {
                keys[0] = 1;
            }

            int index = (key & 0x7f) - 48;
            if (index >= 0 && index < 8) {
                keys[index] = !keys[index];
            }
        }
    }

    return res;
}

int turnOffEcho(int fd, struct termios* old)
{
    if (tcgetattr(fd, old) < 0) {
        return -1;
    }

    struct termios new;
    if (tcgetattr(fd, &new) < 0) {
        return -2;
    }

    new = *old;
    new.c_lflag &= ~(ICANON | ECHO | ISIG);
    new.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
    new.c_cc[VMIN] = 0;
    new.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &new) < 0) {
        return -3;
    }

    return 0;
}

int loadSamples(ThudSample samples[8], const char* names[8])
{
    const uint8_t* opusOctets;
    size_t opusSize;

    for (size_t i = 0; i < 8; ++i) {
        int errorCode = readWholeFile(names[i], &opusOctets, &opusSize);
        if (errorCode < 0) {
            CLOG_VERBOSE("can not find multiball");
            return errorCode;
        }

        errorCode = thudLoadSample(&samples[i], opusOctets, opusSize);
        if (errorCode < 0) {
            CLOG_VERBOSE("can not convert opus data");
            return errorCode;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    g_clog.log = clog_console;

    int fd = STDIN_FILENO;

    struct termios old;
    turnOffEcho(fd, &old);

    thunder_sound_module soundModule;
    imprint_memory memory;
    imprint_memory_init(&memory, 256 * 1024, "thunder sound module");

    thunder_sound_module_init(&soundModule, &memory);

    const char* names[8] = {"multiball.opus", "powerup.opus",   "two.opus",       "multiball.opus",
                            "multiball.opus", "multiball.opus", "multiball.opus", "multiball.opus"};

    ThudSample samples[8];

    loadSamples(samples, names);

    ThudSynth synth;

    thudSynthInit(&synth);

    soundModule.compositor.nodes[0] = synth.stereo;
    soundModule.compositor.nodes[0].is_playing = 1;
    soundModule.compositor.nodes[0].channel_count = 2;
    soundModule.compositor.nodes[0].volume = 1.0;
    soundModule.compositor.nodes_count = 1;

    unsigned char keys[8];
    unsigned char oldKeys[8];
    int usedVoice[8];

    memset(keys, 0, sizeof(keys));
    memset(oldKeys, 0, sizeof(keys));
    memset(usedVoice, 0, sizeof(keys));

    while (1) {
        int buttons = readButton(fd, keys);
        if (buttons > 0) {
            printf("keys: ");
            for (size_t i = 0; i < 8; ++i) {
                printf("%d", keys[i]);
            }
            printf("\n");
        }
        if (keys[0] != 0) {
            break;
        }

        for (size_t i = 1; i < 8; ++i) {
            int wentDown = !oldKeys[i] && keys[i];
            int wentUp = oldKeys[i] && !keys[i];
            if (wentDown) {
                int freeVoice = thudSynthFindLeastUsedVoice(&synth);
                usedVoice[i] = freeVoice;
                thudSynthPressVoice(&synth, freeVoice, &samples[i]);
            } else if (wentUp) {
                thudSynthReleaseVoice(&synth, usedVoice[i]);
            }
        }

        memcpy(oldKeys, keys, sizeof(oldKeys));

        thunder_sound_module_update(&soundModule);
        usleep(1000);
    }

    if (tcsetattr(fd, TCSANOW, &old) < 0) {
        return -5;
    }

    return 0;
}
