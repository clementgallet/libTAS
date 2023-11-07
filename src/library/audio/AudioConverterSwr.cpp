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

#include "AudioConverterSwr.h"

#include "logging.h"
#include "hook.h"
#include "GlobalState.h"

extern "C" {
#include <libavutil/samplefmt.h>
}
#include <stdlib.h>
#include <stdint.h>

namespace libtas {

/* Link dynamically to swresample functions, because there are different
 * library versions depending on your distro.
 */
DEFINE_ORIG_POINTER(swr_alloc)
DEFINE_ORIG_POINTER(swr_free)
DEFINE_ORIG_POINTER(swr_is_initialized)
DEFINE_ORIG_POINTER(swr_close)
DEFINE_ORIG_POINTER(swr_init)
#if LIBSWRESAMPLE_VERSION_INT >= AV_VERSION_INT(4,7,100)
DEFINE_ORIG_POINTER(swr_alloc_set_opts2)
#else
DEFINE_ORIG_POINTER(swr_alloc_set_opts)
#endif
DEFINE_ORIG_POINTER(swr_convert)

AudioConverterSwr::AudioConverterSwr(void)
{
    /* Some systems don't create the unversionned symlinks when the libraries
     * are installed, so we add a link with the major version. */

    /* Disabling logging because we expect some of these to fail */
    {
        GlobalNoLog gnl;
        LINK_NAMESPACE(swr_alloc, "swresample");
        LINK_NAMESPACE_FULLNAME(swr_alloc, "libswresample.so.4");
        LINK_NAMESPACE_FULLNAME(swr_alloc, "libswresample.so.3");
        LINK_NAMESPACE_FULLNAME(swr_alloc, "libswresample.so.2");
    }
    /* Still test if it succeeded. */
    if (!orig::swr_alloc) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "Could not link to swr_alloc, disable audio mixing");
        swr = nullptr;
    }
    else {
        /* We link to swr_free here, because linking during destructor can softlock */
        LINK_NAMESPACE(swr_free, "swresample");

        swr = orig::swr_alloc();        
    }
}

AudioConverterSwr::~AudioConverterSwr(void)
{
    if (orig::swr_free && swr)
        orig::swr_free(&swr);
}

bool AudioConverterSwr::isAvailable()
{
    return swr;
}

bool AudioConverterSwr::isInited()
{
    if (!swr)
        return false;

    LINK_NAMESPACE(swr_is_initialized, "swresample");
    return orig::swr_is_initialized(swr);
}

void AudioConverterSwr::init(AudioBuffer::SampleFormat inFormat, int inChannels, int inFreq, AudioBuffer::SampleFormat outFormat, int outChannels, int outFreq)
{
    if (!swr)
        return;
        
    LINK_NAMESPACE(swr_init, "swresample");
    LINK_NAMESPACE(swr_convert, "swresample");
#if LIBSWRESAMPLE_VERSION_INT >= AV_VERSION_INT(4,7,100)
    LINK_NAMESPACE(swr_alloc_set_opts2, "swresample");
#else
    LINK_NAMESPACE(swr_alloc_set_opts, "swresample");
#endif

    AVSampleFormat inAVFormat = AV_SAMPLE_FMT_U8;
    switch (inFormat) {
        case AudioBuffer::SAMPLE_FMT_U8:
            inAVFormat = AV_SAMPLE_FMT_U8;
            break;
        case AudioBuffer::SAMPLE_FMT_S16:
        case AudioBuffer::SAMPLE_FMT_MSADPCM:
            inAVFormat = AV_SAMPLE_FMT_S16;
            break;
        case AudioBuffer::SAMPLE_FMT_S32:
            inAVFormat = AV_SAMPLE_FMT_S32;
            break;
        case AudioBuffer::SAMPLE_FMT_FLT:
            inAVFormat = AV_SAMPLE_FMT_FLT;
            break;
        case AudioBuffer::SAMPLE_FMT_DBL:
            inAVFormat = AV_SAMPLE_FMT_DBL;
            break;
        default:
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Unknown sample format");
            break;
    }

    AVSampleFormat outAVFormat = AV_SAMPLE_FMT_U8;
    switch (outFormat) {
        case AudioBuffer::SAMPLE_FMT_U8:
            outAVFormat = AV_SAMPLE_FMT_U8;
            break;
        case AudioBuffer::SAMPLE_FMT_S16:
        case AudioBuffer::SAMPLE_FMT_MSADPCM:
            outAVFormat = AV_SAMPLE_FMT_S16;
            break;
        case AudioBuffer::SAMPLE_FMT_S32:
            outAVFormat = AV_SAMPLE_FMT_S32;
            break;
        case AudioBuffer::SAMPLE_FMT_FLT:
            outAVFormat = AV_SAMPLE_FMT_FLT;
            break;
        case AudioBuffer::SAMPLE_FMT_DBL:
            outAVFormat = AV_SAMPLE_FMT_DBL;
            break;
        default:
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Unknown sample format");
            break;
    }

#if LIBSWRESAMPLE_VERSION_INT >= AV_VERSION_INT(4,7,100)
    /* Get the channel layout */
    AVChannelLayout in_ch_layout;
    AVChannelLayout out_ch_layout;

    if (inChannels == 1) {
        in_ch_layout = AV_CHANNEL_LAYOUT_MONO;
    }
    if (inChannels == 2) {
        in_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    }
    if (outChannels == 1) {
        out_ch_layout = AV_CHANNEL_LAYOUT_MONO;
    }
    if (outChannels == 2) {
        out_ch_layout = AV_CHANNEL_LAYOUT_STEREO;
    }

    MYASSERT(0 == orig::swr_alloc_set_opts2(&swr, &out_ch_layout, outAVFormat, outFreq, &in_ch_layout, inAVFormat, inFreq, 0, nullptr));
#else

    /* Get the channel layout */
    int64_t in_ch_layout = 0;
    int64_t out_ch_layout = 0;

    if (inChannels == 1) {
        in_ch_layout = AV_CH_LAYOUT_MONO;
    }
    if (inChannels == 2) {
        in_ch_layout = AV_CH_LAYOUT_STEREO;
    }
    if (outChannels == 1) {
        out_ch_layout = AV_CH_LAYOUT_MONO;
    }
    if (outChannels == 2) {
        out_ch_layout = AV_CH_LAYOUT_STEREO;
    }

    MYASSERT(nullptr != orig::swr_alloc_set_opts(swr, out_ch_layout, outAVFormat, outFreq, in_ch_layout, inAVFormat, inFreq, 0, nullptr));
#endif

    /* Open the context */
    if (orig::swr_init(swr) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "Error initializing swr context");
        return;
    }
}

void AudioConverterSwr::dirty(void)
{
    if (swr) {
        LINK_NAMESPACE(swr_is_initialized, "swresample");
        LINK_NAMESPACE(swr_close, "swresample");
        if (orig::swr_is_initialized(swr))
            orig::swr_close(swr);
    }
}

void AudioConverterSwr::queueSamples(const uint8_t* inSamples, int inNbSamples)
{
    if (!isAvailable() || !isInited())
        return;
    
    orig::swr_convert(swr, nullptr, 0, &inSamples, inNbSamples);
}

int AudioConverterSwr::getSamples(uint8_t* outSamples, int outNbSamples)
{
    if (!isAvailable() || !isInited())
        return 0;
    
    return orig::swr_convert(swr, &outSamples, outNbSamples, nullptr, 0);
}

}
