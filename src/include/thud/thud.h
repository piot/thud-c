/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef THUD_H
#define THUD_H

#include <stddef.h>
#include <stdint.h>

typedef struct ThudSample {
    const int16_t* samples;
    size_t sampleCount;
    size_t channelCount;
} ThudSample;

int thudLoadSample(ThudSample* self, const uint8_t* data, size_t octetCount);

typedef struct Thud {
    ThudSample samples[64];
} Thud;

int thudInit(Thud* self);
int thudUpdate(Thud* self);

#endif
