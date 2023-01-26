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

#include "user32.h"
#include "winehook.h"
#include "../hookpatch.h"
#include "../logging.h"
// #include "../backtrace.h"
#include "../checkpoint/ThreadSync.h"
#include "../inputs/winekeyboardlayout.h"
#include "../inputs/inputs.h"
#include "../../shared/SingleInput.h"

namespace libtas {

typedef struct tagPOINT {
    int32_t x;
    int32_t y;
} POINT;


namespace orig {

static short __stdcall __attribute__((noinline)) GetAsyncKeyState(int key)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static bool __stdcall __attribute__((noinline)) GetCursorPos(POINT *pt)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

static bool __stdcall __attribute__((noinline)) ScreenToClient(void *hWnd, POINT *lpPoint)
{
    HOOK_PLACEHOLDER_RETURN_ZERO
}

}

int __stdcall GetCursorPos(POINT *pt)
{
    DEBUGLOGCALL(LCF_WINE | LCF_MOUSE);

    if (pt) {
        pt->x = game_ai.pointer_x;
        pt->y = game_ai.pointer_y;
    }

    // static bool parity = true;
    // if (parity) ThreadSync::detWaitGlobal(0);
    // parity = !parity;
    return 1;
}

int __stdcall ScreenToClient(void *hWnd, POINT *lpPoint)
{
    DEBUGLOGCALL(LCF_WINE | LCF_MOUSE);
    return 1;
}

short __stdcall GetAsyncKeyState(int key)
{
    DEBUGLOGCALL(LCF_WINE | LCF_KEYBOARD);

    short ret = 0;

    /* Check mouse buttons */
    if ((key == 0x01) && (game_ai.pointer_mask & (1 << SingleInput::POINTER_B1)))
        ret |= 0x8000;
    else if ((key == 0x02) && (game_ai.pointer_mask & (1 << SingleInput::POINTER_B2)))
        ret |= 0x8000;
    else if ((key == 0x04) && (game_ai.pointer_mask & (1 << SingleInput::POINTER_B3)))
        ret |= 0x8000;
    else if ((key == 0x05) && (game_ai.pointer_mask & (1 << SingleInput::POINTER_B4)))
        ret |= 0x8000;
    else if ((key == 0x06) && (game_ai.pointer_mask & (1 << SingleInput::POINTER_B5)))
        ret |= 0x8000;

    /* Check keys */
    else {
        KeySym ks = VKeyToXKeysym(key);

        for (int i=0; i<AllInputs::MAXKEYS; i++) {
            if (game_ai.keyboard[i] == 0) {
                break;
            }
            if (game_ai.keyboard[i] == ks) {
                ret |= 0x8000;
                break;
            }
        }
    }

    // if (key == 0xff) {
    //     static bool parity = false;
    //     if (parity) ThreadSync::detSignalGlobal(1);
    //     parity = !parity;
    // }
    return ret;
}

void hook_user32()
{
    HOOK_PATCH_ORIG(GetCursorPos, "user32.dll.so");
    HOOK_PATCH_ORIG(ScreenToClient, "user32.dll.so");
    HOOK_PATCH_ORIG(GetAsyncKeyState, "user32.dll.so");
}


}
