/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "AudioPlayerCoreAudio.h"
#include "AudioContext.h"

#include "logging.h"
#include "global.h" // Global::shared_config
#include "GlobalState.h"

#include <algorithm>

namespace libtas {

AudioQueueRef AudioPlayerCoreAudio::audioQueue;
std::vector<AudioQueueBufferRef> AudioPlayerCoreAudio::audioQueueBuffers;
AudioPlayerCoreAudio::APStatus AudioPlayerCoreAudio::status = STATUS_UNINIT;
AudioPlayerCoreAudio::cyclic_buffer AudioPlayerCoreAudio::cyclicBuffer;

static void audioCallback(void *inUserData, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
    AudioPlayerCoreAudio::cyclic_buffer* cyclicBuffer = static_cast<AudioPlayerCoreAudio::cyclic_buffer*>(inUserData);
    
    UInt32 remaining = buffer->mAudioDataBytesCapacity;
    uint8_t* ptr = static_cast<uint8_t*>(buffer->mAudioData);
    
    /* Read from cyclic buffer */
    int bytesToRead = std::min(buffer->mAudioDataBytesCapacity, static_cast<uint32_t>(cyclicBuffer->size));

    /* Read at buffer end */
    size_t sizeEnd = std::min(bytesToRead, cyclicBuffer->cap - cyclicBuffer->beg);
    memcpy(ptr, cyclicBuffer->data.data() + cyclicBuffer->beg, sizeEnd);

    /* Read at buffer beginning if needed */
    int sizeBeg = bytesToRead - sizeEnd;
    if (sizeBeg > 0) {
        memcpy(ptr + sizeEnd, cyclicBuffer->data.data(), sizeBeg);
        cyclicBuffer->beg = sizeBeg;
    }
    else
        cyclicBuffer->beg += bytesToRead;        
    cyclicBuffer->size -= bytesToRead;
    
    /* Fill the rest with silence */
    if (bytesToRead < buffer->mAudioDataBytesCapacity) {
        memset(ptr + bytesToRead, cyclicBuffer->silence, buffer->mAudioDataBytesCapacity-bytesToRead);
    }
    
    AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
    buffer->mAudioDataByteSize = buffer->mAudioDataBytesCapacity;
}

bool AudioPlayerCoreAudio::init(AudioContext& ac)
{
    debuglogstdio(LCF_SOUND, "Init audio player");

    GlobalNative gn;
    OSStatus res;

    /* Build stream struct */
    AudioStreamBasicDescription strdesc = {0};
    strdesc.mFormatID = kAudioFormatLinearPCM;
    strdesc.mFormatFlags = kLinearPCMFormatFlagIsPacked;
    strdesc.mChannelsPerFrame = ac.outNbChannels;
    strdesc.mSampleRate = ac.outFrequency;
    strdesc.mFramesPerPacket = 1;

    switch (ac.outBitDepth) {
        case 8:
            strdesc.mBitsPerChannel = 8;
            break;
        case 16:
            strdesc.mBitsPerChannel = 16;
            strdesc.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
            break;
        default:
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Unsupported audio format %d", ac.outBitDepth);
            return false;
    }
    
    strdesc.mBytesPerFrame = strdesc.mChannelsPerFrame * strdesc.mBitsPerChannel / 8;
    strdesc.mBytesPerPacket = strdesc.mBytesPerFrame * strdesc.mFramesPerPacket;

    /* Setup cyclic buffer */
    int buffer_size = (2*ac.outFrequency*Global::shared_config.initial_framerate_den/Global::shared_config.initial_framerate_num);

    cyclicBuffer.data.resize(2*buffer_size*strdesc.mBytesPerFrame);
    cyclicBuffer.beg = 0;
    cyclicBuffer.end = 0;
    cyclicBuffer.size = 0;
    cyclicBuffer.cap = 2*buffer_size*strdesc.mBytesPerFrame;
    cyclicBuffer.silence = (strdesc.mBitsPerChannel==8)?0x80:0;

    /* Init new queue */
    if ((res = AudioQueueNewOutput(&strdesc, audioCallback, &cyclicBuffer, nullptr, nullptr, 0, &audioQueue)) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  Cannot create audio queue: err %d", res);
        return false;
    }

    /* Set the channel layout for the audio queue */
    AudioChannelLayout layout = {0};
    switch (strdesc.mChannelsPerFrame) {
        case 1:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Mono;
            break;
        case 2:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
            break;
        default:
            debuglogstdio(LCF_SOUND | LCF_ERROR, "  Unknown channel layout");
            return false;
    }
    
    if ((res = AudioQueueSetProperty(audioQueue, kAudioQueueProperty_ChannelLayout, &layout, sizeof(layout))) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  AudioQueueSetProperty(kAudioQueueProperty_ChannelLayout) failed: err %d", res);
        return false;
    }

    int numAudioBuffers = 2;
    audioQueueBuffers.resize(numAudioBuffers);

    /* Create audio buffers */
    for (int i = 0; i < numAudioBuffers; i++) {
        if ((res = AudioQueueAllocateBuffer(audioQueue, buffer_size, &audioQueueBuffers[i])) < 0) {
            debuglogstdio(LCF_SOUND | LCF_ERROR, "  AudioQueueAllocateBuffer failed: err %d", res);
            return false;
        }
        
        if (strdesc.mBitsPerChannel == 8)
            memset(audioQueueBuffers[i]->mAudioData, 0x80, audioQueueBuffers[i]->mAudioDataBytesCapacity);
        if (strdesc.mBitsPerChannel == 16)
            memset(audioQueueBuffers[i]->mAudioData, 0, audioQueueBuffers[i]->mAudioDataBytesCapacity);

        audioQueueBuffers[i]->mAudioDataByteSize = audioQueueBuffers[i]->mAudioDataBytesCapacity;
        if ((res = AudioQueueEnqueueBuffer(audioQueue, audioQueueBuffers[i], 0, NULL)) < 0) {
            debuglogstdio(LCF_SOUND | LCF_ERROR, "  AudioQueueEnqueueBuffer failed: err %d", res);
            return false;
        }
    }
    
    if ((res = AudioQueueStart(audioQueue, NULL))) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  AudioQueueEnqueueBuffer failed: err %d", res);
        return false;
    }

    return true;
}

bool AudioPlayerCoreAudio::play(AudioContext& ac)
{
    if (status == STATUS_UNINIT) {
        if (!init(ac)) {
            status = STATUS_ERROR;
            return false;
        }
        status = STATUS_OK;
    }

    if (status == STATUS_ERROR)
        return false;

    if (Global::shared_config.fastforward)
        return true;

    debuglogstdio(LCF_SOUND, "Play an audio frame");
    
    /* Write into circular buffer */
    int bytestoWrite = std::min(ac.outBytes, cyclicBuffer.cap - cyclicBuffer.size);
    if ((cyclicBuffer.cap - cyclicBuffer.size) < ac.outBytes)
        debuglogstdio(LCF_SOUND | LCF_WARNING, "Not enough space in circular buffer to write");

    /* Write until buffer end */
    size_t sizeEnd = std::min(bytestoWrite, cyclicBuffer.cap - cyclicBuffer.end);
    memcpy(cyclicBuffer.data.data() + cyclicBuffer.end, ac.outSamples.data(), sizeEnd);
    
    /* Write at buffer beginning */
    size_t sizeBeg = bytestoWrite - sizeEnd;
    if (sizeBeg > 0) {
        memcpy(cyclicBuffer.data.data(), ac.outSamples.data() + sizeEnd, sizeBeg);
        cyclicBuffer.end = sizeBeg;
    }
    else
        cyclicBuffer.end += bytestoWrite;
    cyclicBuffer.size += bytestoWrite;

    return true;
}

void AudioPlayerCoreAudio::close()
{
    if (status == STATUS_OK) {
        if (audioQueue)
            AudioQueueDispose(audioQueue, 1);
        status = STATUS_UNINIT;
    }
}

}
