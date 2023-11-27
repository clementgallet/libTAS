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

#ifndef LIBTAS_OPENALEXT_H_INCL
#define LIBTAS_OPENALEXT_H_INCL

#include "hook.h"
#include "al.h"
#include "alc.h"

namespace libtas {

/** Extensions **/
ALCboolean  myalcSetThreadContext(ALCcontext *context);
ALCcontext* myalcGetThreadContext(void);

#define AL_DIRECT_CHANNELS_SOFT                  0x1033
#define ALC_FORMAT_CHANNELS_SOFT                 0x1990
#define ALC_FORMAT_TYPE_SOFT                     0x1991

/* Sample types */
#define ALC_BYTE_SOFT                            0x1400
#define ALC_UNSIGNED_BYTE_SOFT                   0x1401
#define ALC_SHORT_SOFT                           0x1402
#define ALC_UNSIGNED_SHORT_SOFT                  0x1403
#define ALC_INT_SOFT                             0x1404
#define ALC_UNSIGNED_INT_SOFT                    0x1405
#define ALC_FLOAT_SOFT                           0x1406

/* Channel configurations */
#define ALC_MONO_SOFT                            0x1500
#define ALC_STEREO_SOFT                          0x1501
#define ALC_QUAD_SOFT                            0x1503
#define ALC_5POINT1_SOFT                         0x1504
#define ALC_6POINT1_SOFT                         0x1505
#define ALC_7POINT1_SOFT                         0x1506

ALCdevice* myalcLoopbackOpenDeviceSOFT(const ALCchar *deviceName);
ALCboolean myalcIsRenderFormatSupportedSOFT(ALCdevice *device, ALCsizei freq, ALCenum channels, ALCenum type);
void myalcRenderSamplesSOFT(ALCdevice *device, ALCvoid *buffer, ALCsizei samples);

#define ALC_HRTF_SOFT                            0x1992
#define ALC_DONT_CARE_SOFT                       0x0002
#define ALC_HRTF_STATUS_SOFT                     0x1993
#define ALC_HRTF_DISABLED_SOFT                   0x0000
#define ALC_HRTF_ENABLED_SOFT                    0x0001
#define ALC_HRTF_DENIED_SOFT                     0x0002
#define ALC_HRTF_REQUIRED_SOFT                   0x0003
#define ALC_HRTF_HEADPHONES_DETECTED_SOFT        0x0004
#define ALC_HRTF_UNSUPPORTED_FORMAT_SOFT         0x0005
#define ALC_NUM_HRTF_SPECIFIERS_SOFT             0x1994
#define ALC_HRTF_SPECIFIER_SOFT                  0x1995
#define ALC_HRTF_ID_SOFT                         0x1996

const ALCchar* myalcGetStringiSOFT(ALCdevice *device, ALCenum paramName, ALCsizei index);
ALCboolean myalcResetDeviceSOFT(ALCdevice *device, const ALCint *attribs);

void myalcDevicePauseSOFT(ALCdevice *device);
void myalcDeviceResumeSOFT(ALCdevice *device);

/** AL_SOFT_buffer_sub_data extension */
#define AL_BYTE_RW_OFFSETS_SOFT                  0x1031
#define AL_SAMPLE_RW_OFFSETS_SOFT                0x1032

void myalBufferSubDataSOFT(ALuint buffer, ALenum format, const ALvoid *data, ALsizei offset, ALsizei length);
void myalBufferDataStatic(ALint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq);

}

#endif
