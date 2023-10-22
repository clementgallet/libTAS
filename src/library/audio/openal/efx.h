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

#ifndef LIBTAS_OPENALEFX_H_INCL
#define LIBTAS_OPENALEFX_H_INCL

#include "al.h"
#include "../../hook.h"

#define ALC_EXT_EFX_NAME                         "ALC_EXT_EFX"

#define ALC_EFX_MAJOR_VERSION                    0x20001
#define ALC_EFX_MINOR_VERSION                    0x20002
#define ALC_MAX_AUXILIARY_SENDS                  0x20003


/* Listener properties. */
#define AL_METERS_PER_UNIT                       0x20004

/* Source properties. */
#define AL_DIRECT_FILTER                         0x20005
#define AL_AUXILIARY_SEND_FILTER                 0x20006
#define AL_AIR_ABSORPTION_FACTOR                 0x20007
#define AL_ROOM_ROLLOFF_FACTOR                   0x20008
#define AL_CONE_OUTER_GAINHF                     0x20009
#define AL_DIRECT_FILTER_GAINHF_AUTO             0x2000A
#define AL_AUXILIARY_SEND_FILTER_GAIN_AUTO       0x2000B
#define AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO     0x2000C


/* Effect properties. */

/* Reverb effect parameters */
#define AL_REVERB_DENSITY                        0x0001
#define AL_REVERB_DIFFUSION                      0x0002
#define AL_REVERB_GAIN                           0x0003
#define AL_REVERB_GAINHF                         0x0004
#define AL_REVERB_DECAY_TIME                     0x0005
#define AL_REVERB_DECAY_HFRATIO                  0x0006
#define AL_REVERB_REFLECTIONS_GAIN               0x0007
#define AL_REVERB_REFLECTIONS_DELAY              0x0008
#define AL_REVERB_LATE_REVERB_GAIN               0x0009
#define AL_REVERB_LATE_REVERB_DELAY              0x000A
#define AL_REVERB_AIR_ABSORPTION_GAINHF          0x000B
#define AL_REVERB_ROOM_ROLLOFF_FACTOR            0x000C
#define AL_REVERB_DECAY_HFLIMIT                  0x000D

/* EAX Reverb effect parameters */
#define AL_EAXREVERB_DENSITY                     0x0001
#define AL_EAXREVERB_DIFFUSION                   0x0002
#define AL_EAXREVERB_GAIN                        0x0003
#define AL_EAXREVERB_GAINHF                      0x0004
#define AL_EAXREVERB_GAINLF                      0x0005
#define AL_EAXREVERB_DECAY_TIME                  0x0006
#define AL_EAXREVERB_DECAY_HFRATIO               0x0007
#define AL_EAXREVERB_DECAY_LFRATIO               0x0008
#define AL_EAXREVERB_REFLECTIONS_GAIN            0x0009
#define AL_EAXREVERB_REFLECTIONS_DELAY           0x000A
#define AL_EAXREVERB_REFLECTIONS_PAN             0x000B
#define AL_EAXREVERB_LATE_REVERB_GAIN            0x000C
#define AL_EAXREVERB_LATE_REVERB_DELAY           0x000D
#define AL_EAXREVERB_LATE_REVERB_PAN             0x000E
#define AL_EAXREVERB_ECHO_TIME                   0x000F
#define AL_EAXREVERB_ECHO_DEPTH                  0x0010
#define AL_EAXREVERB_MODULATION_TIME             0x0011
#define AL_EAXREVERB_MODULATION_DEPTH            0x0012
#define AL_EAXREVERB_AIR_ABSORPTION_GAINHF       0x0013
#define AL_EAXREVERB_HFREFERENCE                 0x0014
#define AL_EAXREVERB_LFREFERENCE                 0x0015
#define AL_EAXREVERB_ROOM_ROLLOFF_FACTOR         0x0016
#define AL_EAXREVERB_DECAY_HFLIMIT               0x0017

/* Chorus effect parameters */
#define AL_CHORUS_WAVEFORM                       0x0001
#define AL_CHORUS_PHASE                          0x0002
#define AL_CHORUS_RATE                           0x0003
#define AL_CHORUS_DEPTH                          0x0004
#define AL_CHORUS_FEEDBACK                       0x0005
#define AL_CHORUS_DELAY                          0x0006

/* Distortion effect parameters */
#define AL_DISTORTION_EDGE                       0x0001
#define AL_DISTORTION_GAIN                       0x0002
#define AL_DISTORTION_LOWPASS_CUTOFF             0x0003
#define AL_DISTORTION_EQCENTER                   0x0004
#define AL_DISTORTION_EQBANDWIDTH                0x0005

/* Echo effect parameters */
#define AL_ECHO_DELAY                            0x0001
#define AL_ECHO_LRDELAY                          0x0002
#define AL_ECHO_DAMPING                          0x0003
#define AL_ECHO_FEEDBACK                         0x0004
#define AL_ECHO_SPREAD                           0x0005

/* Flanger effect parameters */
#define AL_FLANGER_WAVEFORM                      0x0001
#define AL_FLANGER_PHASE                         0x0002
#define AL_FLANGER_RATE                          0x0003
#define AL_FLANGER_DEPTH                         0x0004
#define AL_FLANGER_FEEDBACK                      0x0005
#define AL_FLANGER_DELAY                         0x0006

/* Frequency shifter effect parameters */
#define AL_FREQUENCY_SHIFTER_FREQUENCY           0x0001
#define AL_FREQUENCY_SHIFTER_LEFT_DIRECTION      0x0002
#define AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION     0x0003

/* Vocal morpher effect parameters */
#define AL_VOCAL_MORPHER_PHONEMEA                0x0001
#define AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING  0x0002
#define AL_VOCAL_MORPHER_PHONEMEB                0x0003
#define AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING  0x0004
#define AL_VOCAL_MORPHER_WAVEFORM                0x0005
#define AL_VOCAL_MORPHER_RATE                    0x0006

/* Pitchshifter effect parameters */
#define AL_PITCH_SHIFTER_COARSE_TUNE             0x0001
#define AL_PITCH_SHIFTER_FINE_TUNE               0x0002

/* Ringmodulator effect parameters */
#define AL_RING_MODULATOR_FREQUENCY              0x0001
#define AL_RING_MODULATOR_HIGHPASS_CUTOFF        0x0002
#define AL_RING_MODULATOR_WAVEFORM               0x0003

/* Autowah effect parameters */
#define AL_AUTOWAH_ATTACK_TIME                   0x0001
#define AL_AUTOWAH_RELEASE_TIME                  0x0002
#define AL_AUTOWAH_RESONANCE                     0x0003
#define AL_AUTOWAH_PEAK_GAIN                     0x0004

/* Compressor effect parameters */
#define AL_COMPRESSOR_ONOFF                      0x0001

/* Equalizer effect parameters */
#define AL_EQUALIZER_LOW_GAIN                    0x0001
#define AL_EQUALIZER_LOW_CUTOFF                  0x0002
#define AL_EQUALIZER_MID1_GAIN                   0x0003
#define AL_EQUALIZER_MID1_CENTER                 0x0004
#define AL_EQUALIZER_MID1_WIDTH                  0x0005
#define AL_EQUALIZER_MID2_GAIN                   0x0006
#define AL_EQUALIZER_MID2_CENTER                 0x0007
#define AL_EQUALIZER_MID2_WIDTH                  0x0008
#define AL_EQUALIZER_HIGH_GAIN                   0x0009
#define AL_EQUALIZER_HIGH_CUTOFF                 0x000A

/* Effect type */
#define AL_EFFECT_FIRST_PARAMETER                0x0000
#define AL_EFFECT_LAST_PARAMETER                 0x8000
#define AL_EFFECT_TYPE                           0x8001

/* Effect types, used with the AL_EFFECT_TYPE property */
#define AL_EFFECT_NULL                           0x0000
#define AL_EFFECT_REVERB                         0x0001
#define AL_EFFECT_CHORUS                         0x0002
#define AL_EFFECT_DISTORTION                     0x0003
#define AL_EFFECT_ECHO                           0x0004
#define AL_EFFECT_FLANGER                        0x0005
#define AL_EFFECT_FREQUENCY_SHIFTER              0x0006
#define AL_EFFECT_VOCAL_MORPHER                  0x0007
#define AL_EFFECT_PITCH_SHIFTER                  0x0008
#define AL_EFFECT_RING_MODULATOR                 0x0009
#define AL_EFFECT_AUTOWAH                        0x000A
#define AL_EFFECT_COMPRESSOR                     0x000B
#define AL_EFFECT_EQUALIZER                      0x000C
#define AL_EFFECT_EAXREVERB                      0x8000

/* Auxiliary Effect Slot properties. */
#define AL_EFFECTSLOT_EFFECT                     0x0001
#define AL_EFFECTSLOT_GAIN                       0x0002
#define AL_EFFECTSLOT_AUXILIARY_SEND_AUTO        0x0003

/* NULL Auxiliary Slot ID to disable a source send. */
#define AL_EFFECTSLOT_NULL                       0x0000


/* Filter properties. */

/* Lowpass filter parameters */
#define AL_LOWPASS_GAIN                          0x0001
#define AL_LOWPASS_GAINHF                        0x0002

/* Highpass filter parameters */
#define AL_HIGHPASS_GAIN                         0x0001
#define AL_HIGHPASS_GAINLF                       0x0002

/* Bandpass filter parameters */
#define AL_BANDPASS_GAIN                         0x0001
#define AL_BANDPASS_GAINLF                       0x0002
#define AL_BANDPASS_GAINHF                       0x0003

/* Filter type */
#define AL_FILTER_FIRST_PARAMETER                0x0000
#define AL_FILTER_LAST_PARAMETER                 0x8000
#define AL_FILTER_TYPE                           0x8001

/* Filter types, used with the AL_FILTER_TYPE property */
#define AL_FILTER_NULL                           0x0000
#define AL_FILTER_LOWPASS                        0x0001
#define AL_FILTER_HIGHPASS                       0x0002
#define AL_FILTER_BANDPASS                       0x0003


/*** OpenAL functions ***/
namespace libtas {

OVERRIDE ALvoid myalGenEffects(ALsizei n, ALuint *effects);
OVERRIDE ALvoid myalDeleteEffects(ALsizei n, const ALuint *effects);
OVERRIDE ALboolean myalIsEffect(ALuint effect);
OVERRIDE ALvoid myalEffecti(ALuint effect, ALenum param, ALint iValue);
OVERRIDE ALvoid myalEffectiv(ALuint effect, ALenum param, const ALint *piValues);
OVERRIDE ALvoid myalEffectf(ALuint effect, ALenum param, ALfloat flValue);
OVERRIDE ALvoid myalEffectfv(ALuint effect, ALenum param, const ALfloat *pflValues);
OVERRIDE ALvoid myalGetEffecti(ALuint effect, ALenum param, ALint *piValue);
OVERRIDE ALvoid myalGetEffectiv(ALuint effect, ALenum param, ALint *piValues);
OVERRIDE ALvoid myalGetEffectf(ALuint effect, ALenum param, ALfloat *pflValue);
OVERRIDE ALvoid myalGetEffectfv(ALuint effect, ALenum param, ALfloat *pflValues);

OVERRIDE ALvoid myalGenFilters(ALsizei n, ALuint *filters);
OVERRIDE ALvoid myalDeleteFilters(ALsizei n, const ALuint *filters);
OVERRIDE ALboolean myalIsFilter(ALuint filter);
OVERRIDE ALvoid myalFilteri(ALuint filter, ALenum param, ALint iValue);
OVERRIDE ALvoid myalFilteriv(ALuint filter, ALenum param, const ALint *piValues);
OVERRIDE ALvoid myalFilterf(ALuint filter, ALenum param, ALfloat flValue);
OVERRIDE ALvoid myalFilterfv(ALuint filter, ALenum param, const ALfloat *pflValues);
OVERRIDE ALvoid myalGetFilteri(ALuint filter, ALenum param, ALint *piValue);
OVERRIDE ALvoid myalGetFilteriv(ALuint filter, ALenum param, ALint *piValues);
OVERRIDE ALvoid myalGetFilterf(ALuint filter, ALenum param, ALfloat *pflValue);
OVERRIDE ALvoid myalGetFilterfv(ALuint filter, ALenum param, ALfloat *pflValues);

OVERRIDE ALvoid myalGenAuxiliaryEffectSlots(ALsizei n, ALuint *effectslots);
OVERRIDE ALvoid myalDeleteAuxiliaryEffectSlots(ALsizei n, const ALuint *effectslots);
OVERRIDE ALboolean myalIsAuxiliaryEffectSlot(ALuint effectslot);
OVERRIDE ALvoid myalAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint iValue);
OVERRIDE ALvoid myalAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, const ALint *piValues);
OVERRIDE ALvoid myalAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat flValue);
OVERRIDE ALvoid myalAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, const ALfloat *pflValues);
OVERRIDE ALvoid myalGetAuxiliaryEffectSloti(ALuint effectslot, ALenum param, ALint *piValue);
OVERRIDE ALvoid myalGetAuxiliaryEffectSlotiv(ALuint effectslot, ALenum param, ALint *piValues);
OVERRIDE ALvoid myalGetAuxiliaryEffectSlotf(ALuint effectslot, ALenum param, ALfloat *pflValue);
OVERRIDE ALvoid myalGetAuxiliaryEffectSlotfv(ALuint effectslot, ALenum param, ALfloat *pflValues);

}

#endif
