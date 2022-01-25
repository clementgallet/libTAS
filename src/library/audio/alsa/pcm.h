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

#ifndef LIBTAS_ALSA_PCM_H_INCL
#define LIBTAS_ALSA_PCM_H_INCL

#include "../../global.h"
#include <alsa/asoundlib.h>

namespace libtas {

OVERRIDE int snd_pcm_open(snd_pcm_t **pcm, const char *name,
    		 snd_pcm_stream_t stream, int mode);
OVERRIDE int snd_pcm_open_lconf(snd_pcm_t **pcm, const char *name,
    		       snd_pcm_stream_t stream, int mode,
    		       snd_config_t *lconf);
OVERRIDE int snd_pcm_open_fallback(snd_pcm_t **pcm, snd_config_t *root,
    			  const char *name, const char *orig_name,
    			  snd_pcm_stream_t stream, int mode);

OVERRIDE int snd_pcm_close(snd_pcm_t *pcm);
    // const char *snd_pcm_name(snd_pcm_t *pcm);
    // snd_pcm_type_t snd_pcm_type(snd_pcm_t *pcm);
    // snd_pcm_stream_t snd_pcm_stream(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_poll_descriptors_count(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space);
OVERRIDE int snd_pcm_poll_descriptors_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents);
OVERRIDE int snd_pcm_nonblock(snd_pcm_t *pcm, int nonblock);
    // static __inline__ int snd_pcm_abort(snd_pcm_t *pcm) { return snd_pcm_nonblock(pcm, 2); }
    // int snd_async_add_pcm_handler(snd_async_handler_t **handler, snd_pcm_t *pcm,
    // 			      snd_async_callback_t callback, void *private_data);
    // snd_pcm_t *snd_async_handler_get_pcm(snd_async_handler_t *handler);
OVERRIDE int snd_pcm_info(snd_pcm_t *pcm, snd_pcm_info_t *info);
OVERRIDE int snd_pcm_hw_params_current(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
OVERRIDE int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
    // int snd_pcm_hw_free(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_sw_params_current(snd_pcm_t *pcm, snd_pcm_sw_params_t *params);
OVERRIDE int snd_pcm_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params);
OVERRIDE int snd_pcm_prepare(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_reset(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_status(snd_pcm_t *pcm, snd_pcm_status_t *status);
OVERRIDE int snd_pcm_start(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_drop(snd_pcm_t *pcm);
    // int snd_pcm_drain(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_hw_params_can_pause(const snd_pcm_hw_params_t *params);
OVERRIDE int snd_pcm_pause(snd_pcm_t *pcm, int enable);
OVERRIDE snd_pcm_state_t snd_pcm_state(snd_pcm_t *pcm);
    // int snd_pcm_hwsync(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp);
OVERRIDE int snd_pcm_resume(snd_pcm_t *pcm);
    // int snd_pcm_htimestamp(snd_pcm_t *pcm, snd_pcm_uframes_t *avail, snd_htimestamp_t *tstamp);
OVERRIDE snd_pcm_sframes_t snd_pcm_avail(snd_pcm_t *pcm);
OVERRIDE snd_pcm_sframes_t snd_pcm_avail_update(snd_pcm_t *pcm);
    // int snd_pcm_avail_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *availp, snd_pcm_sframes_t *delayp);
    // snd_pcm_sframes_t snd_pcm_rewindable(snd_pcm_t *pcm);
OVERRIDE snd_pcm_sframes_t snd_pcm_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames);
    // snd_pcm_sframes_t snd_pcm_forwardable(snd_pcm_t *pcm);
    // snd_pcm_sframes_t snd_pcm_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames);
OVERRIDE snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size);
OVERRIDE snd_pcm_sframes_t snd_pcm_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size);
    // snd_pcm_sframes_t snd_pcm_writen(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size);
    // snd_pcm_sframes_t snd_pcm_readn(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size);
OVERRIDE int snd_pcm_wait(snd_pcm_t *pcm, int timeout);
    //
    // int snd_pcm_link(snd_pcm_t *pcm1, snd_pcm_t *pcm2);
    // int snd_pcm_unlink(snd_pcm_t *pcm);
OVERRIDE int snd_pcm_recover(snd_pcm_t *pcm, int err, int silent);

OVERRIDE snd_pcm_sframes_t snd_pcm_bytes_to_frames(snd_pcm_t *pcm, ssize_t bytes);
OVERRIDE ssize_t snd_pcm_frames_to_bytes(snd_pcm_t *pcm, snd_pcm_sframes_t frames);


OVERRIDE int snd_pcm_mmap_begin(snd_pcm_t *pcm, const snd_pcm_channel_area_t **areas, snd_pcm_uframes_t *offset, snd_pcm_uframes_t *frames);
OVERRIDE snd_pcm_sframes_t snd_pcm_mmap_commit(snd_pcm_t *pcm, snd_pcm_uframes_t offset, snd_pcm_uframes_t frames);


OVERRIDE int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params);
OVERRIDE size_t snd_pcm_hw_params_sizeof(void);
OVERRIDE int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **ptr);
OVERRIDE void snd_pcm_hw_params_free(snd_pcm_hw_params_t *obj);
OVERRIDE void snd_pcm_hw_params_copy(snd_pcm_hw_params_t *dst, const snd_pcm_hw_params_t *src);
OVERRIDE int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access);
OVERRIDE int snd_pcm_hw_params_get_access(const snd_pcm_hw_params_t *params, snd_pcm_access_t *_access);
OVERRIDE int snd_pcm_hw_params_set_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val);
OVERRIDE void snd_pcm_hw_params_get_format_mask(snd_pcm_hw_params_t *params, snd_pcm_format_mask_t *mask);

OVERRIDE int snd_pcm_hw_params_get_channels(const snd_pcm_hw_params_t *params, unsigned int *val);
OVERRIDE int snd_pcm_hw_params_get_channels_min(const snd_pcm_hw_params_t *params, unsigned int *val);
OVERRIDE int snd_pcm_hw_params_get_channels_max(const snd_pcm_hw_params_t *params, unsigned int *val);
OVERRIDE int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val);

OVERRIDE int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir);
OVERRIDE int snd_pcm_hw_params_set_rate_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
OVERRIDE int snd_pcm_hw_params_set_rate_resample(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val);
OVERRIDE int snd_pcm_hw_params_get_rate(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
OVERRIDE int snd_pcm_hw_params_get_rate_min(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
OVERRIDE int snd_pcm_hw_params_get_rate_max(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir);

OVERRIDE int snd_pcm_hw_params_get_period_size(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *frames, int *dir);
OVERRIDE int snd_pcm_hw_params_get_period_time_min(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
OVERRIDE int snd_pcm_hw_params_set_period_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
OVERRIDE int snd_pcm_hw_params_set_period_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val, int *dir);
OVERRIDE int snd_pcm_hw_params_set_periods_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
OVERRIDE int snd_pcm_hw_params_get_periods(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir);

OVERRIDE int snd_pcm_hw_params_get_buffer_size(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val);
OVERRIDE int snd_pcm_hw_params_get_buffer_size_min(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val);
OVERRIDE int snd_pcm_hw_params_get_buffer_size_max(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val);
OVERRIDE int snd_pcm_hw_params_get_buffer_time_max(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir);
OVERRIDE int snd_pcm_hw_params_set_buffer_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val);
OVERRIDE int snd_pcm_hw_params_set_buffer_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir);

OVERRIDE int snd_pcm_hw_params_test_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir);
OVERRIDE int snd_pcm_hw_params_test_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val);
OVERRIDE int snd_pcm_hw_params_test_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val);

OVERRIDE int snd_pcm_get_params(snd_pcm_t *pcm, snd_pcm_uframes_t *buffer_size, snd_pcm_uframes_t *period_size);
OVERRIDE int snd_pcm_set_params(snd_pcm_t *pcm, snd_pcm_format_t format, snd_pcm_access_t access, unsigned int channels, unsigned int rate, int soft_resample, unsigned int latency);

OVERRIDE size_t snd_pcm_sw_params_sizeof(void);

// int snd_pcm_sw_params_set_tstamp_mode(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_tstamp_t val);
// int snd_pcm_sw_params_get_tstamp_mode(const snd_pcm_sw_params_t *params, snd_pcm_tstamp_t *val);
// int snd_pcm_sw_params_set_tstamp_type(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_tstamp_type_t val);
// int snd_pcm_sw_params_get_tstamp_type(const snd_pcm_sw_params_t *params, snd_pcm_tstamp_type_t *val);
OVERRIDE int snd_pcm_sw_params_set_avail_min(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val);
// int snd_pcm_sw_params_get_avail_min(const snd_pcm_sw_params_t *params, snd_pcm_uframes_t *val);
// int snd_pcm_sw_params_set_period_event(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, int val);
// int snd_pcm_sw_params_get_period_event(const snd_pcm_sw_params_t *params, int *val);
OVERRIDE int snd_pcm_sw_params_set_start_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val);
// int snd_pcm_sw_params_get_start_threshold(const snd_pcm_sw_params_t *paramsm, snd_pcm_uframes_t *val);
OVERRIDE int snd_pcm_sw_params_set_stop_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val);
// int snd_pcm_sw_params_get_stop_threshold(const snd_pcm_sw_params_t *params, snd_pcm_uframes_t *val);
// int snd_pcm_sw_params_set_silence_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val);
// int snd_pcm_sw_params_get_silence_threshold(const snd_pcm_sw_params_t *params, snd_pcm_uframes_t *val);
// int snd_pcm_sw_params_set_silence_size(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val);
// int snd_pcm_sw_params_get_silence_size(const snd_pcm_sw_params_t *params, snd_pcm_uframes_t *val);

// snd_pcm_chmap_query_t **snd_pcm_query_chmaps(snd_pcm_t *pcm);
// snd_pcm_chmap_query_t **snd_pcm_query_chmaps_from_hw(int card, int dev, int subdev, snd_pcm_stream_t stream);
// void snd_pcm_free_chmaps(snd_pcm_chmap_query_t **maps);
OVERRIDE snd_pcm_chmap_t *snd_pcm_get_chmap(snd_pcm_t *pcm);
// int snd_pcm_set_chmap(snd_pcm_t *pcm, const snd_pcm_chmap_t *map);

// const char *snd_pcm_chmap_type_name(enum snd_pcm_chmap_type val);
// const char *snd_pcm_chmap_name(enum snd_pcm_chmap_position val);
// const char *snd_pcm_chmap_long_name(enum snd_pcm_chmap_position val);
// int snd_pcm_chmap_print(const snd_pcm_chmap_t *map, size_t maxlen, char *buf);
// unsigned int snd_pcm_chmap_from_string(const char *str);
// snd_pcm_chmap_t *snd_pcm_chmap_parse_string(const char *str);

OVERRIDE int snd_pcm_format_mask_malloc(snd_pcm_format_mask_t **ptr);
OVERRIDE void snd_pcm_format_mask_free(snd_pcm_format_mask_t *obj);
// void snd_pcm_format_mask_copy(snd_pcm_format_mask_t *dst, const snd_pcm_format_mask_t *src);
// void snd_pcm_format_mask_none(snd_pcm_format_mask_t *mask);
// void snd_pcm_format_mask_any(snd_pcm_format_mask_t *mask);
OVERRIDE int snd_pcm_format_mask_test(const snd_pcm_format_mask_t *mask, snd_pcm_format_t val);
// int snd_pcm_format_mask_empty(const snd_pcm_format_mask_t *mask);
// void snd_pcm_format_mask_set(snd_pcm_format_mask_t *mask, snd_pcm_format_t val);
// void snd_pcm_format_mask_reset(snd_pcm_format_mask_t *mask, snd_pcm_format_t val);


}

#endif
