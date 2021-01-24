#include <monsoon/monsoon.h>
#include <thud/thud.h>

int thudCreateSound(Thud* self, const uint8_t* data, size_t octetCount)
{
    Monsoon monsoon;

    int errorCode = monsoonInit(&monsoon, 48000, data, octetCount);

    const int tempSize = 32 * 1024;
    int16_t tempSampleTarget[tempSize];

    int decodedSamplesInStereo = monsoonDecode(&monsoon, tempSampleTarget, tempSize);

    return decodedSamplesInStereo;
}
