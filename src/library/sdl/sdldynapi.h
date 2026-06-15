/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLDYNAPI_H_INCLUDED
#define LIBTAS_SDLDYNAPI_H_INCLUDED

#include "hook.h"

#include "../external/SDL2.h"
#include "../external/SDL3.h"

#include "sdl/sdlversion.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

namespace libtas {

/* The SDL hook layer receives raw argument words from the trampoline, and we
 * may need to forward them to both SDL2 and SDL3 entry points of the same
 * symbol name but with different parameter types. Decode the storage back to
 * the real C++ types before calling the appropriate implementation.
 */
template <typename T>
constexpr std::size_t storage_words_for() noexcept
{
    return (sizeof(T) + sizeof(std::uintptr_t) - 1) / sizeof(std::uintptr_t);
}

template <typename T>
T decode_storage(const std::uintptr_t* storage, std::size_t offset) noexcept
{
    static_assert(std::is_trivially_copyable_v<T>, "SDL argument type must be trivially copyable");

    T value{};
    std::array<std::uint8_t, sizeof(T)> bytes{};
    std::size_t consumed = 0;

    while (consumed < sizeof(T)) {
        const std::size_t chunk = std::min(sizeof(std::uintptr_t), sizeof(T) - consumed);
        std::memcpy(bytes.data() + consumed, storage + offset, chunk);
        ++offset;
        consumed += chunk;
    }

    std::memcpy(&value, bytes.data(), sizeof(T));
    return value;
}

template <typename R, typename... Args, std::size_t... Is>
R invoke_from_storage_impl(R (*fn)(Args...), const std::uintptr_t* storage, std::index_sequence<Is...>)
{
    std::array<std::size_t, sizeof...(Args)> offsets{};
    std::size_t cursor = 0;

    ((offsets[Is] = cursor, cursor += storage_words_for<Args>()), ...);

    return fn(decode_storage<Args>(storage, offsets[Is])...);
}

template <typename R, typename... Args>
R invoke_from_storage(R (*fn)(Args...), const std::uintptr_t* storage)
{
    return invoke_from_storage_impl(fn, storage, std::index_sequence_for<Args...>{});
}

template <typename... Args>
bool invoke_from_storage(void (*fn)(Args...), const std::uintptr_t* storage)
{
    invoke_from_storage_impl(fn, storage, std::index_sequence_for<Args...>{});
    return true;
}

template <typename T>
using common_return_t = std::conditional_t<std::is_void_v<T>, bool, T>;

template <typename R2, typename R3, bool HasVoidR2 = std::is_void_v<R2>, bool HasVoidR3 = std::is_void_v<R3>>
struct common_storage_result;

template <typename R2, typename R3>
struct common_storage_result<R2, R3, false, false>
{
    using type = std::common_type_t<R2, R3>;
};

template <typename R2, typename R3>
struct common_storage_result<R2, R3, true, false>
{
    using type = common_return_t<R3>;
};

template <typename R2, typename R3>
struct common_storage_result<R2, R3, false, true>
{
    using type = common_return_t<R2>;
};

template <typename R2, typename R3>
struct common_storage_result<R2, R3, true, true>
{
    using type = void;
};

template <typename R2, typename R3>
using common_storage_result_t = typename common_storage_result<R2, R3>::type;

/* Generic trampoline for SDL symbols that exist in both SDL2 and SDL3 with
 * different parameter types. The raw storage words are decoded to the real
 * C++ types for the selected SDL version at the call site.
 */
template <typename R2, typename... Args2, typename R3, typename... Args3>
auto invoke_sdl2_or_sdl3_from_storage(R2 (*sdl2_fn)(Args2...),
                                       R3 (*sdl3_fn)(Args3...),
                                       const std::uintptr_t* storage)
    -> std::enable_if_t<std::is_void_v<common_storage_result_t<R2, R3>>, void>
{
    if (get_sdlversion() == 3) {
        invoke_from_storage(sdl3_fn, storage);
        return;
    }

    invoke_from_storage(sdl2_fn, storage);
}

template <typename R2, typename... Args2, typename R3, typename... Args3>
auto invoke_sdl2_or_sdl3_from_storage(R2 (*sdl2_fn)(Args2...),
                                       R3 (*sdl3_fn)(Args3...),
                                       const std::uintptr_t* storage)
    -> std::enable_if_t<!std::is_void_v<common_storage_result_t<R2, R3>>, common_storage_result_t<R2, R3>>
{
    if (get_sdlversion() == 3)
        return invoke_from_storage(sdl3_fn, storage);

    return invoke_from_storage(sdl2_fn, storage);
}

void setDynapiAddr(uint64_t addr);
void setSDLFullscreenAddr(uint64_t addr);

namespace index_sdl2 {
    enum {
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) fn,
#define SDL_DYNAPI_PROC_NO_VARARGS 0
#include "../../external/SDL2_dynapi_procs.h"
#undef SDL_DYNAPI_PROC_NO_VARARGS
#undef SDL_DYNAPI_PROC
        SDL_EnumCount
    };
}

namespace index_sdl3 {
    enum {
#define SDL_DYNAPI_PROC(rc,fn,params,args,ret) fn,
// As opposed to SDL2, in SDL3, we need SDL_DYNAPI_PROC_NO_VARARGS to be not defined!
#include "../../external/SDL3_dynapi_procs.h"
#undef SDL_DYNAPI_PROC
        SDL_EnumCount
    };
}

int getSDLApiver();

/* Return a pointer to the location of the original SDL2 or SDL3 function, based on its index. */
void** getOrigSDLFuncLoc(int index_sdl);
void** getOrigSDLFuncLoc(int index_sdl2, int index_sdl3);

#ifdef __unix__
#define LINK_NAMESPACE_SDL1(FUNC) link_function((void**)&orig::FUNC, #FUNC, "libSDL-1.2.so.0")
#elif defined(__APPLE__) && defined(__MACH__)
#define LINK_NAMESPACE_SDL1(FUNC) link_function((void**)&orig::FUNC, #FUNC, "libSDL-1.2.0.dylib")
#endif

/* Return a pointer to the original SDL2 or SDL3 function */
#define ORIG_SDL2_FUNCTION_POINTER(FUNC) getOrigSDLFuncLoc(index_sdl2::FUNC)
#define ORIG_SDL3_FUNCTION_POINTER(FUNC) getOrigSDLFuncLoc(index_sdl3::FUNC)

/* When both SDL2 and SDL3 functions are available, return either one */
#define ORIG_SDL23_FUNCTION_POINTER(FUNC) getOrigSDLFuncLoc(index_sdl2::FUNC, index_sdl3::FUNC)

/* Call a SDL2 function, and optionally link it if it is not defined already.
 * We also catch here SDL1 functions that are available in SDL2. */
#define ORIG_SDL2_LINK(FUNC) link_function(getOrigSDLFuncLoc(index_sdl2::FUNC), #FUNC, get_sdlversion()==1?"libSDL-1.2.so.0":"libSDL2-2.0.so.0")
#define ORIG_SDL2_CALL(FUNC, PARAMS) reinterpret_cast<decltype(&sdl2::FUNC)>(ORIG_SDL2_LINK(FUNC)) PARAMS

/* Call a SDL3 function */
#define ORIG_SDL3_LINK(FUNC) link_function(getOrigSDLFuncLoc(index_sdl3::FUNC), #FUNC, "libSDL3.so.0")
#define ORIG_SDL3_CALL(FUNC, PARAMS) reinterpret_cast<decltype(&sdl3::FUNC)>(ORIG_SDL3_LINK(FUNC)) PARAMS

/* Call a SDL2 or SDL3 function depending on which version is used. The function signature must be the same!
 * We also catch here SDL1 functions that are available in SDL2/SDL3. */
#define ORIG_SDL23_LINK(FUNC) link_function(getOrigSDLFuncLoc(index_sdl2::FUNC, index_sdl3::FUNC), #FUNC, get_sdlversion()==1?"libSDL-1.2.so.0":(get_sdlversion()==2?"libSDL2-2.0.so.0":"libSDL3.so.0"))
#define ORIG_SDL23_CALL(FUNC, PARAMS) reinterpret_cast<decltype(&sdl2::FUNC)>(ORIG_SDL23_LINK(FUNC)) PARAMS

/**
 * This function initializes the SDL jump table.
 */
OVERRIDE Sint32 SDL_DYNAPI_entry(Uint32 apiver, void *table, Uint32 tablesize);

}

#endif
