/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
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
                if (voice->stereoSamples != 0) {
                    voice->wait++;
                    if (voice->wait > 32000) {
                        voice->wait = 0;
                        voice->isPlaying = 1;
                    }
                }
                continue;
            }
            accumulator += voice->stereoSamples[voice->index];
            voice->index++;
            if (voice->index >= voice->sampleCount) {
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
    for (size_t i = 0; i < self->voiceCount; ++i) {
        self->voices[i].isPlaying = 0;
        self->voices[i].wait = 0;
        self->voices[i].index = 0;
    }
}

void thudSynthSetVoice(ThudSynth* self, int index, const ThudSample* sample)
{
    ThudVoice* voice = &self->voices[index];
    voice->sampleCount = sample->sampleCount;
    voice->index = 0;
    voice->isPlaying = 1;
    voice->stereoSamples = sample->samples;
    self->voiceCount = 8;
}