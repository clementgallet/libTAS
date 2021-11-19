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

#ifndef LIBTAS_OPENAL_H_INCL
#define LIBTAS_OPENAL_H_INCL

#include "../../global.h"

typedef char ALboolean;
typedef char ALchar;
typedef signed char ALbyte;
typedef unsigned char ALubyte;
typedef short ALshort;
typedef unsigned short ALushort;
typedef int ALint;
typedef unsigned int ALuint;
typedef int ALsizei;
typedef int ALenum;
typedef float ALfloat;
typedef double ALdouble;
typedef void ALvoid;

/** "no distance model" or "no buffer" */
#define AL_NONE                                  0

/** Boolean False. */
#define AL_FALSE                                 0

/** Boolean True. */
#define AL_TRUE                                  1

/**
 * Relative source.
 * Type:    ALboolean
 * Range:   [AL_TRUE, AL_FALSE]
 * Default: AL_FALSE
 *
 * Specifies if the Source has relative coordinates.
 */
#define AL_SOURCE_RELATIVE                       0x202

/**
 * Inner cone angle, in degrees.
 * Type:    ALint, ALfloat
 * Range:   [0 - 360]
 * Default: 360
 *
 * The angle covered by the inner cone, where the source will not attenuate.
 */
#define AL_CONE_INNER_ANGLE                      0x1001

/**
 * Outer cone angle, in degrees.
 * Range:   [0 - 360]
 * Default: 360
 *
 * The angle covered by the outer cone, where the source will be fully
 * attenuated.
 */
#define AL_CONE_OUTER_ANGLE                      0x1002

/**
 * Source pitch.
 * Type:    ALfloat
 * Range:   [0.5 - 2.0]
 * Default: 1.0
 *
 * A multiplier for the frequency (sample rate) of the source's buffer.
 */
#define AL_PITCH                                 0x1003

/**
 * Source or listener position.
 * Type:    ALfloat[3], ALint[3]
 * Default: {0, 0, 0}
 *
 * The source or listener location in three dimensional space.
 *
 * OpenAL, like OpenGL, uses a right handed coordinate system, where in a
 * frontal default view X (thumb) points right, Y points up (index finger), and
 * Z points towards the viewer/camera (middle finger).
 *
 * To switch from a left handed coordinate system, flip the sign on the Z
 * coordinate.
 */
#define AL_POSITION                              0x1004

/**
 * Source direction.
 * Type:    ALfloat[3], ALint[3]
 * Default: {0, 0, 0}
 *
 * Specifies the current direction in local space.
 * A zero-length vector specifies an omni-directional source (cone is ignored).
 */
#define AL_DIRECTION                             0x1005

/**
 * Source or listener velocity.
 * Type:    ALfloat[3], ALint[3]
 * Default: {0, 0, 0}
 *
 * Specifies the current velocity in local space.
 */
#define AL_VELOCITY                              0x1006

/**
 * Source looping.
 * Type:    ALboolean
 * Range:   [AL_TRUE, AL_FALSE]
 * Default: AL_FALSE
 *
 * Specifies whether source is looping.
 */
#define AL_LOOPING                               0x1007

/**
 * Source buffer.
 * Type:  ALuint
 * Range: any valid Buffer.
 *
 * Specifies the buffer to provide sound samples.
 */
#define AL_BUFFER                                0x1009

/**
 * Source or listener gain.
 * Type:  ALfloat
 * Range: [0.0 - ]
 *
 * A value of 1.0 means unattenuated. Each division by 2 equals an attenuation
 * of about -6dB. Each multiplicaton by 2 equals an amplification of about
 * +6dB.
 *
 * A value of 0.0 is meaningless with respect to a logarithmic scale; it is
 * silent.
 */
#define AL_GAIN                                  0x100A

/**
 * Minimum source gain.
 * Type:  ALfloat
 * Range: [0.0 - 1.0]
 *
 * The minimum gain allowed for a source, after distance and cone attenation is
 * applied (if applicable).
 */
#define AL_MIN_GAIN                              0x100D

/**
 * Maximum source gain.
 * Type:  ALfloat
 * Range: [0.0 - 1.0]
 *
 * The maximum gain allowed for a source, after distance and cone attenation is
 * applied (if applicable).
 */
#define AL_MAX_GAIN                              0x100E

/**
 * Listener orientation.
 * Type: ALfloat[6]
 * Default: {0.0, 0.0, -1.0, 0.0, 1.0, 0.0}
 *
 * Effectively two three dimensional vectors. The first vector is the front (or
 * "at") and the second is the top (or "up").
 *
 * Both vectors are in local space.
 */
#define AL_ORIENTATION                           0x100F

/**
 * Source state (query only).
 * Type:  ALint
 * Range: [AL_INITIAL, AL_PLAYING, AL_PAUSED, AL_STOPPED]
 */
#define AL_SOURCE_STATE                          0x1010

/** Source state value. */
#define AL_INITIAL                               0x1011
#define AL_PLAYING                               0x1012
#define AL_PAUSED                                0x1013
#define AL_STOPPED                               0x1014

/**
 * Source Buffer Queue size (query only).
 * Type: ALint
 *
 * The number of buffers queued using alSourceQueueBuffers, minus the buffers
 * removed with alSourceUnqueueBuffers.
 */
#define AL_BUFFERS_QUEUED                        0x1015

/**
 * Source Buffer Queue processed count (query only).
 * Type: ALint
 *
 * The number of queued buffers that have been fully processed, and can be
 * removed with alSourceUnqueueBuffers.
 *
 * Looping sources will never fully process buffers because they will be set to
 * play again for when the source loops.
 */
#define AL_BUFFERS_PROCESSED                     0x1016

/**
 * Source reference distance.
 * Type:    ALfloat
 * Range:   [0.0 - ]
 * Default: 1.0
 *
 * The distance in units that no attenuation occurs.
 *
 * At 0.0, no distance attenuation ever occurs on non-linear attenuation models.
 */
#define AL_REFERENCE_DISTANCE                    0x1020

/**
 * Source rolloff factor.
 * Type:    ALfloat
 * Range:   [0.0 - ]
 * Default: 1.0
 *
 * Multiplier to exaggerate or diminish distance attenuation.
 *
 * At 0.0, no distance attenuation ever occurs.
 */
#define AL_ROLLOFF_FACTOR                        0x1021

/**
 * Outer cone gain.
 * Type:    ALfloat
 * Range:   [0.0 - 1.0]
 * Default: 0.0
 *
 * The gain attenuation applied when the listener is outside of the source's
 * outer cone.
 */
#define AL_CONE_OUTER_GAIN                       0x1022

/**
 * Source maximum distance.
 * Type:    ALfloat
 * Range:   [0.0 - ]
 * Default: +inf
 *
 * The distance above which the source is not attenuated any further with a
 * clamped distance model, or where attenuation reaches 0.0 gain for linear
 * distance models with a default rolloff factor.
 */
#define AL_MAX_DISTANCE                          0x1023

/** Source buffer position, in seconds */
#define AL_SEC_OFFSET                            0x1024
/** Source buffer position, in sample frames */
#define AL_SAMPLE_OFFSET                         0x1025
/** Source buffer position, in bytes */
#define AL_BYTE_OFFSET                           0x1026

/**
 * Source type (query only).
 * Type:  ALint
 * Range: [AL_STATIC, AL_STREAMING, AL_UNDETERMINED]
 *
 * A Source is Static if a Buffer has been attached using AL_BUFFER.
 *
 * A Source is Streaming if one or more Buffers have been attached using
 * alSourceQueueBuffers.
 *
 * A Source is Undetermined when it has the NULL buffer attached using
 * AL_BUFFER.
 */
#define AL_SOURCE_TYPE                           0x1027

/** Source type value. */
#define AL_STATIC                                0x1028
#define AL_STREAMING                             0x1029
#define AL_UNDETERMINED                          0x1030

/** Buffer format specifier. */
#define AL_FORMAT_MONO8                          0x1100
#define AL_FORMAT_MONO16                         0x1101
#define AL_FORMAT_STEREO8                        0x1102
#define AL_FORMAT_STEREO16                       0x1103

/** Buffer frequency (query only). */
#define AL_FREQUENCY                             0x2001
/** Buffer bits per sample (query only). */
#define AL_BITS                                  0x2002
/** Buffer channel count (query only). */
#define AL_CHANNELS                              0x2003
/** Buffer data size (query only). */
#define AL_SIZE                                  0x2004

/**
 * Buffer state.
 *
 * Not for public use.
 */
#define AL_UNUSED                                0x2010
#define AL_PENDING                               0x2011
#define AL_PROCESSED                             0x2012

#define AL_LOOP_POINTS_SOFT                      0x2015

/** No error. */
#define AL_NO_ERROR                              0

/** Invalid name paramater passed to AL call. */
#define AL_INVALID_NAME                          0xA001

/** Invalid enum parameter passed to AL call. */
#define AL_INVALID_ENUM                          0xA002

/** Invalid value parameter passed to AL call. */
#define AL_INVALID_VALUE                         0xA003

/** Illegal AL call. */
#define AL_INVALID_OPERATION                     0xA004

/** Not enough memory. */
#define AL_OUT_OF_MEMORY                         0xA005


/** Context string: Vendor ID. */
#define AL_VENDOR                                0xB001
/** Context string: Version. */
#define AL_VERSION                               0xB002
/** Context string: Renderer ID. */
#define AL_RENDERER                              0xB003
/** Context string: Space-separated extension list. */
#define AL_EXTENSIONS                            0xB004

/*** OpenAL extensions ***/
#define AL_FORMAT_IMA_ADPCM_MONO16_EXT           0x10000
#define AL_FORMAT_IMA_ADPCM_STEREO16_EXT         0x10001

#define AL_FORMAT_WAVE_EXT                       0x10002

#define AL_FORMAT_VORBIS_EXT                     0x10003

#define AL_FORMAT_QUAD8_LOKI                     0x10004
#define AL_FORMAT_QUAD16_LOKI                    0x10005

#define AL_FORMAT_MONO_FLOAT32                   0x10010
#define AL_FORMAT_STEREO_FLOAT32                 0x10011

#define AL_FORMAT_MONO_DOUBLE_EXT                0x10012
#define AL_FORMAT_STEREO_DOUBLE_EXT              0x10013

#define AL_FORMAT_MONO_MULAW_EXT                 0x10014
#define AL_FORMAT_STEREO_MULAW_EXT               0x10015

#define AL_FORMAT_MONO_ALAW_EXT                  0x10016
#define AL_FORMAT_STEREO_ALAW_EXT                0x10017

#define ALC_CHAN_MAIN_LOKI                       0x500001
#define ALC_CHAN_PCM_LOKI                        0x500002
#define ALC_CHAN_CD_LOKI                         0x500003

#define AL_FORMAT_QUAD8                          0x1204
#define AL_FORMAT_QUAD16                         0x1205
#define AL_FORMAT_QUAD32                         0x1206
#define AL_FORMAT_REAR8                          0x1207
#define AL_FORMAT_REAR16                         0x1208
#define AL_FORMAT_REAR32                         0x1209
#define AL_FORMAT_51CHN8                         0x120A
#define AL_FORMAT_51CHN16                        0x120B
#define AL_FORMAT_51CHN32                        0x120C
#define AL_FORMAT_61CHN8                         0x120D
#define AL_FORMAT_61CHN16                        0x120E
#define AL_FORMAT_61CHN32                        0x120F
#define AL_FORMAT_71CHN8                         0x1210
#define AL_FORMAT_71CHN16                        0x1211
#define AL_FORMAT_71CHN32                        0x1212

#define AL_FORMAT_MONO_MULAW                     0x10014
#define AL_FORMAT_STEREO_MULAW                   0x10015
#define AL_FORMAT_QUAD_MULAW                     0x10021
#define AL_FORMAT_REAR_MULAW                     0x10022
#define AL_FORMAT_51CHN_MULAW                    0x10023
#define AL_FORMAT_61CHN_MULAW                    0x10024
#define AL_FORMAT_71CHN_MULAW                    0x10025

#define AL_FORMAT_MONO_IMA4                      0x1300
#define AL_FORMAT_STEREO_IMA4                    0x1301

#define AL_UNPACK_BLOCK_ALIGNMENT_SOFT           0x200C
#define AL_PACK_BLOCK_ALIGNMENT_SOFT             0x200D

#define AL_FORMAT_MONO_MSADPCM_SOFT              0x1302
#define AL_FORMAT_STEREO_MSADPCM_SOFT            0x1303

#define AL_FORMAT_BFORMAT2D_8                    0x20021
#define AL_FORMAT_BFORMAT2D_16                   0x20022
#define AL_FORMAT_BFORMAT2D_FLOAT32              0x20023
#define AL_FORMAT_BFORMAT3D_8                    0x20031
#define AL_FORMAT_BFORMAT3D_16                   0x20032
#define AL_FORMAT_BFORMAT3D_FLOAT32              0x20033

#define AL_FORMAT_BFORMAT2D_MULAW                0x10031
#define AL_FORMAT_BFORMAT3D_MULAW                0x10032

/*** OpenAL functions ***/
namespace libtas {

/** Renderer State management. */
// AL_API void AL_APIENTRY alEnable(ALenum capability);
// AL_API void AL_APIENTRY alDisable(ALenum capability);
// AL_API ALboolean AL_APIENTRY alIsEnabled(ALenum capability);

/** State retrieval. */
OVERRIDE const ALchar* alGetString(ALenum param);
// AL_API void AL_APIENTRY alGetBooleanv(ALenum param, ALboolean *values);
// AL_API void AL_APIENTRY alGetIntegerv(ALenum param, ALint *values);
// AL_API void AL_APIENTRY alGetFloatv(ALenum param, ALfloat *values);
// AL_API void AL_APIENTRY alGetDoublev(ALenum param, ALdouble *values);
// AL_API ALboolean AL_APIENTRY alGetBoolean(ALenum param);
// AL_API ALint AL_APIENTRY alGetInteger(ALenum param);
// AL_API ALfloat AL_APIENTRY alGetFloat(ALenum param);
// AL_API ALdouble AL_APIENTRY alGetDouble(ALenum param);


OVERRIDE ALenum alGetError(ALvoid);
void alSetError(ALenum error);

/**
 * Extension support.
 *
 * Query for the presence of an extension, and obtain any appropriate function
 * pointers and enum values.
 */
OVERRIDE ALboolean alIsExtensionPresent(const ALchar *extname);
OVERRIDE void* alGetProcAddress(const ALchar *fname);
// OVERRIDE ALenum alGetEnumValue(const ALchar *ename);

/** Create Buffer objects */
OVERRIDE void alGenBuffers(ALsizei n, ALuint *buffers);
/** Delete Buffer objects */
OVERRIDE void alDeleteBuffers(ALsizei n, ALuint *buffers);
/** Verify a handle is a valid Buffer */
OVERRIDE ALboolean alIsBuffer(ALuint buffer);

/** Specifies the data to be copied into a buffer */
OVERRIDE void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);

/** Set Buffer parameters, */
OVERRIDE void alBufferf(ALuint buffer, ALenum param, ALfloat value);
OVERRIDE void alBuffer3f(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
OVERRIDE void alBufferfv(ALuint buffer, ALenum param, const ALfloat *values);
OVERRIDE void alBufferi(ALuint buffer, ALenum param, ALint value);
OVERRIDE void alBuffer3i(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3);
OVERRIDE void alBufferiv(ALuint buffer, ALenum param, const ALint *values);

/** Get Buffer parameters. */
OVERRIDE void alGetBufferi(ALuint buffer, ALenum pname, ALint *value);
OVERRIDE void alGetBufferiv(ALuint buffer, ALenum pname, ALint *values);

/** Create Source objects. */
OVERRIDE void alGenSources(ALsizei n, ALuint *sources);
/** Delete Source objects. */
OVERRIDE void alDeleteSources(ALsizei n, ALuint *sources);
/** Verify a handle is a valid Source. */
OVERRIDE ALboolean alIsSource(ALuint source);

/** Set Source parameters. */
OVERRIDE void alSourcef(ALuint source, ALenum param, ALfloat value);
OVERRIDE void alSource3f(ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
OVERRIDE void alSourcefv(ALuint source, ALenum param, ALfloat *values);
OVERRIDE void alSourcei(ALuint source, ALenum param, ALint value);
OVERRIDE void alSource3i(ALuint source, ALenum param, ALint v1, ALint v2, ALint v3);
OVERRIDE void alSourceiv(ALuint source, ALenum param, ALint *values);

/** Get Source parameters. */
OVERRIDE void alGetSourcef(ALuint source, ALenum param, ALfloat *value);
OVERRIDE void alGetSource3f(ALuint source, ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3);
OVERRIDE void alGetSourcefv(ALuint source, ALenum param, ALfloat *values);
OVERRIDE void alGetSourcei(ALuint source, ALenum param, ALint *value);
OVERRIDE void alGetSource3i(ALuint source, ALenum param, ALint *v1, ALint *v2, ALint *v3);
OVERRIDE void alGetSourceiv(ALuint source, ALenum param, ALint *values);

/** Play, replay, or resume (if paused) a list of Sources */
OVERRIDE void alSourcePlayv(ALsizei n, ALuint *sources);
/** Stop a list of Sources */
OVERRIDE void alSourceStopv(ALsizei n, ALuint *sources);
/** Rewind a list of Sources */
OVERRIDE void alSourceRewindv(ALsizei n, ALuint *sources);
/** Pause a list of Sources */
OVERRIDE void alSourcePausev(ALsizei n, ALuint *sources);

/** Play, replay, or resume a Source */
OVERRIDE void alSourcePlay(ALuint source);
/** Stop a Source */
OVERRIDE void alSourceStop(ALuint source);
/** Rewind a Source (set playback postiton to beginning) */
OVERRIDE void alSourceRewind(ALuint source);
/** Pause a Source */
OVERRIDE void alSourcePause(ALuint source);

/** Queue buffers onto a source */
OVERRIDE void alSourceQueueBuffers(ALuint source, ALsizei n, ALuint* buffers);
/** Unqueue processed buffers from a source */
OVERRIDE void alSourceUnqueueBuffers(ALuint source, ALsizei n, ALuint* buffers);

/** Set Listener parameters */
OVERRIDE void alListenerf(ALenum param, ALfloat value);
OVERRIDE void alListener3f(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
OVERRIDE void alListenerfv(ALenum param, ALfloat *values);
OVERRIDE void alListeneri(ALenum param, ALint value);
OVERRIDE void alListener3i(ALenum param, ALint v1, ALint v2, ALint v3);
OVERRIDE void alListeneriv(ALenum param, ALint *values);

/** Get Listener parameters */
OVERRIDE void alGetListenerf(ALenum param, ALfloat *value);
OVERRIDE void alGetListener3f(ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3);
OVERRIDE void alGetListenerfv(ALenum param, ALfloat *values);
OVERRIDE void alGetListeneri(ALenum param, ALint *value);
OVERRIDE void alGetListener3i(ALenum param, ALint *v1, ALint *v2, ALint *v3);
OVERRIDE void alGetListeneriv(ALenum param, ALint *values);

}

#endif
