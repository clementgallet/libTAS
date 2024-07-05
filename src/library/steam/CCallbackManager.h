/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_CCALLBACK_MANAGER_H_INCL
#define LIBTAS_CCALLBACK_MANAGER_H_INCL

#include "CCallback.h"
#include "steamtypes.h"

namespace libtas {

namespace CCallbackManager {

int Init(void);
void RegisterCallback(CCallbackBase *callback, enum steam_callback_type type);
void UnregisterCallback(CCallbackBase *callback);
void RegisterApiCallResult(CCallbackBase *callback, SteamAPICall_t api_call);
void UnregisterApiCallResult(CCallbackBase *callback, SteamAPICall_t api_call);
void DispatchCallbackOutput(enum steam_callback_type type, void *data, size_t data_size);
SteamAPICall_t AwaitApiCallResultOutput(void);
void DispatchApiCallResultOutput(SteamAPICall_t api_call, enum steam_callback_type type, bool io_failure, void *data, size_t data_size);
bool ApiCallResultIsOutputAvailable(SteamAPICall_t api_call, bool *io_failure);
bool ApiCallResultGetOutput(SteamAPICall_t api_call, void *data, int data_size, enum steam_callback_type type_expected, bool *io_failure);
void Run(void);

}
}

#endif /* CALLBACKS_H */
