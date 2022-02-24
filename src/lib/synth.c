/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <limits.h>
#include <thud/synth.h>
#include <thud/thud.h>

// thunder_audio_node_output_func
static void thudSynth(void* _self, thunder_sample* sample, int sample_count)
{
    ThudSynth* self = (ThudSynth*) _self;

    thunder_sample* p = sample;

    int divisor = 0;

    for (size_t i = 0; i < sample_count * 2; ++i) {
        thunder_mix_sample accumulator = 0;
        divisor = 0;
        for (size_t voiceId = 0; voiceId < self->voiceCapacity; ++voiceId) {
            if (self->voiceCapacity > 3) {
                CLOG_ERROR("break")
            }
            ThudVoice* voice = &self->voices[voiceId];

            if (voice->isPlaying != 0 &&voice->isPlaying != 1) {
                CLOG_ERROR("break. isplaying is wrong")
            }

            if (!voice->isPlaying) {
                continue;
            }
            divisor++;
            accumulator += voice->stereoSamples[voice->index];
            voice->index++;
            if (voice->index >= voice->sampleCountInMono) {
                voice->index = 0;
                voice->isPlaying = 0;
                if (voice->isLooping && voice->keyIsPressed) {
                    voice->isPlaying = 1;
                }
            }
        }

        *p++ = divisor == 0 ? 0 : accumulator / divisor;
    }
}

void thudSynthInit(ThudSynth* self)
{
    thunder_audio_node_init(&self->stereo, thudSynth, self);
    self->stereo.is_playing = 1;
    self->stereo.channel_count = 2;
    self->stereo.volume = 0.3f;
    self->voiceCapacity = 3;
    self->time = 0;
    for (size_t i = 0; i < self->voiceCapacity; ++i) {
        self->voices[i].isPlaying = 0;
        self->voices[i].index = 0;
        self->voices[i].keyIsPressed = 0;
    }

}

void thudSynthPressVoice(ThudSynth* self, size_t index, const ThudSample* sample)
{
    //CLOG_VERBOSE("pressing voice %d", index);
    if (index >= self->voiceCapacity) {
        CLOG_ERROR("overwrite voices");
        return;
    }
    ThudVoice* voice = &self->voices[index];
    voice->sampleCountInMono = sample->sampleCount * sample->channelCount;
    voice->index = 0;
    voice->isPlaying = (voice->sampleCountInMono > 0);
    voice->stereoSamples = sample->samples;
    voice->keyIsPressed = 1;
    voice->startedAtTime = self->time++;
    voice->isLooping = 1;

    if (voice->isPlaying != 0 && voice->isPlaying != 1) {
        CLOG_ERROR("isplayiung is wrong after set");
    }

    //CLOG_VERBOSE("sampleCount:%d", voice->sampleCountInMono);
}

int thudSynthFindLeastUsedVoice(ThudSynth* self)
{
    for (size_t i = 0; i < self->voiceCapacity; ++i) {
        const ThudVoice* voice = &self->voices[i];
        if (!voice->isPlaying && !voice->keyIsPressed) {
            return i;
        }
    }

    uint64_t lowestTime = INT64_MAX;
    int bestVoice = -1;
    for (size_t i = 0; i < self->voiceCapacity; ++i) {
        const ThudVoice* voice = &self->voices[i];
        if (voice->startedAtTime < lowestTime) {
            bestVoice = i;
            lowestTime = voice->startedAtTime;
        }
    }

    return bestVoice;
}

ThudVoiceInstanceHandle thudSynthKeyDown(ThudSynth* self, const struct ThudSample* sample, const ThudVoiceInfo* info)
{
    int voiceIndex = thudSynthFindLeastUsedVoice(self);
    //CLOG_VERBOSE("found free voice %d", voiceIndex);
    ThudVoice* voice = &self->voices[voiceIndex];

    thudSynthPressVoice(self, voiceIndex, sample);

    if (info->loopCount == 0) {
        voice->isLooping = 0;
    } else {
        voice->isLooping = 1;
    }

    return ((voice->startedAtTime & 0xff) << 8 | (voiceIndex & 0xff));
}

void thudSynthKeyUp(ThudSynth* self, ThudVoiceInstanceHandle handle)
{
    if (handle == THUD_ILLEGAL_VOICE_INSTANCE) {
        return;
    }

    size_t voiceIndex = handle & 0xff;
    int timeSuffix = (handle >> 8) & 0xff;

    if (voiceIndex >= self->voiceCapacity) {
        CLOG_ERROR("overwrite voices")
        return;
    }

    ThudVoice* voice = &self->voices[voiceIndex];
    if (voice->keyIsPressed) {
        if ((voice->startedAtTime & 0xff) != timeSuffix) {
            CLOG_WARN("this voice is not around anymore")
            return;
        }

        voice->keyIsPressed = 0;
    }
}

void thudSynthReleaseVoice(ThudSynth* self, size_t voiceIndex)
{
    if (voiceIndex >= self->voiceCapacity) {
        CLOG_ERROR("overwrite voices");
        return;
    }
    ThudVoice* voice = &self->voices[voiceIndex];
    voice->keyIsPressed = 0;
}
