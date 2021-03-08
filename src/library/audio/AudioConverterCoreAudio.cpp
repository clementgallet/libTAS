/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AudioConverterCoreAudio.h"
#include "../logging.h"
#include "hook.h"
#include <stdlib.h>
#include <stdint.h>

namespace libtas {

AudioConverterCoreAudio::~AudioConverterCoreAudio(void)
{
    if (converter)
        AudioConverterDispose(converter);
}

bool AudioConverterCoreAudio::isAvailable()
{
    return true;
}

bool AudioConverterCoreAudio::isInited()
{
    return converter;
}

void AudioConverterCoreAudio::init(AudioBuffer::SampleFormat inFormat, int inChannels, int inFreq, AudioBuffer::SampleFormat outFormat, int outChannels, int outFreq)
{
    in.mFormatID = kAudioFormatLinearPCM;
    in.mFormatFlags = kLinearPCMFormatFlagIsPacked;
    in.mChannelsPerFrame = inChannels;
    in.mSampleRate = inFreq;
    in.mFramesPerPacket = 1;
    switch (inFormat) {
        case AudioBuffer::SAMPLE_FMT_U8:
            in.mBitsPerChannel = 8;
            break;
        case AudioBuffer::SAMPLE_FMT_S16:
            in.mBitsPerChannel = 16;
            in.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
            break;
        case AudioBuffer::SAMPLE_FMT_S32:
            in.mBitsPerChannel = 32;
            in.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
            break;
        case AudioBuffer::SAMPLE_FMT_FLT:
            in.mBitsPerChannel = 32;
            in.mFormatFlags |= kLinearPCMFormatFlagIsFloat;
            break;
        case AudioBuffer::SAMPLE_FMT_DBL:
            in.mBitsPerChannel = 64;
            in.mFormatFlags |= kLinearPCMFormatFlagIsFloat;
            break;
        default:
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Unsupported audio format %d", inFormat);
            return;
    }
    in.mBytesPerFrame = in.mChannelsPerFrame * in.mBitsPerChannel / 8;
    in.mBytesPerPacket = in.mBytesPerFrame * in.mFramesPerPacket;

    out.mFormatID = kAudioFormatLinearPCM;
    out.mFormatFlags = kLinearPCMFormatFlagIsPacked;
    out.mChannelsPerFrame = inChannels;
    out.mSampleRate = inFreq;
    out.mFramesPerPacket = 1;
    switch (outFormat) {
        case AudioBuffer::SAMPLE_FMT_U8:
            out.mBitsPerChannel = 8;
            break;
        case AudioBuffer::SAMPLE_FMT_S16:
            out.mBitsPerChannel = 16;
            out.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
            break;
        case AudioBuffer::SAMPLE_FMT_S32:
            out.mBitsPerChannel = 32;
            out.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
            break;
        case AudioBuffer::SAMPLE_FMT_FLT:
            out.mBitsPerChannel = 32;
            out.mFormatFlags |= kLinearPCMFormatFlagIsFloat;
            break;
        case AudioBuffer::SAMPLE_FMT_DBL:
            out.mBitsPerChannel = 64;
            out.mFormatFlags |= kLinearPCMFormatFlagIsFloat;
            break;
        default:
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Unsupported audio format %d", outFormat);
            return;
    }
    out.mBytesPerFrame = out.mChannelsPerFrame * out.mBitsPerChannel / 8;
    out.mBytesPerPacket = out.mBytesPerFrame * out.mFramesPerPacket;

    OSStatus err = AudioConverterNew(&in, &out, &converter);
    
    /* Open the context */
    if (err < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "Error initializing AudioConverter: %d", err);
        return;
    }
}

void AudioConverterCoreAudio::dirty(void)
{
    AudioConverterDispose(converter);
    converter = nullptr;
}

static OSStatus converterCallback(AudioConverterRef inAudioConverter, int *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription **outDataPacketDescription, void *inUserData)
{
    AudioConverterCoreAudio *converter = static_cast<AudioConverterCoreAudio*>(inUserData);
    AudioBufferList *bufferList = converter->inBufferList;
    ioData->mBuffers[0].mNumberChannels = bufferList->mBuffers[0].mNumberChannels;
    ioData->mBuffers[0].mData = bufferList->mBuffers[0].mData;
    ioData->mBuffers[0].mDataByteSize = bufferList->mBuffers[0].mDataByteSize;

    *ioNumberDataPackets = ioData->mBuffers[0].mDataByteSize / converter->in.mBytesPerPacket;
    return noErr;
}

void AudioConverterCoreAudio::queueSamples(const uint8_t* inSamples, int inNbSamples)
{
    if (!isAvailable() || !isInited())
        return;

    /* Create an AudioBuffer object */
    AudioBuffer ab;
    ab.mData = inSamples;
    ab.mDataByteSize = in.mBytesPerFrame*inNbSamples;
    ab.mNumberChannels = in.mChannelsPerFrame;
    
    inBufferList.mNumberBuffers = 1
    inBufferList.mBuffers[0] = ab;

    AudioBufferList outBufferList;

    int ioOutputDataPacketSize = 0;
    OSStatus err = AudioConverterFillComplexBuffer(converter, converterCallback, 
    this, &ioOutputDataPacketSize, &outBufferList, NULL);
}

int AudioConverterCoreAudio::getSamples(uint8_t* outSamples, int outNbSamples)
{
    if (!isAvailable() || !isInited())
        return 0;
    
    /* Create an AudioBuffer object */
    AudioBuffer ab;
    ab.mData = nullptr;
    ab.mDataByteSize = 0;
    ab.mNumberChannels = in.mChannelsPerFrame;
    
    inBufferList.mNumberBuffers = 1
    inBufferList.mBuffers[0] = ab;

    AudioBuffer outAb;
    outAb.mData = outSamples;
    outAb.mDataByteSize = out.mBytesPerFrame*outNbSamples;
    outAb.mNumberChannels = out.mChannelsPerFrame;

    AudioBufferList outBufferList;
    outBufferList.mNumberBuffers = 1
    outBufferList.mBuffers[0] = outAb;

    int ioOutputDataPacketSize = outSamples;
    OSStatus err = AudioConverterFillComplexBuffer(converter, converterCallback, 
    this, &ioOutputDataPacketSize, &outBufferList, NULL);

    return ioOutputDataPacketSize;
}
