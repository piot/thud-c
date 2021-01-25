/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <limits.h>
#include <thud/synth.h>
#include <thud/thud.h>

// thunder_audio_node_output_func
void thudSynth(void* _self, thunder_sample* sample, int sample_count)
{
    ThudSynth* self = (ThudSynth*) _self;

    thunder_sample* p = sample;

    for (size_t i = 0; i < sample_count * 2; ++i) {
        thunder_mix_sample accumulator = 0;
        for (size_t voiceId = 0; voiceId < self->voiceCount; ++voiceId) {
            ThudVoice* voice = &self->voices[voiceId];
            if (!voice->isPlaying) {
                if (voice->stereoSamples != 0 && voice->keyIsPressed) {
                    voice->wait++;
                    if (voice->wait > 52000) {
                        voice->wait = 0;
                        voice->isPlaying = 1;
                    }
                }
                continue;
            }
            accumulator += voice->stereoSamples[voice->index];
            voice->index++;
            if (voice->index >= voice->sampleCount * 2) {
                voice->index = 0;
                voice->isPlaying = 0;
                voice->wait = 0;
            }
        }
        *p++ = accumulator / 2; // divide by number of playing voices
    }
}

void thudSynthInit(ThudSynth* self)
{
    thunder_audio_node_init(&self->stereo, thudSynth, self);
    self->voiceCount = 8;
    self->time = 0;
    for (size_t i = 0; i < self->voiceCount; ++i) {
        self->voices[i].isPlaying = 0;
        self->voices[i].wait = 0;
        self->voices[i].index = 0;
        self->voices[i].keyIsPressed = 0;
    }
}

void thudSynthPressVoice(ThudSynth* self, int index, const ThudSample* sample)
{
    ThudVoice* voice = &self->voices[index];
    voice->sampleCount = sample->sampleCount;
    voice->index = 0;
    voice->isPlaying = 1;
    voice->stereoSamples = sample->samples;
    voice->keyIsPressed = 1;
    voice->startedAtTime = self->time++;
}

int thudSynthFindLeastUsedVoice(ThudSynth* self)
{
    for (size_t i = 0; i < self->voiceCount; ++i) {
        const ThudVoice* voice = &self->voices[i];
        if (!voice->isPlaying && !voice->keyIsPressed) {
            return i;
        }
    }

    int lowestTime = INT_MAX;
    int bestVoice = -1;
    for (size_t i = 0; i < self->voiceCount; ++i) {
        const ThudVoice* voice = &self->voices[i];
        if (voice->startedAtTime < lowestTime) {
            bestVoice = i;
            lowestTime = 0;
            return i;
        }
    }

    return bestVoice;
}

void thudSynthReleaseVoice(ThudSynth* self, int index)
{
    ThudVoice* voice = &self->voices[index];
    voice->keyIsPressed = 0;
}