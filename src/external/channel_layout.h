/*
  * Copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
  * Copyright (c) 2008 Peter Ross
  *
  * This file is part of FFmpeg.
  *
  * FFmpeg is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * FFmpeg is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with FFmpeg; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
  */
  
 #ifndef LIBTAS_AVUTIL_CHANNEL_LAYOUT_H
 #define LIBTAS_AVUTIL_CHANNEL_LAYOUT_H
  
 #include <stdint.h>
 // #include <stdlib.h>
  
 // #include "version.h"
 // #include "attributes.h"
  
 /**
  * @file
  * @ingroup lavu_audio_channels
  * Public libavutil channel layout APIs header.
  */
  
  
 /**
  * @defgroup lavu_audio_channels Audio channels
  * @ingroup lavu_audio
  *
  * Audio channel layout utility functions
  *
  * @{
  */
  
 enum AVChannel {
     ///< Invalid channel index
     AV_CHAN_NONE = -1,
     AV_CHAN_FRONT_LEFT,
     AV_CHAN_FRONT_RIGHT,
     AV_CHAN_FRONT_CENTER,
     AV_CHAN_LOW_FREQUENCY,
     AV_CHAN_BACK_LEFT,
     AV_CHAN_BACK_RIGHT,
     AV_CHAN_FRONT_LEFT_OF_CENTER,
     AV_CHAN_FRONT_RIGHT_OF_CENTER,
     AV_CHAN_BACK_CENTER,
     AV_CHAN_SIDE_LEFT,
     AV_CHAN_SIDE_RIGHT,
     AV_CHAN_TOP_CENTER,
     AV_CHAN_TOP_FRONT_LEFT,
     AV_CHAN_TOP_FRONT_CENTER,
     AV_CHAN_TOP_FRONT_RIGHT,
     AV_CHAN_TOP_BACK_LEFT,
     AV_CHAN_TOP_BACK_CENTER,
     AV_CHAN_TOP_BACK_RIGHT,
     /** Stereo downmix. */
     AV_CHAN_STEREO_LEFT = 29,
     /** See above. */
     AV_CHAN_STEREO_RIGHT,
     AV_CHAN_WIDE_LEFT,
     AV_CHAN_WIDE_RIGHT,
     AV_CHAN_SURROUND_DIRECT_LEFT,
     AV_CHAN_SURROUND_DIRECT_RIGHT,
     AV_CHAN_LOW_FREQUENCY_2,
     AV_CHAN_TOP_SIDE_LEFT,
     AV_CHAN_TOP_SIDE_RIGHT,
     AV_CHAN_BOTTOM_FRONT_CENTER,
     AV_CHAN_BOTTOM_FRONT_LEFT,
     AV_CHAN_BOTTOM_FRONT_RIGHT,
  
     /** Channel is empty can be safely skipped. */
     AV_CHAN_UNUSED = 0x200,
  
     /** Channel contains data, but its position is unknown. */
     AV_CHAN_UNKNOWN = 0x300,
  
     /**
      * Range of channels between AV_CHAN_AMBISONIC_BASE and
      * AV_CHAN_AMBISONIC_END represent Ambisonic components using the ACN system.
      *
      * Given a channel id `<i>` between AV_CHAN_AMBISONIC_BASE and
      * AV_CHAN_AMBISONIC_END (inclusive), the ACN index of the channel `<n>` is
      * `<n> = <i> - AV_CHAN_AMBISONIC_BASE`.
      *
      * @note these values are only used for AV_CHANNEL_ORDER_CUSTOM channel
      * orderings, the AV_CHANNEL_ORDER_AMBISONIC ordering orders the channels
      * implicitly by their position in the stream.
      */
     AV_CHAN_AMBISONIC_BASE = 0x400,
     // leave space for 1024 ids, which correspond to maximum order-32 harmonics,
     // which should be enough for the foreseeable use cases
     AV_CHAN_AMBISONIC_END  = 0x7ff,
 };
  
 enum AVChannelOrder {
     /**
      * Only the channel count is specified, without any further information
      * about the channel order.
      */
     AV_CHANNEL_ORDER_UNSPEC,
     /**
      * The native channel order, i.e. the channels are in the same order in
      * which they are defined in the AVChannel enum. This supports up to 63
      * different channels.
      */
     AV_CHANNEL_ORDER_NATIVE,
     /**
      * The channel order does not correspond to any other predefined order and
      * is stored as an explicit map. For example, this could be used to support
      * layouts with 64 or more channels, or with empty/skipped (AV_CHAN_SILENCE)
      * channels at arbitrary positions.
      */
     AV_CHANNEL_ORDER_CUSTOM,
     /**
      * The audio is represented as the decomposition of the sound field into
      * spherical harmonics. Each channel corresponds to a single expansion
      * component. Channels are ordered according to ACN (Ambisonic Channel
      * Number).
      *
      * The channel with the index n in the stream contains the spherical
      * harmonic of degree l and order m given by
      * @code{.unparsed}
      *   l   = floor(sqrt(n)),
      *   m   = n - l * (l + 1).
      * @endcode
      *
      * Conversely given a spherical harmonic of degree l and order m, the
      * corresponding channel index n is given by
      * @code{.unparsed}
      *   n = l * (l + 1) + m.
      * @endcode
      *
      * Normalization is assumed to be SN3D (Schmidt Semi-Normalization)
      * as defined in AmbiX format $ 2.1.
      */
     AV_CHANNEL_ORDER_AMBISONIC,
 };
  
  
 /**
  * @defgroup channel_masks Audio channel masks
  *
  * A channel layout is a 64-bits integer with a bit set for every channel.
  * The number of bits set must be equal to the number of channels.
  * The value 0 means that the channel layout is not known.
  * @note this data structure is not powerful enough to handle channels
  * combinations that have the same channel multiple times, such as
  * dual-mono.
  *
  * @{
  */
 #define AV_CH_FRONT_LEFT             (1ULL << AV_CHAN_FRONT_LEFT           )
 #define AV_CH_FRONT_RIGHT            (1ULL << AV_CHAN_FRONT_RIGHT          )
 #define AV_CH_FRONT_CENTER           (1ULL << AV_CHAN_FRONT_CENTER         )
 #define AV_CH_LOW_FREQUENCY          (1ULL << AV_CHAN_LOW_FREQUENCY        )
 #define AV_CH_BACK_LEFT              (1ULL << AV_CHAN_BACK_LEFT            )
 #define AV_CH_BACK_RIGHT             (1ULL << AV_CHAN_BACK_RIGHT           )
 #define AV_CH_FRONT_LEFT_OF_CENTER   (1ULL << AV_CHAN_FRONT_LEFT_OF_CENTER )
 #define AV_CH_FRONT_RIGHT_OF_CENTER  (1ULL << AV_CHAN_FRONT_RIGHT_OF_CENTER)
 #define AV_CH_BACK_CENTER            (1ULL << AV_CHAN_BACK_CENTER          )
 #define AV_CH_SIDE_LEFT              (1ULL << AV_CHAN_SIDE_LEFT            )
 #define AV_CH_SIDE_RIGHT             (1ULL << AV_CHAN_SIDE_RIGHT           )
 #define AV_CH_TOP_CENTER             (1ULL << AV_CHAN_TOP_CENTER           )
 #define AV_CH_TOP_FRONT_LEFT         (1ULL << AV_CHAN_TOP_FRONT_LEFT       )
 #define AV_CH_TOP_FRONT_CENTER       (1ULL << AV_CHAN_TOP_FRONT_CENTER     )
 #define AV_CH_TOP_FRONT_RIGHT        (1ULL << AV_CHAN_TOP_FRONT_RIGHT      )
 #define AV_CH_TOP_BACK_LEFT          (1ULL << AV_CHAN_TOP_BACK_LEFT        )
 #define AV_CH_TOP_BACK_CENTER        (1ULL << AV_CHAN_TOP_BACK_CENTER      )
 #define AV_CH_TOP_BACK_RIGHT         (1ULL << AV_CHAN_TOP_BACK_RIGHT       )
 #define AV_CH_STEREO_LEFT            (1ULL << AV_CHAN_STEREO_LEFT          )
 #define AV_CH_STEREO_RIGHT           (1ULL << AV_CHAN_STEREO_RIGHT         )
 #define AV_CH_WIDE_LEFT              (1ULL << AV_CHAN_WIDE_LEFT            )
 #define AV_CH_WIDE_RIGHT             (1ULL << AV_CHAN_WIDE_RIGHT           )
 #define AV_CH_SURROUND_DIRECT_LEFT   (1ULL << AV_CHAN_SURROUND_DIRECT_LEFT )
 #define AV_CH_SURROUND_DIRECT_RIGHT  (1ULL << AV_CHAN_SURROUND_DIRECT_RIGHT)
 #define AV_CH_LOW_FREQUENCY_2        (1ULL << AV_CHAN_LOW_FREQUENCY_2      )
 #define AV_CH_TOP_SIDE_LEFT          (1ULL << AV_CHAN_TOP_SIDE_LEFT        )
 #define AV_CH_TOP_SIDE_RIGHT         (1ULL << AV_CHAN_TOP_SIDE_RIGHT       )
 #define AV_CH_BOTTOM_FRONT_CENTER    (1ULL << AV_CHAN_BOTTOM_FRONT_CENTER  )
 #define AV_CH_BOTTOM_FRONT_LEFT      (1ULL << AV_CHAN_BOTTOM_FRONT_LEFT    )
 #define AV_CH_BOTTOM_FRONT_RIGHT     (1ULL << AV_CHAN_BOTTOM_FRONT_RIGHT   )
  
 /**
  * @}
  * @defgroup channel_mask_c Audio channel layouts
  * @{
  * */
 #define AV_CH_LAYOUT_MONO              (AV_CH_FRONT_CENTER)
 #define AV_CH_LAYOUT_STEREO            (AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT)
 #define AV_CH_LAYOUT_2POINT1           (AV_CH_LAYOUT_STEREO|AV_CH_LOW_FREQUENCY)
 #define AV_CH_LAYOUT_2_1               (AV_CH_LAYOUT_STEREO|AV_CH_BACK_CENTER)
 #define AV_CH_LAYOUT_SURROUND          (AV_CH_LAYOUT_STEREO|AV_CH_FRONT_CENTER)
 #define AV_CH_LAYOUT_3POINT1           (AV_CH_LAYOUT_SURROUND|AV_CH_LOW_FREQUENCY)
 #define AV_CH_LAYOUT_4POINT0           (AV_CH_LAYOUT_SURROUND|AV_CH_BACK_CENTER)
 #define AV_CH_LAYOUT_4POINT1           (AV_CH_LAYOUT_4POINT0|AV_CH_LOW_FREQUENCY)
 #define AV_CH_LAYOUT_2_2               (AV_CH_LAYOUT_STEREO|AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT)
 #define AV_CH_LAYOUT_QUAD              (AV_CH_LAYOUT_STEREO|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
 #define AV_CH_LAYOUT_5POINT0           (AV_CH_LAYOUT_SURROUND|AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT)
 #define AV_CH_LAYOUT_5POINT1           (AV_CH_LAYOUT_5POINT0|AV_CH_LOW_FREQUENCY)
 #define AV_CH_LAYOUT_5POINT0_BACK      (AV_CH_LAYOUT_SURROUND|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
 #define AV_CH_LAYOUT_5POINT1_BACK      (AV_CH_LAYOUT_5POINT0_BACK|AV_CH_LOW_FREQUENCY)
 #define AV_CH_LAYOUT_6POINT0           (AV_CH_LAYOUT_5POINT0|AV_CH_BACK_CENTER)
 #define AV_CH_LAYOUT_6POINT0_FRONT     (AV_CH_LAYOUT_2_2|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
 #define AV_CH_LAYOUT_HEXAGONAL         (AV_CH_LAYOUT_5POINT0_BACK|AV_CH_BACK_CENTER)
 #define AV_CH_LAYOUT_6POINT1           (AV_CH_LAYOUT_5POINT1|AV_CH_BACK_CENTER)
 #define AV_CH_LAYOUT_6POINT1_BACK      (AV_CH_LAYOUT_5POINT1_BACK|AV_CH_BACK_CENTER)
 #define AV_CH_LAYOUT_6POINT1_FRONT     (AV_CH_LAYOUT_6POINT0_FRONT|AV_CH_LOW_FREQUENCY)
 #define AV_CH_LAYOUT_7POINT0           (AV_CH_LAYOUT_5POINT0|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
 #define AV_CH_LAYOUT_7POINT0_FRONT     (AV_CH_LAYOUT_5POINT0|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
 #define AV_CH_LAYOUT_7POINT1           (AV_CH_LAYOUT_5POINT1|AV_CH_BACK_LEFT|AV_CH_BACK_RIGHT)
 #define AV_CH_LAYOUT_7POINT1_WIDE      (AV_CH_LAYOUT_5POINT1|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
 #define AV_CH_LAYOUT_7POINT1_WIDE_BACK (AV_CH_LAYOUT_5POINT1_BACK|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER)
 #define AV_CH_LAYOUT_7POINT1_TOP_BACK  (AV_CH_LAYOUT_5POINT1_BACK|AV_CH_TOP_FRONT_LEFT|AV_CH_TOP_FRONT_RIGHT)
 #define AV_CH_LAYOUT_OCTAGONAL         (AV_CH_LAYOUT_5POINT0|AV_CH_BACK_LEFT|AV_CH_BACK_CENTER|AV_CH_BACK_RIGHT)
 #define AV_CH_LAYOUT_CUBE              (AV_CH_LAYOUT_QUAD|AV_CH_TOP_FRONT_LEFT|AV_CH_TOP_FRONT_RIGHT|AV_CH_TOP_BACK_LEFT|AV_CH_TOP_BACK_RIGHT)
 #define AV_CH_LAYOUT_HEXADECAGONAL     (AV_CH_LAYOUT_OCTAGONAL|AV_CH_WIDE_LEFT|AV_CH_WIDE_RIGHT|AV_CH_TOP_BACK_LEFT|AV_CH_TOP_BACK_RIGHT|AV_CH_TOP_BACK_CENTER|AV_CH_TOP_FRONT_CENTER|AV_CH_TOP_FRONT_LEFT|AV_CH_TOP_FRONT_RIGHT)
 #define AV_CH_LAYOUT_STEREO_DOWNMIX    (AV_CH_STEREO_LEFT|AV_CH_STEREO_RIGHT)
 #define AV_CH_LAYOUT_22POINT2          (AV_CH_LAYOUT_5POINT1_BACK|AV_CH_FRONT_LEFT_OF_CENTER|AV_CH_FRONT_RIGHT_OF_CENTER|AV_CH_BACK_CENTER|AV_CH_LOW_FREQUENCY_2|AV_CH_SIDE_LEFT|AV_CH_SIDE_RIGHT|AV_CH_TOP_FRONT_LEFT|AV_CH_TOP_FRONT_RIGHT|AV_CH_TOP_FRONT_CENTER|AV_CH_TOP_CENTER|AV_CH_TOP_BACK_LEFT|AV_CH_TOP_BACK_RIGHT|AV_CH_TOP_SIDE_LEFT|AV_CH_TOP_SIDE_RIGHT|AV_CH_TOP_BACK_CENTER|AV_CH_BOTTOM_FRONT_CENTER|AV_CH_BOTTOM_FRONT_LEFT|AV_CH_BOTTOM_FRONT_RIGHT)
  
 enum AVMatrixEncoding {
     AV_MATRIX_ENCODING_NONE,
     AV_MATRIX_ENCODING_DOLBY,
     AV_MATRIX_ENCODING_DPLII,
     AV_MATRIX_ENCODING_DPLIIX,
     AV_MATRIX_ENCODING_DPLIIZ,
     AV_MATRIX_ENCODING_DOLBYEX,
     AV_MATRIX_ENCODING_DOLBYHEADPHONE,
     AV_MATRIX_ENCODING_NB
 };
  
 /**
  * @}
  */
  
 /**
  * An AVChannelCustom defines a single channel within a custom order layout
  *
  * Unlike most structures in FFmpeg, sizeof(AVChannelCustom) is a part of the
  * public ABI.
  *
  * No new fields may be added to it without a major version bump.
  */
 typedef struct AVChannelCustom {
     enum AVChannel id;
     char name[16];
     void *opaque;
 } AVChannelCustom;
  
 /**
  * An AVChannelLayout holds information about the channel layout of audio data.
  *
  * A channel layout here is defined as a set of channels ordered in a specific
  * way (unless the channel order is AV_CHANNEL_ORDER_UNSPEC, in which case an
  * AVChannelLayout carries only the channel count).
  * All orders may be treated as if they were AV_CHANNEL_ORDER_UNSPEC by
  * ignoring everything but the channel count, as long as av_channel_layout_check()
  * considers they are valid.
  *
  * Unlike most structures in FFmpeg, sizeof(AVChannelLayout) is a part of the
  * public ABI and may be used by the caller. E.g. it may be allocated on stack
  * or embedded in caller-defined structs.
  *
  * AVChannelLayout can be initialized as follows:
  * - default initialization with {0}, followed by setting all used fields
  *   correctly;
  * - by assigning one of the predefined AV_CHANNEL_LAYOUT_* initializers;
  * - with a constructor function, such as av_channel_layout_default(),
  *   av_channel_layout_from_mask() or av_channel_layout_from_string().
  *
  * The channel layout must be unitialized with av_channel_layout_uninit()
  *
  * Copying an AVChannelLayout via assigning is forbidden,
  * av_channel_layout_copy() must be used instead (and its return value should
  * be checked)
  *
  * No new fields may be added to it without a major version bump, except for
  * new elements of the union fitting in sizeof(uint64_t).
  */
 typedef struct AVChannelLayout {
     /**
      * Channel order used in this layout.
      * This is a mandatory field.
      */
     enum AVChannelOrder order;
  
     /**
      * Number of channels in this layout. Mandatory field.
      */
     int nb_channels;
  
     /**
      * Details about which channels are present in this layout.
      * For AV_CHANNEL_ORDER_UNSPEC, this field is undefined and must not be
      * used.
      */
     union {
         /**
          * This member must be used for AV_CHANNEL_ORDER_NATIVE, and may be used
          * for AV_CHANNEL_ORDER_AMBISONIC to signal non-diegetic channels.
          * It is a bitmask, where the position of each set bit means that the
          * AVChannel with the corresponding value is present.
          *
          * I.e. when (mask & (1 << AV_CHAN_FOO)) is non-zero, then AV_CHAN_FOO
          * is present in the layout. Otherwise it is not present.
          *
          * @note when a channel layout using a bitmask is constructed or
          * modified manually (i.e.  not using any of the av_channel_layout_*
          * functions), the code doing it must ensure that the number of set bits
          * is equal to nb_channels.
          */
         uint64_t mask;
         /**
          * This member must be used when the channel order is
          * AV_CHANNEL_ORDER_CUSTOM. It is a nb_channels-sized array, with each
          * element signalling the presence of the AVChannel with the
          * corresponding value in map[i].id.
          *
          * I.e. when map[i].id is equal to AV_CHAN_FOO, then AV_CH_FOO is the
          * i-th channel in the audio data.
          *
          * When map[i].id is in the range between AV_CHAN_AMBISONIC_BASE and
          * AV_CHAN_AMBISONIC_END (inclusive), the channel contains an ambisonic
          * component with ACN index (as defined above)
          * n = map[i].id - AV_CHAN_AMBISONIC_BASE.
          *
          * map[i].name may be filled with a 0-terminated string, in which case
          * it will be used for the purpose of identifying the channel with the
          * convenience functions below. Otherise it must be zeroed.
          */
         AVChannelCustom *map;
     } u;
  
     /**
      * For some private data of the user.
      */
     void *opaque;
 } AVChannelLayout;
  
 #define AV_CHANNEL_LAYOUT_MASK(nb, m) \
     { .order = AV_CHANNEL_ORDER_NATIVE, .nb_channels = (nb), .u = { .mask = (m) }}
  
 /**
  * @name Common pre-defined channel layouts
  * @{
  */
 #define AV_CHANNEL_LAYOUT_MONO              AV_CHANNEL_LAYOUT_MASK(1,  AV_CH_LAYOUT_MONO)
 #define AV_CHANNEL_LAYOUT_STEREO            AV_CHANNEL_LAYOUT_MASK(2,  AV_CH_LAYOUT_STEREO)
 #define AV_CHANNEL_LAYOUT_2POINT1           AV_CHANNEL_LAYOUT_MASK(3,  AV_CH_LAYOUT_2POINT1)
 #define AV_CHANNEL_LAYOUT_2_1               AV_CHANNEL_LAYOUT_MASK(3,  AV_CH_LAYOUT_2_1)
 #define AV_CHANNEL_LAYOUT_SURROUND          AV_CHANNEL_LAYOUT_MASK(3,  AV_CH_LAYOUT_SURROUND)
 #define AV_CHANNEL_LAYOUT_3POINT1           AV_CHANNEL_LAYOUT_MASK(4,  AV_CH_LAYOUT_3POINT1)
 #define AV_CHANNEL_LAYOUT_4POINT0           AV_CHANNEL_LAYOUT_MASK(4,  AV_CH_LAYOUT_4POINT0)
 #define AV_CHANNEL_LAYOUT_4POINT1           AV_CHANNEL_LAYOUT_MASK(5,  AV_CH_LAYOUT_4POINT1)
 #define AV_CHANNEL_LAYOUT_2_2               AV_CHANNEL_LAYOUT_MASK(4,  AV_CH_LAYOUT_2_2)
 #define AV_CHANNEL_LAYOUT_QUAD              AV_CHANNEL_LAYOUT_MASK(4,  AV_CH_LAYOUT_QUAD)
 #define AV_CHANNEL_LAYOUT_5POINT0           AV_CHANNEL_LAYOUT_MASK(5,  AV_CH_LAYOUT_5POINT0)
 #define AV_CHANNEL_LAYOUT_5POINT1           AV_CHANNEL_LAYOUT_MASK(6,  AV_CH_LAYOUT_5POINT1)
 #define AV_CHANNEL_LAYOUT_5POINT0_BACK      AV_CHANNEL_LAYOUT_MASK(5,  AV_CH_LAYOUT_5POINT0_BACK)
 #define AV_CHANNEL_LAYOUT_5POINT1_BACK      AV_CHANNEL_LAYOUT_MASK(6,  AV_CH_LAYOUT_5POINT1_BACK)
 #define AV_CHANNEL_LAYOUT_6POINT0           AV_CHANNEL_LAYOUT_MASK(6,  AV_CH_LAYOUT_6POINT0)
 #define AV_CHANNEL_LAYOUT_6POINT0_FRONT     AV_CHANNEL_LAYOUT_MASK(6,  AV_CH_LAYOUT_6POINT0_FRONT)
 #define AV_CHANNEL_LAYOUT_HEXAGONAL         AV_CHANNEL_LAYOUT_MASK(6,  AV_CH_LAYOUT_HEXAGONAL)
 #define AV_CHANNEL_LAYOUT_6POINT1           AV_CHANNEL_LAYOUT_MASK(7,  AV_CH_LAYOUT_6POINT1)
 #define AV_CHANNEL_LAYOUT_6POINT1_BACK      AV_CHANNEL_LAYOUT_MASK(7,  AV_CH_LAYOUT_6POINT1_BACK)
 #define AV_CHANNEL_LAYOUT_6POINT1_FRONT     AV_CHANNEL_LAYOUT_MASK(7,  AV_CH_LAYOUT_6POINT1_FRONT)
 #define AV_CHANNEL_LAYOUT_7POINT0           AV_CHANNEL_LAYOUT_MASK(7,  AV_CH_LAYOUT_7POINT0)
 #define AV_CHANNEL_LAYOUT_7POINT0_FRONT     AV_CHANNEL_LAYOUT_MASK(7,  AV_CH_LAYOUT_7POINT0_FRONT)
 #define AV_CHANNEL_LAYOUT_7POINT1           AV_CHANNEL_LAYOUT_MASK(8,  AV_CH_LAYOUT_7POINT1)
 #define AV_CHANNEL_LAYOUT_7POINT1_WIDE      AV_CHANNEL_LAYOUT_MASK(8,  AV_CH_LAYOUT_7POINT1_WIDE)
 #define AV_CHANNEL_LAYOUT_7POINT1_WIDE_BACK AV_CHANNEL_LAYOUT_MASK(8,  AV_CH_LAYOUT_7POINT1_WIDE_BACK)
 #define AV_CHANNEL_LAYOUT_7POINT1_TOP_BACK  AV_CHANNEL_LAYOUT_MASK(8,  AV_CH_LAYOUT_7POINT1_TOP_BACK)
 #define AV_CHANNEL_LAYOUT_OCTAGONAL         AV_CHANNEL_LAYOUT_MASK(8,  AV_CH_LAYOUT_OCTAGONAL)
 #define AV_CHANNEL_LAYOUT_CUBE              AV_CHANNEL_LAYOUT_MASK(8,  AV_CH_LAYOUT_CUBE)
 #define AV_CHANNEL_LAYOUT_HEXADECAGONAL     AV_CHANNEL_LAYOUT_MASK(16, AV_CH_LAYOUT_HEXADECAGONAL)
 #define AV_CHANNEL_LAYOUT_STEREO_DOWNMIX    AV_CHANNEL_LAYOUT_MASK(2,  AV_CH_LAYOUT_STEREO_DOWNMIX)
 #define AV_CHANNEL_LAYOUT_22POINT2          AV_CHANNEL_LAYOUT_MASK(24, AV_CH_LAYOUT_22POINT2)
 #define AV_CHANNEL_LAYOUT_AMBISONIC_FIRST_ORDER \
     { .order = AV_CHANNEL_ORDER_AMBISONIC, .nb_channels = 4, .u = { .mask = 0 }}
 /** @} */
  
 #endif /* AVUTIL_CHANNEL_LAYOUT_H */
