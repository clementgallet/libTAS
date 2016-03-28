/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef AUDIOPLAYER_H_INCL
#define AUDIOPLAYER_H_INCL

#include "AudioContext.h"
#include <pulse/simple.h>

class AudioPlayer
{
    public:
        AudioPlayer();
		
		/* Store if the connection was inited */
		bool inited;

        /* Connection to the pulseaudio server */
        pa_simple *pa_s;

        /* Init the connection to the server.
         * Return if the connection was successful
         */
		bool init(pa_sample_format_t format, int nbChannels, int frequency);

        /* Play the audio buffer stored in the audio context */
		bool play(AudioContext& ac);
};

extern AudioPlayer audioplayer;

#endif
