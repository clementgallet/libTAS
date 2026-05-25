/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include <cstdlib>
#include <unistd.h> // usleep
#include <sstream>
#include <iomanip>
#include <sys/wait.h> // waitpid

namespace libtas {


int AVEncoder::segment_number = 0;
char AVEncoder::dumpfile[4096] = {0};
char AVEncoder::ffmpeg_options[4096] = {0};

std::unique_ptr<AVEncoder> avencoder;


AVEncoder::AVEncoder() {
    std::ostringstream commandline;
    commandline << "ffmpeg -xerror -hide_banner -y -f nut -i - ";
    commandline << ffmpeg_options;
    commandline << " \"";
    char* dumpfile_ext = strrchr(dumpfile, '.');
    if (dumpfile_ext == NULL) dumpfile_ext = dumpfile + strnlen(dumpfile, 4096);
    
    commandline.write(dumpfile, static_cast<int>(dumpfile_ext - dumpfile));
    /* Add segment number to filename if not the first */
    if (segment_number > 0) {
        commandline << "_" << segment_number;
    }
    commandline << dumpfile_ext;
    commandline << "\"";

    {
        GlobalNative gn;
        
        int pipefd[2] = {-1, -1};
        if (pipe(pipefd) < 0) {
            LOG(LL_ERROR, LCF_DUMP, "Could not create a pipe to ffmpeg");
            return;
        }

        ffmpeg_pid = fork();
        if (ffmpeg_pid < 0) {
            LOG(LL_ERROR, LCF_DUMP, "Could not fork the ffmpeg process");
            close(pipefd[0]);
            close(pipefd[1]);
            return;
        }

        if (ffmpeg_pid == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            execlp("sh", "sh", "-c", commandline.str().c_str(), nullptr);
            _exit(127);
        }

        close(pipefd[0]);
        ffmpeg_pipe = fdopen(pipefd[1], "w");
        if (!ffmpeg_pipe) {
            LOG(LL_ERROR, LCF_DUMP, "Could not open the ffmpeg pipe stream");
            close(pipefd[1]);
            waitpid(ffmpeg_pid, nullptr, 0);
            ffmpeg_pid = -1;
            return;
        }
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
    if (nutMuxer) {
        nutMuxer->finish();
        delete nutMuxer;
        nutMuxer = nullptr;
    }

    int width, height;
    ScreenCapture::getDimensions(width, height);

    const char* pixfmt = ScreenCapture::getPixelFormat();

    /* Initialize the muxer with either framerate or video framerate */
    AudioContext& audiocontext = AudioContext::get();

    if (!audiocontext.isInited()) {
        LOG(LL_WARN, LCF_SOUND, "Some audio parameters were set to auto. However, we need to start encoding now, so we cannot wait for the game to initialize the audio parameters. Default audio parameters will be chosen.");
        audiocontext.initDefaults();
    }

    if (Global::shared_config.video_framerate)
        nutMuxer = new NutMuxer(width, height, Global::shared_config.video_framerate, 1, pixfmt, audiocontext.frequency, audiocontext.bytes_per_sample, audiocontext.channels, ffmpeg_pipe);
    else
        nutMuxer = new NutMuxer(width, height, Global::shared_config.initial_framerate_num, Global::shared_config.initial_framerate_den, pixfmt, audiocontext.frequency, audiocontext.bytes_per_sample, audiocontext.channels, ffmpeg_pipe);
}

void AVEncoder::encodeOneFrame(bool draw, TimeHolder frametime) {
    if (!ffmpeg_pipe)
        return;
    
    /* Check if ffmpeg did exit for some reason */
    int ret_pid = waitpid(ffmpeg_pid, nullptr, WNOHANG);
    if (ret_pid == ffmpeg_pid) {
        LOG(LL_WARN, LCF_DUMP, "ffmpeg process exited, encoding stopped");
        NATIVECALL(fclose(ffmpeg_pipe));
        ffmpeg_pipe = nullptr;
        return;
    }

    /* If the muxer is not initialized, try to initialize it. Otherwise, store
     * that we skipped one frame and we need to encode it later.
     */
    AudioContext& audiocontext = AudioContext::get();
    if (!nutMuxer) {
        if (ScreenCapture::isInited()) {
            initMuxer();

            /* Encode audio samples that we skipped */
            if (startup_audio_frames > 0) {
                std::vector<uint8_t> empty_samples(audiocontext.samples_byte_size, AudioBuffer::formatToSilenceByte(audiocontext.format));
                for (int i=0; i<startup_audio_frames; i++) {
                    nutMuxer->writeAudioFrame(empty_samples.data(), audiocontext.samples_byte_size);
                }
            }

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

            /* Store audio samples that we skipped, or keep track of skipped audio frames */
            if (audiocontext.isInited()) {
                startup_audio_bytes.insert(startup_audio_bytes.end(), audiocontext.samples_data.data(), audiocontext.samples_data.data() + audiocontext.samples_byte_size);
            }
            else {
                startup_audio_frames++;
            }
            return;
        }
    }

    /*** Audio ***/
    LOG(LL_DEBUG, LCF_DUMP, "Encode an audio frame");

    nutMuxer->writeAudioFrame(audiocontext.samples_data.data(), audiocontext.samples_byte_size);

    /*** Video ***/

    /* Number of frames to encode */
    int frames = 1;

    if (Global::shared_config.video_framerate) {
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
        delete nutMuxer;
        nutMuxer = nullptr;
    }

    if (ffmpeg_pipe) {
        int ret;
        NATIVECALL(ret = fclose(ffmpeg_pipe));
        if (ret < 0) {
            LOG(LL_ERROR, LCF_DUMP, "Could not close the pipe to ffmpeg");
        }
        ffmpeg_pipe = nullptr;
    }

    if (ffmpeg_pid > 0) {
        waitpid(ffmpeg_pid, nullptr, 0);
        ffmpeg_pid = -1;
    }
}

}

// #endif
