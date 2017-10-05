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
#if 0
#include "pcm.h"
#include "../../logging.h"
//#include "../../GlobalState.h"
#include "../../hook.h"
//#include "../../threadwrappers.h" // isMainThread()

namespace libtas {

int snd_pcm_open(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_close(snd_pcm_t *pcm)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_nonblock(snd_pcm_t *pcm, int nonblock)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_prepare(snd_pcm_t *pcm)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
    debuglog(LCF_SOUND, __func__, " call with ", size, " bytes");
    return static_cast<snd_pcm_sframes_t>(size);
}

snd_pcm_sframes_t snd_pcm_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
    debuglog(LCF_SOUND, __func__, " call with ", size, " bytes");
    return static_cast<snd_pcm_sframes_t>(size);
}

int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

size_t snd_pcm_hw_params_sizeof(void)
{
    DEBUGLOGCALL(LCF_SOUND);
    return 8;
}

int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access)
{
    debuglog(LCF_SOUND, __func__, " call with access ", _access);
    return 0;
}

int snd_pcm_hw_params_set_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val)
{
    debuglog(LCF_SOUND, __func__, " call with format ", val);
    return 0;
}

int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
{
    debuglog(LCF_SOUND, __func__, " call with channels ", val);
    return 0;
}

int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir)
{
    debuglog(LCF_SOUND, __func__, " call with rate ", val, " and dir ", dir);
    return 0;
}

int snd_pcm_hw_params_set_period_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val, int *dir)
{
    debuglog(LCF_SOUND, __func__, " call with period size ", *val, " and dir ", *dir);
    return 0;
}

int snd_pcm_hw_params_set_buffer_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
{
    debuglog(LCF_SOUND, __func__, " call with buffer size ", *val);
    return 0;
}

}
#endif
