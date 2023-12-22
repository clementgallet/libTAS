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

#include "RenderHUD_Base_Linux.h"

#include "logging.h"
#include "GlobalState.h"
#include "xlib/xwindows.h"
#include "xlib/xdisplay.h"
#include "../external/imgui/imgui.h"

#include <X11/cursorfont.h>

namespace libtas {

RenderHUD_Base_Linux::~RenderHUD_Base_Linux() {}

void RenderHUD_Base_Linux::updateCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    
    GlobalNative gn;
    
    /* If no game window yet, returns */
    if (x11::gameXWindows.empty())
        return;
    
    Display* firstDisplay = 0;
    for (int i=0; i<GAMEDISPLAYNUM; i++) {
        if (x11::gameDisplays[i]) {
            firstDisplay = x11::gameDisplays[i];
            break;
        }
    }
    
    if (cursors.empty()) {
        cursors.resize(ImGuiMouseCursor_COUNT);
        cursors[ImGuiMouseCursor_Arrow] = XCreateFontCursor(firstDisplay, XC_left_ptr);
        cursors[ImGuiMouseCursor_TextInput] = XCreateFontCursor(firstDisplay, XC_xterm);
        cursors[ImGuiMouseCursor_ResizeAll] = XCreateFontCursor(firstDisplay, XC_fleur);
        cursors[ImGuiMouseCursor_ResizeNS] = XCreateFontCursor(firstDisplay, XC_sb_v_double_arrow);
        cursors[ImGuiMouseCursor_ResizeEW] = XCreateFontCursor(firstDisplay, XC_sb_h_double_arrow);
        cursors[ImGuiMouseCursor_ResizeNESW] = XCreateFontCursor(firstDisplay, XC_fleur);
        cursors[ImGuiMouseCursor_ResizeNWSE] = XCreateFontCursor(firstDisplay, XC_fleur);
        cursors[ImGuiMouseCursor_Hand] = XCreateFontCursor(firstDisplay, XC_hand2);
        cursors[ImGuiMouseCursor_NotAllowed] = XCreateFontCursor(firstDisplay, XC_pirate);

        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    }
    
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        for (Window w : x11::gameXWindows)
            XUndefineCursor(firstDisplay, w);
    }
    else {
        // Show OS mouse cursor
        Cursor expected_cursor = cursors[imgui_cursor] ? cursors[imgui_cursor] : cursors[ImGuiMouseCursor_Arrow];
        if (last_cursor != expected_cursor) {
            for (Window w : x11::gameXWindows)
                XDefineCursor(firstDisplay, w, expected_cursor);
            last_cursor = expected_cursor;
        }
    }
    XFlush(firstDisplay);
}

}
