/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

// #include "hook.h"
#include "../logging.h"
#include "../ScreenCapture.h"
#include "../audio/AudioContext.h"
#include "../global.h" // shared_config
#include "../GlobalState.h"

#include <cstdint>
#include <unistd.h> // usleep

// #define ASSERT_RETURN_VOID(expr, msg) do {
//         if (!(expr)) {
//             error_msg = msg;
//             debuglog(LCF_DUMP | LCF_ERROR, msg);
//             error = -1;
//             return;
//         }
//     } while (false)
//
// #define ASSERT_RETURN(ret, msg) do {
//         if (ret < 0) {
//             error_msg = msg;
//             debuglog(LCF_DUMP | LCF_ERROR, msg);
//             return ret;
//         }
//     } while (false)

namespace libtas {

// static std::string error_msg;

AVEncoder::AVEncoder(SDL_Window* window) {
    // error = 0;

    // ASSERT_RETURN_VOID(shared_config.framerate_num > 0, "Not supporting non deterministic timer");

    std::string commandline = "ffmpeg -hide_banner -y -v 56 -f nut -i - ";
    commandline += ffmpeg_options;
    commandline += " \"";
    commandline += dumpfile;
    commandline += "\"";

    NATIVECALL(ffmpeg_pipe = popen(commandline.c_str(), "w"));

    // start_frame = sf;
    // accum_samples = 0;

    int width, height;
    ScreenCapture::getDimensions(width, height);

    const char* pixfmt = ScreenCapture::getPixelFormat();
    // ASSERT_RETURN_VOID(pixfmt != AV_PIX_FMT_NONE, "Unable to get pixel format");

    nutMuxer = new NutMuxer(width, height, shared_config.framerate_num, shared_config.framerate_den, pixfmt, audiocontext.outFrequency, audiocontext.outNbChannels, ffmpeg_pipe);

}

/*
 * Encode one frame and send it to the muxer
 * Returns 0 if no error was encountered, or a negative value if an error
 * was encountered.
 */
int AVEncoder::encodeOneFrame(bool draw) {

    /*** Video ***/
    debuglog(LCF_DUMP | LCF_FRAME, "Encode a video frame");

    uint8_t* pixels = nullptr;
    int size;

    /* Access to the screen pixels if the current frame is a draw frame
     * or if we never drew. If not, we will capture the last drawn frame.
     */
    if (draw || !pixels) {
        size = ScreenCapture::getPixels(&pixels);
    }

    nutMuxer->writeVideoFrame(pixels, size);

    /*** Audio ***/
    debuglog(LCF_DUMP, "Encode an audio frame");

    nutMuxer->writeAudioFrame(audiocontext.outSamples.data(), audiocontext.outBytes);

    return 0;
}

AVEncoder::~AVEncoder() {
    nutMuxer->finish();
    NATIVECALL(pclose(ffmpeg_pipe));
}

char AVEncoder::dumpfile[4096] = {0};
char AVEncoder::ffmpeg_options[4096] = {0};

std::unique_ptr<AVEncoder> avencoder;

}

// #endif
