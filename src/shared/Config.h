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

#ifndef LIBTAS_CONFIG_H_INCLUDED
#define LIBTAS_CONFIG_H_INCLUDED

/* Structure holding program configuration that is saved in a file */

class Config {
    public:
        /* Display frame count in the HUD */
        bool hud_framecount;

        /* Display inputs in the HUD */
        bool hud_inputs;

        /* Prevent the game to write into savefiles */
        bool prevent_savefiles;

        /** Sound config **/
        /* Bit depth of the buffer (usually 8 or 16) */
        int audio_bitdepth;

        /* Number of channels of the buffer */
        int audio_channels;

        /* Frequency of buffer in Hz */
        int audio_frequency;



        void save_config();

        void load_config();

};

extern struct Config config;

#endif
