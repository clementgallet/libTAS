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

#ifndef LIBTAS_CUBEB_H_INCL
#define LIBTAS_CUBEB_H_INCL

#include "../../global.h"
#include "../../../external/cubeb.h"

#include <stdint.h>

namespace libtas {

OVERRIDE int cubeb_init(cubeb ** context, char const * context_name, char const * backend_name);

OVERRIDE char const * cubeb_get_backend_id(cubeb * context);

OVERRIDE int cubeb_get_max_channel_count(cubeb * context, uint32_t * max_channels);

OVERRIDE int cubeb_get_min_latency(cubeb * context, cubeb_stream_params * params, uint32_t * latency_frames);

OVERRIDE int cubeb_get_preferred_sample_rate(cubeb * context, uint32_t * rate);

OVERRIDE void cubeb_destroy(cubeb * context);

OVERRIDE int cubeb_stream_init(cubeb * context,
                               cubeb_stream ** stream,
                               char const * stream_name,
                               cubeb_devid input_device,
                               cubeb_stream_params * input_stream_params,
                               cubeb_devid output_device,
                               cubeb_stream_params * output_stream_params,
                               uint32_t latency_frames,
                               cubeb_data_callback data_callback,
                               cubeb_state_callback state_callback,
                               void * user_ptr);

OVERRIDE void cubeb_stream_destroy(cubeb_stream * stream);

OVERRIDE int cubeb_stream_start(cubeb_stream * stream);
OVERRIDE int cubeb_stream_stop(cubeb_stream * stream);
OVERRIDE int cubeb_stream_reset_default_device(cubeb_stream * stream);
OVERRIDE int cubeb_stream_get_position(cubeb_stream * stream, uint64_t * position);
OVERRIDE int cubeb_stream_get_latency(cubeb_stream * stream, uint32_t * latency);
OVERRIDE int cubeb_stream_get_input_latency(cubeb_stream * stream, uint32_t * latency);
OVERRIDE int cubeb_stream_set_volume(cubeb_stream * stream, float volume);
OVERRIDE int cubeb_stream_get_current_device(cubeb_stream * stm,
                                                 cubeb_device ** const device);
OVERRIDE int cubeb_stream_device_destroy(cubeb_stream * stream, cubeb_device * devices);

OVERRIDE int cubeb_stream_register_device_changed_callback(cubeb_stream * stream,
                                                               cubeb_device_changed_callback device_changed_callback);

OVERRIDE void * cubeb_stream_user_ptr(cubeb_stream * stream);

OVERRIDE int cubeb_enumerate_devices(cubeb * context,
                                         cubeb_device_type devtype,
                                         cubeb_device_collection * collection);

OVERRIDE int cubeb_device_collection_destroy(cubeb * context,
                                                 cubeb_device_collection * collection);

OVERRIDE int cubeb_register_device_collection_changed(cubeb * context,
                                                          cubeb_device_type devtype,
                                                          cubeb_device_collection_changed_callback callback,
                                                          void * user_ptr);

OVERRIDE int cubeb_set_log_callback(cubeb_log_level log_level,
                                        cubeb_log_callback log_callback);


}

#endif
