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

#ifndef LIBTAS_PULSEAUDIO_CHANNELMAP_H_INCL
#define LIBTAS_PULSEAUDIO_CHANNELMAP_H_INCL

#include "../../global.h"

namespace libtas {

/** A list of channel labels */
typedef enum pa_channel_position {
    PA_CHANNEL_POSITION_INVALID = -1,
    PA_CHANNEL_POSITION_MONO = 0,

    PA_CHANNEL_POSITION_FRONT_LEFT,               /**< Apple, Dolby call this 'Left' */
    PA_CHANNEL_POSITION_FRONT_RIGHT,              /**< Apple, Dolby call this 'Right' */
    PA_CHANNEL_POSITION_FRONT_CENTER,             /**< Apple, Dolby call this 'Center' */

/** \cond fulldocs */
    PA_CHANNEL_POSITION_LEFT = PA_CHANNEL_POSITION_FRONT_LEFT,
    PA_CHANNEL_POSITION_RIGHT = PA_CHANNEL_POSITION_FRONT_RIGHT,
    PA_CHANNEL_POSITION_CENTER = PA_CHANNEL_POSITION_FRONT_CENTER,
/** \endcond */

    PA_CHANNEL_POSITION_REAR_CENTER,              /**< Microsoft calls this 'Back Center', Apple calls this 'Center Surround', Dolby calls this 'Surround Rear Center' */
    PA_CHANNEL_POSITION_REAR_LEFT,                /**< Microsoft calls this 'Back Left', Apple calls this 'Left Surround' (!), Dolby calls this 'Surround Rear Left'  */
    PA_CHANNEL_POSITION_REAR_RIGHT,               /**< Microsoft calls this 'Back Right', Apple calls this 'Right Surround' (!), Dolby calls this 'Surround Rear Right'  */

    PA_CHANNEL_POSITION_LFE,                      /**< Microsoft calls this 'Low Frequency', Apple calls this 'LFEScreen' */
/** \cond fulldocs */
    PA_CHANNEL_POSITION_SUBWOOFER = PA_CHANNEL_POSITION_LFE,
/** \endcond */

    PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER,     /**< Apple, Dolby call this 'Left Center' */
    PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER,    /**< Apple, Dolby call this 'Right Center */

    PA_CHANNEL_POSITION_SIDE_LEFT,                /**< Apple calls this 'Left Surround Direct', Dolby calls this 'Surround Left' (!) */
    PA_CHANNEL_POSITION_SIDE_RIGHT,               /**< Apple calls this 'Right Surround Direct', Dolby calls this 'Surround Right' (!) */

    PA_CHANNEL_POSITION_AUX0,
    PA_CHANNEL_POSITION_AUX1,
    PA_CHANNEL_POSITION_AUX2,
    PA_CHANNEL_POSITION_AUX3,
    PA_CHANNEL_POSITION_AUX4,
    PA_CHANNEL_POSITION_AUX5,
    PA_CHANNEL_POSITION_AUX6,
    PA_CHANNEL_POSITION_AUX7,
    PA_CHANNEL_POSITION_AUX8,
    PA_CHANNEL_POSITION_AUX9,
    PA_CHANNEL_POSITION_AUX10,
    PA_CHANNEL_POSITION_AUX11,
    PA_CHANNEL_POSITION_AUX12,
    PA_CHANNEL_POSITION_AUX13,
    PA_CHANNEL_POSITION_AUX14,
    PA_CHANNEL_POSITION_AUX15,
    PA_CHANNEL_POSITION_AUX16,
    PA_CHANNEL_POSITION_AUX17,
    PA_CHANNEL_POSITION_AUX18,
    PA_CHANNEL_POSITION_AUX19,
    PA_CHANNEL_POSITION_AUX20,
    PA_CHANNEL_POSITION_AUX21,
    PA_CHANNEL_POSITION_AUX22,
    PA_CHANNEL_POSITION_AUX23,
    PA_CHANNEL_POSITION_AUX24,
    PA_CHANNEL_POSITION_AUX25,
    PA_CHANNEL_POSITION_AUX26,
    PA_CHANNEL_POSITION_AUX27,
    PA_CHANNEL_POSITION_AUX28,
    PA_CHANNEL_POSITION_AUX29,
    PA_CHANNEL_POSITION_AUX30,
    PA_CHANNEL_POSITION_AUX31,

    PA_CHANNEL_POSITION_TOP_CENTER,               /**< Apple calls this 'Top Center Surround' */

    PA_CHANNEL_POSITION_TOP_FRONT_LEFT,           /**< Apple calls this 'Vertical Height Left' */
    PA_CHANNEL_POSITION_TOP_FRONT_RIGHT,          /**< Apple calls this 'Vertical Height Right' */
    PA_CHANNEL_POSITION_TOP_FRONT_CENTER,         /**< Apple calls this 'Vertical Height Center' */

    PA_CHANNEL_POSITION_TOP_REAR_LEFT,            /**< Microsoft and Apple call this 'Top Back Left' */
    PA_CHANNEL_POSITION_TOP_REAR_RIGHT,           /**< Microsoft and Apple call this 'Top Back Right' */
    PA_CHANNEL_POSITION_TOP_REAR_CENTER,          /**< Microsoft and Apple call this 'Top Back Center' */

    PA_CHANNEL_POSITION_MAX
} pa_channel_position_t;

/** A list of channel mapping definitions for pa_channel_map_init_auto() */
typedef enum pa_channel_map_def {
    PA_CHANNEL_MAP_AIFF,
    /**< The mapping from RFC3551, which is based on AIFF-C */

/** \cond fulldocs */
    PA_CHANNEL_MAP_ALSA,
    /**< The default mapping used by ALSA. This mapping is probably
     * not too useful since ALSA's default channel mapping depends on
     * the device string used. */
/** \endcond */

    PA_CHANNEL_MAP_AUX,
    /**< Only aux channels */

    PA_CHANNEL_MAP_WAVEEX,
    /**< Microsoft's WAVEFORMATEXTENSIBLE mapping. This mapping works
     * as if all LSBs of dwChannelMask are set.  */

/** \cond fulldocs */
    PA_CHANNEL_MAP_OSS,
    /**< The default channel mapping used by OSS as defined in the OSS
     * 4.0 API specs. This mapping is probably not too useful since
     * the OSS API has changed in this respect and no longer knows a
     * default channel mapping based on the number of channels. */
/** \endcond */

    /**< Upper limit of valid channel mapping definitions */
    PA_CHANNEL_MAP_DEF_MAX,

    PA_CHANNEL_MAP_DEFAULT = PA_CHANNEL_MAP_AIFF
    /**< The default channel map */
} pa_channel_map_def_t;

/** Maximum number of allowed channels */
#define PA_CHANNELS_MAX 32U

/** A channel map which can be used to attach labels to specific
 * channels of a stream. These values are relevant for conversion and
 * mixing of streams */
typedef struct pa_channel_map {
    uint8_t channels;
    /**< Number of channels */

    pa_channel_position_t map[PA_CHANNELS_MAX];
    /**< Channel labels */
} pa_channel_map;

/** Initialize the specified channel map for the specified number of
 * channels using default labels and return a pointer to it. This call
 * will fail (return NULL) if there is no default channel map known for this
 * specific number of channels and mapping. */
OVERRIDE pa_channel_map* pa_channel_map_init_auto(pa_channel_map *m, unsigned channels, pa_channel_map_def_t def);

}

#endif
