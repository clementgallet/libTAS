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

#ifndef LIBTAS_AUDIOPLAYER_H_INCL
#define LIBTAS_AUDIOPLAYER_H_INCL

#ifdef LIBTAS_ENABLE_SOUNDPLAYBACK

#include "AudioContext.h"
#include <alsa/asoundlib.h>

/* Class in charge of sending the mixed samples to the audio device */
class AudioPlayer
{
    public:
        AudioPlayer();
        ~AudioPlayer();
		
		/* Store if the connection was inited */
		bool inited;

        /* Connection to the sound system */
        snd_pcm_t *phandle;

        /* Init the connection to the server.
         * Return if the connection was successful
         */
		bool init(snd_pcm_format_t format, int nbChannels, unsigned int frequency);

        /* Play the audio buffer stored in the audio context */
		bool play(AudioContext& ac);
};

extern AudioPlayer audioplayer;

#endif
#endif

