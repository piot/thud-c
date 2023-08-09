/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef THUD_SYNTH_H
#define THUD_SYNTH_H

#include <stddef.h>
#include <thunder/audio_node.h>

struct ThudSample;
typedef uint16_t ThudVoiceInstanceHandle;

#define THUD_ILLEGAL_VOICE_INSTANCE (0xffff)

typedef struct ThudVoice {
    const int16_t* stereoSamples;
    size_t sampleCountInMono;
    size_t index;
    int isPlaying;
    int keyIsPressed;
    int isLooping;
    uint64_t startedAtTime;
} ThudVoice;

typedef struct ThudSynth {
    ThunderAudioNode stereo;
    ThudVoice voices[8];
    size_t voiceCapacity;
    uint64_t time;
} ThudSynth;

typedef struct ThudVoiceInfo {
    int loopCount;
} ThudVoiceInfo;

void thudSynthInit(ThudSynth* self);

void thudSynthPressVoice(ThudSynth* self, size_t index, const struct ThudSample* sample);
void thudSynthReleaseVoice(ThudSynth* self, size_t index);

ThudVoiceInstanceHandle thudSynthKeyDown(ThudSynth* self, const struct ThudSample* sample, const ThudVoiceInfo* info);
void thudSynthKeyUp(ThudSynth* self, ThudVoiceInstanceHandle handle);

size_t thudSynthFindLeastUsedVoice(ThudSynth* self);

#endif
