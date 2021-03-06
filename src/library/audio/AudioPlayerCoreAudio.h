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

#ifndef LIBTAS_AUDIOPLAYERCOREAUDIO_H_INCL
#define LIBTAS_AUDIOPLAYERCOREAUDIO_H_INCL

#include "AudioContext.h"
#include <AudioToolbox/AudioToolbox.h>
#include <stdint.h>

namespace libtas {
/* Class in charge of sending the mixed samples to the audio device */
class AudioPlayerCoreAudio
{
private:
    /* Status */
    enum APStatus {
        STATUS_ERROR = -1,
        STATUS_UNINIT = 0,
        STATUS_OK = 1,
    };

    static APStatus status;

    struct cyclic_buffer {
        std::vector<uint8_t> data;
        int beg;
        int end;
        int size;
        int cap;
    };

    static cyclic_buffer buffer;

public:
    /* Init the connection to the server.
     * Return if the connection was successful
     */
	static bool init(AudioContext& ac);

    /* Play the audio buffer stored in the audio context */
	static bool play(AudioContext& ac);

    /* Close the connection to the server */
    static void close();
};
}

#endif
