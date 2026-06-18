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

/* ============================================================================
 * RAW-STORAGE DECODING & SDL2/SDL3 DISPATCHING
 * ============================================================================
 *
 * PROBLEM:
 * - 32-bit games call the 64-bit libtas through a trampoline layer that
 *   receives arguments as raw ABI words (std::uintptr_t).
 * - SDL2 and SDL3 symbols may have different signatures (parameter types,
 *   return types, argument counts) for the same logical function.
 * - We need to: (1) decode raw words to actual C++ types, (2) dispatch to
 *   either SDL2 or SDL3 implementations based on runtime SDL version.
 *
 * SOLUTION:
 * - storage_words_for<T>(): Calculate how many ABI words a type T occupies.
 *   E.g., a 64-bit flag on a 32-bit system needs 2 words (64 bits / 32 bits).
 * - decode_storage<T>(): Extract a value of type T from the word array by
 *   computing the byte offset and memcpying from that position.
 * - invoke_from_storage(): Decode all arguments for a function from storage,
 *   call the function with decoded arguments, and return/convert the result.
 * - invoke_sdl2_or_sdl3_from_storage(): Dispatch to either SDL2 or SDL3
 *   implementation based on get_sdlversion().
 *
 * ============================================================================
 */

/* Calculate how many pointer-sized words are needed to store type T.
 * For a 64-bit value on a 32-bit system, this returns 2.
 * For a 32-bit value on a 64-bit system, this returns 1.
 */
template <typename T>
constexpr std::size_t storage_words_for() noexcept
{
    return (sizeof(T) + sizeof(std::uintptr_t) - 1) / sizeof(std::uintptr_t);
}

/* Convert return type: void functions return bool (true on success),
 * non-void functions return their actual value.
 */
template <typename T>
using common_return_t = std::conditional_t<std::is_void_v<T>, bool, T>;

/* Compute the return type when dispatching between SDL2 (R2) and SDL3 (R3).
 * If both are void, return void.
 * If one is void, return the converted non-void type (void -> bool).
 * If both are non-void, return a common type via std::common_type_t.
 */
template <typename R2, typename R3>
using sdl_dispatch_return_t = std::conditional_t<
    std::is_void_v<R2> && std::is_void_v<R3>,
    void,
    std::conditional_t<std::is_void_v<R2>, common_return_t<R3>,
        std::conditional_t<std::is_void_v<R3>, common_return_t<R2>,
            std::common_type_t<common_return_t<R2>, common_return_t<R3>>
        >
    >
>;

/* Extract a value of type T from raw word storage at a given offset.
 * The offset is in units of pointer-words (e.g., offset=1 on 32-bit means
 * 4 bytes; offset=1 on 64-bit means 8 bytes).
 * This handles multi-word types: a 64-bit value on a 32-bit system spans
 * 2 words and is correctly extracted by this function.
 */
template <typename T>
T decode_storage(const std::uintptr_t* storage, std::size_t offset) noexcept
{
    static_assert(std::is_trivially_copyable_v<T>, "SDL argument type must be trivially copyable");

    T value{};
    constexpr std::size_t word_bytes = sizeof(std::uintptr_t);
    const auto* raw_bytes = reinterpret_cast<const std::uint8_t*>(storage);
    std::memcpy(&value, raw_bytes + offset * word_bytes, sizeof(T));
    return value;
}

/* Internal helper: decode all arguments from storage using compile-time indices.
 * Computes the byte offset for each argument based on its width, decodes it,
 * then calls the function with all decoded arguments.
 */
template <typename R, typename... Args, std::size_t... Is>
common_return_t<R> invoke_from_storage_impl(R (*fn)(Args...), const std::uintptr_t* storage, std::index_sequence<Is...>)
{
    std::array<std::size_t, sizeof...(Args)> offsets{};
    std::size_t cursor = 0;
    ((offsets[Is] = cursor, cursor += storage_words_for<Args>()), ...);

    if constexpr (std::is_void_v<R>) {
        fn(decode_storage<Args>(storage, offsets[Is])...);
        return true;
    } else {
        return fn(decode_storage<Args>(storage, offsets[Is])...);
    }
}

/* Main decoding function: takes a function pointer and raw word storage,
 * decodes all arguments from storage, calls the function, and returns the
 * result (or true for void functions).
 */
template <typename R, typename... Args>
common_return_t<R> invoke_from_storage(R (*fn)(Args...), const std::uintptr_t* storage)
{
    return invoke_from_storage_impl(fn, storage, std::index_sequence_for<Args...>{});
}



/* SDL2/SDL3 DISPATCHER
 * Dispatches to the appropriate SDL implementation based on runtime version.
 * Handles the case where SDL2 and SDL3 have different signatures for the same
 * symbol (e.g., SDL_GetScancodeFromKey has different parameter counts and
 * return types in SDL2 vs SDL3).
 * Arguments and return values are properly decoded/converted based on the
 * selected SDL version.
 */
template <typename R2, typename... Args2, typename R3, typename... Args3>
sdl_dispatch_return_t<R2, R3> invoke_sdl2_or_sdl3_from_storage(R2 (*sdl2_fn)(Args2...),
                                       R3 (*sdl3_fn)(Args3...),
                                       const std::uintptr_t* storage)
{
    if constexpr (std::is_void_v<R2> && std::is_void_v<R3>) {
        if (get_sdlversion() == 3) {
            invoke_from_storage(sdl3_fn, storage);
        } else {
            invoke_from_storage(sdl2_fn, storage);
        }
    } else {
        if (get_sdlversion() == 3)
            return invoke_from_storage(sdl3_fn, storage);

        return invoke_from_storage(sdl2_fn, storage);
    }
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
