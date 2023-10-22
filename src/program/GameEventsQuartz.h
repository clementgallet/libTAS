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

#ifndef LIBTAS_GAMEEVENTS_QUARTZ_H_INCLUDED
#define LIBTAS_GAMEEVENTS_QUARTZ_H_INCLUDED

#include <memory>
#include <stdint.h>

#include "GameEvents.h"
#include "KeyMapping.h"

#import <AppKit/AppKit.h>
#include <CoreGraphics/CoreGraphics.h>

/* Forward declaration */
class MovieFile;
struct Context;

class GameEventsQuartz : public GameEvents {
public:
    GameEventsQuartz(Context *c, MovieFile* m);

    void init();

    /* Register and select events from the window handle */
    void registerGameWindow(uint32_t gameWindow);

    /* Determine if we are allowed to send inputs to the game, based on which
     * window has focus and our settings. */
    bool haveFocus();

    /* We need this function to be static, so it can be called from the static callback. */
    static bool haveFocus(Context *c);

private:
    /* Game running app */
    static NSRunningApplication* gameApp;
    
    /* Event tap */
    CFMachPortRef eventTap;
    
    EventType nextEvent(struct HotKey &hk);
};

#endif
