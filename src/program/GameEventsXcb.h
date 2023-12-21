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

#ifndef LIBTAS_GAMEEVENTS_XCB_H_INCLUDED
#define LIBTAS_GAMEEVENTS_XCB_H_INCLUDED

#include "GameEvents.h"
#include "KeyMapping.h"

#include <memory>
#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

/* Forward declaration */
class MovieFile;
struct Context;

class GameEventsXcb : public GameEvents {
public:
    GameEventsXcb(Context *c, MovieFile* m);

    void init();

    /* Register and select events from the window handle */
    void registerGameWindow(uint32_t gameWindow);

    /* Determine if we are allowed to send inputs to the game, based on which
     * window has focus and our settings. */
    bool haveFocus();

private:
    /* Keyboard layout */
    std::unique_ptr<xcb_key_symbols_t, void(*)(xcb_key_symbols_t*)> keysyms;

    xcb_keycode_t last_pressed_key;
    std::unique_ptr<xcb_generic_event_t> next_event;

    /* parent window of game window */
    xcb_window_t parent_game_window = 0;

    EventType nextEvent(struct HotKey &hk);
};

#endif
