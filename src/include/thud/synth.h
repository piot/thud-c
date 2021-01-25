/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef THUD_EXAMPLE_INCLUDE_THUD_SYNTH_H
#define THUD_EXAMPLE_INCLUDE_THUD_SYNTH_H

#include <thunder/audio_node.h>

struct ThudSample;

typedef struct ThudVoice {
    const int16_t* stereoSamples;
    size_t sampleCount;
    size_t index;
    int isPlaying;
    size_t wait;
    int keyIsPressed;
    int startedAtTime;
} ThudVoice;

typedef struct ThudSynth {
    thunder_audio_node stereo;
    ThudVoice voices[8];
    size_t voiceCount;
    int time;
} ThudSynth;

void thudSynthInit(ThudSynth* self);
void thudSynthPressVoice(ThudSynth* self, int index, const struct ThudSample* sample);
void thudSynthReleaseVoice(ThudSynth* self, int index);
int thudSynthFindLeastUsedVoice(ThudSynth* self);

#endif // THUD_EXAMPLE_INCLUDE_THUD_SYNTH_H
