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

#include "cubeb.h"

#include "logging.h"
#include "hook.h"
#include "global.h"
#include "audio/AudioContext.h"
#include "audio/AudioSource.h"
#include "audio/AudioBuffer.h"

#include <stdint.h>

namespace libtas {

int cubeb_init(cubeb ** context, char const * context_name, char const * backend_name)
{
    LOGTRACE(LCF_SOUND);
    return CUBEB_OK;
}

static const char* dummy_backend = "libtas";

char const * cubeb_get_backend_id(cubeb * context)
{
    LOGTRACE(LCF_SOUND);
    return dummy_backend;
}

int cubeb_get_max_channel_count(cubeb * context, uint32_t * max_channels)
{
    LOGTRACE(LCF_SOUND);
    if (max_channels)
        *max_channels = 2;
    return CUBEB_OK;
}

int cubeb_get_min_latency(cubeb * context, cubeb_stream_params * params, uint32_t * latency_frames)
{
    LOGTRACE(LCF_SOUND);
    if (latency_frames)
        *latency_frames = Global::shared_config.audio_frequency; // Not sure what to put here.
    return CUBEB_OK;    
}

int cubeb_get_preferred_sample_rate(cubeb * context, uint32_t * rate)
{
    LOGTRACE(LCF_SOUND);
    if (rate)
        *rate = Global::shared_config.audio_frequency;
    return CUBEB_OK;    
}

void cubeb_destroy(cubeb * context)
{
    LOGTRACE(LCF_SOUND);
}

int cubeb_stream_init(cubeb * context,
                      cubeb_stream ** stream,
                      char const * stream_name,
                      cubeb_devid input_device,
                      cubeb_stream_params * input_stream_params,
                      cubeb_devid output_device,
                      cubeb_stream_params * output_stream_params,
                      uint32_t latency_frames,
                      cubeb_data_callback data_callback,
                      cubeb_state_callback state_callback,
                      void * user_ptr)
{
    LOGTRACE(LCF_SOUND);

    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* Create the buffer and fill params */
    int bufferId = audiocontext.createBuffer();
    auto buffer = audiocontext.getBuffer(bufferId);

    buffer->frequency = output_stream_params->rate;
    LOG(LL_DEBUG, LCF_SOUND, "   Frequency %d Hz", buffer->frequency);

    switch(output_stream_params->format) {
        case CUBEB_SAMPLE_S16LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_S16;
            break;
        case CUBEB_SAMPLE_FLOAT32LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_FLT;
            break;
        default:
            LOG(LL_DEBUG, LCF_SOUND, "   Unsupported audio format");
            return -1;
    }

    buffer->nbChannels = output_stream_params->channels;
    LOG(LL_DEBUG, LCF_SOUND, "   Channels %d", buffer->nbChannels);

    buffer->update();
    LOG(LL_DEBUG, LCF_SOUND, "   Format %d bits", buffer->bitDepth);

    buffer->size = latency_frames * buffer->alignSize;
    buffer->update(); // Yes, a second time, to fill sampleSize based on size.
    buffer->samples.resize(buffer->size);

    /* Push buffers in a source */
    int sourceId = audiocontext.createSource();
    auto source = audiocontext.getSource(sourceId);
    *stream = reinterpret_cast<cubeb_stream*>(sourceId);

    source->buffer_queue.push_back(buffer);

    source->source = AudioSource::SOURCE_CALLBACK;
    source->callback = ([data_callback, stream, user_ptr](AudioBuffer& ab) {
        int samples = data_callback(*stream, user_ptr, nullptr, ab.samples.data(), ab.sampleSize);
        if (samples != ab.size)
            LOG(LL_WARN, LCF_SOUND, "   Buffer not filled completely (%d / %d)", samples, ab.sampleSize);
    });
    
    /* We simulate an empty buffer by setting the position at the end */
    source->position = buffer->sampleSize;
    return CUBEB_OK;
}

void cubeb_stream_destroy(cubeb_stream * stream)
{
    LOGTRACE(LCF_SOUND);
    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    int sourceId = reinterpret_cast<intptr_t>(stream);
    audiocontext.deleteSource(sourceId);
}

int cubeb_stream_start(cubeb_stream * stream)
{
    LOGTRACE(LCF_SOUND);
    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    int sourceId = reinterpret_cast<intptr_t>(stream);
    auto source = audiocontext.getSource(sourceId);
    
    if (!source)
        return CUBEB_ERROR;
        
    source->state = AudioSource::SOURCE_PLAYING;
    return CUBEB_OK;
}

int cubeb_stream_stop(cubeb_stream * stream)
{
    LOGTRACE(LCF_SOUND);
    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    int sourceId = reinterpret_cast<intptr_t>(stream);
    auto source = audiocontext.getSource(sourceId);
    
    if (!source)
        return CUBEB_ERROR;
        
    source->state = AudioSource::SOURCE_PAUSED;
    return CUBEB_OK;
}

int cubeb_stream_reset_default_device(cubeb_stream * stream)
{
    LOGTRACE(LCF_SOUND);
    return CUBEB_OK;
}

int cubeb_stream_get_position(cubeb_stream * stream, uint64_t * position)
{
    LOGTRACE(LCF_SOUND);
    if (position) {
        AudioContext& audiocontext = AudioContext::get();
        std::lock_guard<std::mutex> lock(audiocontext.mutex);
        int sourceId = reinterpret_cast<intptr_t>(stream);
        auto source = audiocontext.getSource(sourceId);
        *position = source->getPosition(); // this is probabaly not the expected value
                                         // because the callback is reusing the same buffer
        return CUBEB_OK;
    }
    return CUBEB_ERROR;
}

int cubeb_stream_get_latency(cubeb_stream * stream, uint32_t * latency)
{
    LOGTRACE(LCF_SOUND);
    if (latency) {
        AudioContext& audiocontext = AudioContext::get();
        std::lock_guard<std::mutex> lock(audiocontext.mutex);
        int sourceId = reinterpret_cast<intptr_t>(stream);
        auto source = audiocontext.getSource(sourceId);
        *latency = source->queueSize() - source->getPosition();
        return CUBEB_OK;
    }
    return CUBEB_ERROR;
}

int cubeb_stream_get_input_latency(cubeb_stream * stream, uint32_t * latency)
{
    LOGTRACE(LCF_SOUND);
    return CUBEB_ERROR;
}

int cubeb_stream_set_volume(cubeb_stream * stream, float volume)
{
    LOGTRACE(LCF_SOUND);
    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    int sourceId = reinterpret_cast<intptr_t>(stream);
    auto source = audiocontext.getSource(sourceId);
    if (!source)
        return CUBEB_ERROR_INVALID_PARAMETER;
    source->volume = volume;
    return CUBEB_OK;
}

int cubeb_stream_get_current_device(cubeb_stream * stm, cubeb_device ** const device)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return CUBEB_ERROR_NOT_SUPPORTED;
}

int cubeb_stream_device_destroy(cubeb_stream * stream, cubeb_device * devices)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return CUBEB_ERROR_NOT_SUPPORTED;
}

int cubeb_stream_register_device_changed_callback(cubeb_stream * stream, cubeb_device_changed_callback device_changed_callback)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return CUBEB_ERROR_NOT_SUPPORTED;
}

void * cubeb_stream_user_ptr(cubeb_stream * stream)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return nullptr;
}

int cubeb_enumerate_devices(cubeb * context, cubeb_device_type devtype, cubeb_device_collection * collection)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return CUBEB_ERROR_NOT_SUPPORTED;
}

int cubeb_device_collection_destroy(cubeb * context, cubeb_device_collection * collection)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return CUBEB_ERROR_NOT_SUPPORTED;
}

int cubeb_register_device_collection_changed(cubeb * context, cubeb_device_type devtype, cubeb_device_collection_changed_callback callback, void * user_ptr)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return CUBEB_ERROR_NOT_SUPPORTED;
}

int cubeb_set_log_callback(cubeb_log_level log_level, cubeb_log_callback log_callback)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return CUBEB_ERROR_NOT_SUPPORTED;
}

}
