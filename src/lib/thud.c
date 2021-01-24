/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <monsoon/monsoon.h>
#include <thud/thud.h>
#include <thunder/audio_node.h>
#include <thunder/sound_buffer.h>
#include <tiny-libc/tiny_libc.h>

int thudLoadSample(ThudSample* self, const uint8_t* data, size_t octetCount)
{
    Monsoon monsoon;

    int errorCode = monsoonInit(&monsoon, 48000, data, octetCount);
    if (errorCode < 0) {
        return errorCode;
    }

    const int tempSize = 256 * 1024;
    int16_t tempSampleTarget[tempSize];

    int decodedSamplesInStereo = monsoonDecodeAll(&monsoon, tempSampleTarget, tempSize / 2);
    if (decodedSamplesInStereo <= 0) {
        CLOG_VERBOSE("could not decode %d", decodedSamplesInStereo);
        return decodedSamplesInStereo;
    }

    self->samples = tc_malloc_type_count(int16_t, decodedSamplesInStereo);
    self->sampleCount = decodedSamplesInStereo;
    tc_memcpy_type_n((int16_t*) self->samples, tempSampleTarget, decodedSamplesInStereo);

    CLOG_VERBOSE("loaded %d samples", decodedSamplesInStereo);

    return decodedSamplesInStereo;
}

int thudInit(Thud* self)
{
    return 0;
}

int thudUpdate()
{
    return 0;
}