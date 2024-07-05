/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "AVEncoder.h"
#include "NutMuxer.h"

#include "logging.h"
#include "screencapture/ScreenCapture.h"
#include "audio/AudioContext.h"
#include "global.h" // Global::shared_config
#include "GlobalState.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

#include <cstdint>
#include <unistd.h> // usleep
#include <sstream>
#include <iomanip>

namespace libtas {


int AVEncoder::segment_number = 0;
char AVEncoder::dumpfile[4096] = {0};
char AVEncoder::ffmpeg_options[4096] = {0};

std::unique_ptr<AVEncoder> avencoder;


AVEncoder::AVEncoder() {
    std::ostringstream commandline;
    commandline << "ffmpeg -hide_banner -y -f nut -i - ";
    commandline << ffmpeg_options;
    commandline << " \"";
    commandline.write(dumpfile, static_cast<int>(strrchr(dumpfile, '.') - dumpfile));
    /* Add segment number to filename if not the first */
    if (segment_number > 0) {
        commandline << "_" << segment_number;
    }
    commandline << strrchr(dumpfile, '.');
    commandline << "\"";

    NATIVECALL(ffmpeg_pipe = popen(commandline.str().c_str(), "w"));

    if (! ffmpeg_pipe) {
        LOG(LL_ERROR, LCF_DUMP, "Could not create a pipe to ffmpeg");
        return;
    }

    if (ScreenCapture::isInited()) {
        initMuxer();
    }

    segment_number++;
    /* Socket is already locked in frame.cpp */
    sendMessage(MSGB_ENCODING_SEGMENT);
    sendData(&segment_number, sizeof(int));
}

void AVEncoder::initMuxer() {
    int width, height;
    ScreenCapture::getDimensions(width, height);

    const char* pixfmt = ScreenCapture::getPixelFormat();

    /* Initialize the muxer with either framerate or video framerate */
    AudioContext& audiocontext = AudioContext::get();
    if (Global::shared_config.variable_framerate)
        nutMuxer = new NutMuxer(width, height, Global::shared_config.video_framerate, 1, pixfmt, audiocontext.outFrequency, audiocontext.outAlignSize, audiocontext.outNbChannels, ffmpeg_pipe);
    else
        nutMuxer = new NutMuxer(width, height, Global::shared_config.initial_framerate_num, Global::shared_config.initial_framerate_den, pixfmt, audiocontext.outFrequency, audiocontext.outAlignSize, audiocontext.outNbChannels, ffmpeg_pipe);
}

void AVEncoder::encodeOneFrame(bool draw, TimeHolder frametime) {

    /* If the muxer is not initialized, try to initialize it. Otherwise, store
     * that we skipped one frame and we need to encode it later.
     */
    AudioContext& audiocontext = AudioContext::get();
    if (!nutMuxer) {
        if (ScreenCapture::isInited()) {
            initMuxer();

            /* Encode audio samples that we skipped */
            nutMuxer->writeAudioFrame(startup_audio_bytes.data(), startup_audio_bytes.size());

            /* Encode startup frames that we skipped */

            /* Just getting the size of an image */
            int size = ScreenCapture::getSize();
            startup_audio_bytes.resize(size, 0); // reusing the audio samples vector
            for (int i=0; i<startup_video_frames; i++) {
                nutMuxer->writeVideoFrame(startup_audio_bytes.data(), size);
            }
        }
        else {
            startup_video_frames++;
            /* Store audio samples that we skipped */
            startup_audio_bytes.insert(startup_audio_bytes.end(), audiocontext.outSamples.data(), audiocontext.outSamples.data() + audiocontext.outBytes);
            return;
        }
    }

    /*** Audio ***/
    LOG(LL_DEBUG, LCF_DUMP, "Encode an audio frame");

    nutMuxer->writeAudioFrame(audiocontext.outSamples.data(), audiocontext.outBytes);

    /*** Video ***/

    /* Number of frames to encode */
    int frames = 1;

    if (Global::shared_config.variable_framerate) {
        /* We must send as many video frames to match the video framerate parameter */
        frame_remainder += (frametime.tv_sec + ((double)frametime.tv_nsec) / 1000000000.0) * Global::shared_config.video_framerate;

        frames = (int)(frame_remainder + 0.5);
        frame_remainder -= frames;
    }

    /* Access to the screen pixels, or last screen pixels if not a draw frame */
    int size = ScreenCapture::getPixelsFromSurface(&pixels, draw);

    for (int f=0; f<frames; f++) {
        LOG(LL_DEBUG, LCF_DUMP, "Encode a video frame");
        nutMuxer->writeVideoFrame(pixels, size);
    }
}

AVEncoder::~AVEncoder() {
    if (nutMuxer) {
        nutMuxer->finish();
    }

    if (ffmpeg_pipe) {
        int ret;
        NATIVECALL(ret = pclose(ffmpeg_pipe));
        if (ret < 0) {
            LOG(LL_ERROR, LCF_DUMP, "Could not close the pipe to ffmpeg");
        }
    }
}

}

// #endif
