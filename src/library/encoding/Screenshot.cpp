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

#include "Screenshot.h"
#include "NutMuxer.h"

#include "logging.h"
#include "screencapture/ScreenCapture.h"
#include "global.h" // Global::shared_config
#include "GlobalState.h"

#include <cstdint>
#include <sstream>

namespace libtas {

int Screenshot::save(const std::string& screenshotfile, bool draw) {
    
    if (!ScreenCapture::isInited()) {
        LOG(LL_ERROR, LCF_DUMP, "Screen was not inited");
        return ESCREENSHOT_NOSCREEN;
    }

    std::ostringstream commandline;
    commandline << "ffmpeg -loglevel warning -hide_banner -y -guess_layout_max 0 -f nut -i - -frames:v 1 -update 1 \"";
    commandline << screenshotfile;
    commandline << "\"";

    FILE *ffmpeg_pipe = nullptr;
    NATIVECALL(ffmpeg_pipe = popen(commandline.str().c_str(), "w"));

    if (! ffmpeg_pipe) {
        LOG(LL_ERROR, LCF_DUMP, "Could not create a pipe to ffmpeg");
        return ESCREENSHOT_NOPIPE;
    }
    
    int width, height;
    ScreenCapture::getDimensions(width, height);

    const char* pixfmt = ScreenCapture::getPixelFormat();

    /* Initialize the muxer. Audio parameters don't matter here for screenshot */
    NutMuxer* nutMuxer = new NutMuxer(width, height, Global::shared_config.initial_framerate_num, Global::shared_config.initial_framerate_den, pixfmt, 44100, 1, 1, ffmpeg_pipe);

    /* Access to the screen pixels, or last screen pixels if not a draw frame */
    uint8_t* pixels = nullptr;
    int size = ScreenCapture::getPixelsFromSurface(&pixels, draw);

    LOG(LL_DEBUG, LCF_DUMP, "Perform the screenshot");
    nutMuxer->writeVideoFrame(pixels, size);
    
    if (ffmpeg_pipe) {
        int ret;
        NATIVECALL(ret = pclose(ffmpeg_pipe));
        if (ret < 0) {
            LOG(LL_ERROR, LCF_DUMP, "Could not close the pipe to ffmpeg");
            return ESCREENSHOT_NOPIPE;
        }
    }
    
    return ESCREENSHOT_OK;
}

}

// #endif
