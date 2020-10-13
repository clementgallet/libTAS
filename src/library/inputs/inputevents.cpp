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

#include "config.h"
#include "inputevents.h"
#include "inputs.h"
#include "keyboard_helper.h"
#include "xkeyboardlayout.h"
#include "../logging.h"
#include "../../shared/AllInputs.h"
#include "../../shared/SingleInput.h"
#include "../../shared/SharedConfig.h"
// #include <X11/keysym.h>
#include "../DeterministicTimer.h"
#include "sdlgamecontroller.h" // sdl_controller_events
#include "sdljoystick.h" // sdl_joystick_event
#include "sdltextinput.h" // SDL_EnableUNICODE
#include "../sdl/SDLEventQueue.h"
#include "../../external/SDL1.h"
#include "jsdev.h"
#include "evdev.h"
#include "../global.h" // game_info
#include "xinput.h"
#include "../xlib/XlibEventQueueList.h"
#include "../xcb/XcbEventQueueList.h"
#include "../xlib/xevents.h"

#include <stdlib.h>
#include <SDL2/SDL.h>
#include <linux/joystick.h>
#include <linux/input.h>

namespace libtas {

void generateKeyUpEvents(void)
{
    int i, j;

    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (i=0; i<AllInputs::MAXKEYS; i++) {
        if (!old_game_ai.keyboard[i]) {
            continue;
        }
        for (j=0; j<AllInputs::MAXKEYS; j++) {
            if (old_game_ai.keyboard[i] == game_ai.keyboard[j]) {
                /* Key was not released */
                break;
            }
        }
        if (j == AllInputs::MAXKEYS) {
            /* Key was released. Generate event */
            if (game_info.keyboard & GameInfo::SDL2) {
                SDL_Event event2;
                event2.type = SDL_KEYUP;
                event2.key.state = SDL_RELEASED;
                event2.key.windowID = 1;
                event2.key.timestamp = timestamp;
                event2.key.repeat = 0;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, old_game_ai.keyboard[i]);
                event2.key.keysym = keysym;

                sdlEventQueue.insert(&event2);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYUP with key ", event2.key.keysym.sym);
            }

            if (game_info.keyboard & GameInfo::SDL1) {
                SDL1::SDL_Event event1;
                event1.type = SDL1::SDL_KEYUP;
                event1.key.which = 0; // FIXME: I don't know what is going here
                event1.key.state = SDL_RELEASED;

                SDL1::SDL_keysym keysym;
                xkeysymToSDL1(&keysym, old_game_ai.keyboard[i]);
                event1.key.keysym = keysym;

                int isUnicodeEnabled;
                NOLOGCALL(isUnicodeEnabled = SDL_EnableUNICODE(-1));
                if (isUnicodeEnabled) {
                    /* Add an Unicode representation of the key */
                    /* SDL keycode is identical to its char number for common chars */
                    event1.key.keysym.unicode = static_cast<char>(event1.key.keysym.sym & 0xff);
                }

                sdlEventQueue.insert(&event1);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL1 event KEYUP with key ", event1.key.keysym.sym);
            }

            if ((game_info.keyboard & GameInfo::XEVENTS) && !gameXWindows.empty()) {
                XEvent event;
                event.xkey.type = KeyRelease;
                event.xkey.state = 0; // TODO: Do we have to set the key modifiers?
                event.xkey.window = gameXWindows.front();
                event.xkey.time = timestamp; // TODO: Wrong! timestamp is from X server start
                event.xkey.same_screen = 1;
                event.xkey.send_event = 0;
                event.xkey.subwindow = 0;
                event.xkey.root = rootWindow;
                NOLOGCALL(event.xkey.keycode = XKeysymToKeycode(nullptr, old_game_ai.keyboard[i]));
                for (int d=0; d<GAMEDISPLAYNUM; d++) {
                    if (gameDisplays[d]) {
                        event.xkey.root = XRootWindow(gameDisplays[d], 0);
                        xlibEventQueueList.insert(gameDisplays[d], &event);
                    }
                }

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XEvent KeyRelease with keycode ", event.xkey.keycode);
            }

            if ((game_info.keyboard & GameInfo::XCBEVENTS) && !gameXWindows.empty()) {
                xcb_key_release_event_t event;
                event.response_type = XCB_KEY_RELEASE;
                event.state = 0; // TODO: Do we have to set the key modifiers?
                event.event = gameXWindows.front();
                event.time = timestamp; // TODO: Wrong! timestamp is from X server start
                event.same_screen = 1;
                event.child = 0;
                event.root = rootWindow;
                NOLOGCALL(event.detail = XKeysymToKeycode(nullptr, old_game_ai.keyboard[i]));
                for (int c=0; c<GAMECONNECTIONNUM; c++) {
                    if (gameConnections[c]) {
                        // event.root = XRootWindow(gameConnections[c], 0);
                        xcbEventQueueList.insert(gameConnections[c], reinterpret_cast<xcb_generic_event_t*>(&event));
                    }
                }

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate xcb XCB_KEY_RELEASE with keycode ", event.detail);
            }

            if ((game_info.keyboard & GameInfo::XIEVENTS) && !gameXWindows.empty()) {
                XEvent event;
                XIDeviceEvent *dev = static_cast<XIDeviceEvent*>(calloc(1, sizeof(XIDeviceEvent)));
                event.xcookie.type = GenericEvent;
                event.xcookie.extension = xinput_opcode;
                event.xcookie.evtype = XI_KeyRelease;
                event.xcookie.data = dev;
                dev->evtype = XI_KeyRelease;
                dev->event = gameXWindows.front();
                dev->time = timestamp; // TODO: Wrong! timestamp is from X server start
                NOLOGCALL(dev->detail = XKeysymToKeycode(nullptr, old_game_ai.keyboard[i]));
                for (int d=0; d<GAMEDISPLAYNUM; d++) {
                    if (gameDisplays[d]) {
                        dev->root = XRootWindow(gameDisplays[d], 0);
                        xlibEventQueueList.insert(gameDisplays[d], &event);
                    }
                }

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent KeyRelease with keycode ", dev->detail);
            }

            if (game_info.keyboard & GameInfo::XIRAWEVENTS) {
                XEvent event;
                XIRawEvent *rev = static_cast<XIRawEvent*>(calloc(1, sizeof(XIRawEvent)));
                event.xcookie.type = GenericEvent;
                event.xcookie.extension = xinput_opcode;
                event.xcookie.evtype = XI_RawKeyRelease;
                event.xcookie.data = rev;
                rev->evtype = XI_RawKeyRelease;
                rev->time = timestamp; // TODO: Wrong! timestamp is from X server start
                NOLOGCALL(rev->detail = XKeysymToKeycode(nullptr, old_game_ai.keyboard[i]));
                xlibEventQueueList.insert(&event);

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent RawKeyRelease with keycode ", rev->detail);
            }
        }
    }
}


/* Generate pressed keyboard input events */
void generateKeyDownEvents(void)
{
    int i,j;

    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (i=0; i<AllInputs::MAXKEYS; i++) {
        if (!game_ai.keyboard[i]) {
            continue;
        }
        for (j=0; j<AllInputs::MAXKEYS; j++) {
            if (game_ai.keyboard[i] == old_game_ai.keyboard[j]) {
                /* Key was not pressed */
                break;
            }
        }
        if (j == AllInputs::MAXKEYS) {
            /* Key was pressed. Generate event */
            if (game_info.keyboard & GameInfo::SDL2) {
                SDL_Event event2;
                event2.type = SDL_KEYDOWN;
                event2.key.state = SDL_PRESSED;
                event2.key.windowID = 1;
                event2.key.timestamp = timestamp;
                event2.key.repeat = 0;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, game_ai.keyboard[i]);
                event2.key.keysym = keysym;

                sdlEventQueue.insert(&event2);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key ", event2.key.keysym.sym);

                /* Generate a text input event if active */
                SDL_bool isTextInputActive;
                NOLOGCALL(isTextInputActive = SDL_IsTextInputActive());
                if ((isTextInputActive == SDL_TRUE) && ((event2.key.keysym.sym >> 8) == 0)) {
                    event2.type = SDL_TEXTINPUT;
                    event2.text.windowID = 1;
                    event2.text.timestamp = timestamp;
                    /* SDL keycode is identical to its char number for common chars */
                    event2.text.text[0] = static_cast<char>(event2.key.keysym.sym & 0xff);
                    event2.text.text[1] = '\0';

                    sdlEventQueue.insert(&event2);

                    debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event SDL_TEXTINPUT with text ", event2.text.text);
                }
            }

            if (game_info.keyboard & GameInfo::SDL1) {
                SDL1::SDL_Event event1;
                event1.type = SDL1::SDL_KEYDOWN;
                event1.key.which = 0; // FIXME: I don't know what is going here
                event1.key.state = SDL_PRESSED;

                SDL1::SDL_keysym keysym;
                xkeysymToSDL1(&keysym, game_ai.keyboard[i]);
                event1.key.keysym = keysym;

                int isUnicodeEnabled;
                NOLOGCALL(isUnicodeEnabled = SDL_EnableUNICODE(-1));
                if (isUnicodeEnabled) {
                    /* Add an Unicode representation of the key */
                    /* SDL keycode is identical to its char number for common chars */
                    event1.key.keysym.unicode = static_cast<char>(event1.key.keysym.sym & 0xff);
                }

                sdlEventQueue.insert(&event1);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL1 event KEYDOWN with key ", event1.key.keysym.sym);
            }

            if ((game_info.keyboard & GameInfo::XEVENTS) && !gameXWindows.empty()) {
                XEvent event;
                event.xkey.type = KeyPress;
                event.xkey.state = 0; // TODO: Do we have to set the key modifiers?
                event.xkey.window = gameXWindows.front();
                event.xkey.time = timestamp;
                event.xkey.same_screen = 1;
                event.xkey.send_event = 0;
                event.xkey.subwindow = 0;
                event.xkey.root = rootWindow;
                NOLOGCALL(event.xkey.keycode = XKeysymToKeycode(nullptr, game_ai.keyboard[i]));
                for (int d=0; d<GAMEDISPLAYNUM; d++) {
                    if (gameDisplays[d]) {
                        event.xkey.root = XRootWindow(gameDisplays[d], 0);
                        xlibEventQueueList.insert(gameDisplays[d], &event);
                    }
                }

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XEvent KeyPress with keycode ", event.xkey.keycode);
            }

            if ((game_info.keyboard & GameInfo::XCBEVENTS) && !gameXWindows.empty()) {
                xcb_key_press_event_t event;
                event.response_type = XCB_KEY_PRESS;
                event.state = 0; // TODO: Do we have to set the key modifiers?
                event.event = gameXWindows.front();
                event.time = timestamp; // TODO: Wrong! timestamp is from X server start
                event.same_screen = 1;
                event.child = 0;
                event.root = rootWindow;
                NOLOGCALL(event.detail = XKeysymToKeycode(nullptr, game_ai.keyboard[i]));
                for (int c=0; c<GAMECONNECTIONNUM; c++) {
                    if (gameConnections[c]) {
                        // event.root = XRootWindow(gameConnections[c], 0);
                        xcbEventQueueList.insert(gameConnections[c], reinterpret_cast<xcb_generic_event_t*>(&event));
                    }
                }

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate xcb XCB_KEY_PRESS with keycode ", event.detail);
            }

            if ((game_info.keyboard & GameInfo::XIEVENTS) && !gameXWindows.empty()) {
                XEvent event;
                XIDeviceEvent *dev = static_cast<XIDeviceEvent*>(calloc(1, sizeof(XIDeviceEvent)));
                event.xcookie.type = GenericEvent;
                event.xcookie.extension = xinput_opcode;
                event.xcookie.evtype = XI_KeyPress;
                event.xcookie.data = dev;
                dev->evtype = XI_KeyPress;
                dev->event = gameXWindows.front();
                dev->time = timestamp;
                NOLOGCALL(dev->detail = XKeysymToKeycode(nullptr, game_ai.keyboard[i]));
                for (int d=0; d<GAMEDISPLAYNUM; d++) {
                    if (gameDisplays[d]) {
                        dev->root = XRootWindow(gameDisplays[d], 0);
                        xlibEventQueueList.insert(gameDisplays[d], &event);
                    }
                }

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent KeyPress with keycode ", dev->detail);
            }

            if (game_info.keyboard & GameInfo::XIRAWEVENTS) {
                XEvent event;
                XIRawEvent *rev = static_cast<XIRawEvent*>(calloc(1, sizeof(XIRawEvent)));
                event.xcookie.type = GenericEvent;
                event.xcookie.extension = xinput_opcode;
                event.xcookie.evtype = XI_RawKeyPress;
                event.xcookie.data = rev;
                rev->evtype = XI_RawKeyPress;
                rev->time = timestamp;
                NOLOGCALL(rev->detail = XKeysymToKeycode(nullptr, game_ai.keyboard[i]));
                xlibEventQueueList.insert(&event);

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent RawKeyPress with keycode ", rev->detail);
            }
        }
    }
}

void generateControllerAdded(void)
{
    if (!(game_info.joystick & GameInfo::SDL2))
        return;

    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    static bool init_added = false;

    if (!init_added) {
        init_added = true;
        for (int i = 0; i < shared_config.nb_controllers; i++) {
            SDL_Event ev;
            ev.type = SDL_CONTROLLERDEVICEADDED;
            ev.cdevice.timestamp = timestamp;
            ev.cdevice.which = i;
            sdlEventQueue.insert(&ev);
            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_CONTROLLERDEVICEADDED with joy ", i);

            ev.type = SDL_JOYDEVICEADDED;
            ev.jdevice.timestamp = timestamp;
            ev.jdevice.which = i;
            sdlEventQueue.insert(&ev);
            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_JOYDEVICEADDED with joy ", i);
        }
    }

    if (!game_ai.flags) return;

    int added_flags[4] = {
        SingleInput::FLAG_CONTROLLER1_ADDED,
        SingleInput::FLAG_CONTROLLER2_ADDED,
        SingleInput::FLAG_CONTROLLER3_ADDED,
        SingleInput::FLAG_CONTROLLER4_ADDED,
    };

    for (int i=0; i<4; i++) {
        if ((game_ai.flags & (1<<added_flags[i])) &&
            (shared_config.nb_controllers >= i)) {
            SDL_Event ev;
            ev.type = SDL_CONTROLLERDEVICEADDED;
            ev.cdevice.timestamp = timestamp;
            ev.cdevice.which = i;
            sdlEventQueue.insert(&ev);
            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_CONTROLLERDEVICEADDED with joy ", i);

            ev.type = SDL_JOYDEVICEADDED;
            ev.jdevice.timestamp = timestamp;
            ev.jdevice.which = i;
            sdlEventQueue.insert(&ev);
            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_JOYDEVICEADDED with joy ", i);
        }
    }

    int removed_flags[4] = {
        SingleInput::FLAG_CONTROLLER1_REMOVED,
        SingleInput::FLAG_CONTROLLER2_REMOVED,
        SingleInput::FLAG_CONTROLLER3_REMOVED,
        SingleInput::FLAG_CONTROLLER4_REMOVED,
    };

    for (int i=0; i<4; i++) {
        if ((game_ai.flags & (1<<removed_flags[i])) &&
            (shared_config.nb_controllers >= i)) {
            SDL_Event ev;
            ev.type = SDL_CONTROLLERDEVICEREMOVED;
            ev.cdevice.timestamp = timestamp;
            ev.cdevice.which = i;
            sdlEventQueue.insert(&ev);
            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_CONTROLLERDEVICEREMOVED with joy ", i);

            ev.type = SDL_JOYDEVICEREMOVED;
            ev.jdevice.timestamp = timestamp;
            ev.jdevice.which = i;
            sdlEventQueue.insert(&ev);
            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_JOYDEVICEADDED with joy ", i);

            /* Disconnect connected joystick */
            GlobalNoLog gnl;
            while (SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&i)))
                SDL_GameControllerClose(reinterpret_cast<SDL_GameController*>(&i));

            while (SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&i)))
                SDL_JoystickClose(reinterpret_cast<SDL_Joystick*>(&i));
        }
    }
}

void generateControllerEvents(void)
{
    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (int ji=0; ji<shared_config.nb_controllers; ji++) {

        /* Check if we need to generate any joystick events for that
         * particular joystick. If not, we {continue;} here because
         * we must not update the joystick state (game_ai) as specified
         * in the SDL documentation. The game must then call
         * SDL_[Joystick/GameController]Update to update the joystick state.
         */
        bool genGC = true, genJoy = true;

        if (game_info.joystick & GameInfo::SDL2) {
            GlobalNoLog gnl;
            genGC = (SDL_GameControllerEventState(SDL_QUERY) == SDL_ENABLE) && SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&ji));
            //bool genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && SDL_JoystickGetAttached(&ji);
            /* I'm not sure this is the right thing to do, but enabling joystick events when only the GC is opened */
            genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && (SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&ji)) || SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&ji)));

            if (!genGC && !genJoy)
                continue;
        }

        if (game_info.joystick & GameInfo::SDL1) {
            GlobalNoLog gnl;
            genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&ji));

            if (!genJoy)
                continue;
        }

        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            /* Check for axes change */
            if (game_ai.controller_axes[ji][axis] != old_game_ai.controller_axes[ji][axis]) {
                /* We got a change in a controller axis value */

                if (game_info.joystick & GameInfo::SDL2) {
                    if (genGC) {
                        SDL_Event event2;
                        event2.type = SDL_CONTROLLERAXISMOTION;
                        event2.caxis.timestamp = timestamp;
                        event2.caxis.which = ji;
                        event2.caxis.axis = SingleInput::toSDL2Axis(axis);
                        event2.caxis.value = game_ai.controller_axes[ji][axis];
                        sdlEventQueue.insert(&event2);
                        debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERAXISMOTION with axis ", axis);
                    }
                    if (genJoy) {
                        SDL_Event event2;
                        event2.type = SDL_JOYAXISMOTION;
                        event2.jaxis.timestamp = timestamp;
                        event2.jaxis.which = ji;
                        event2.jaxis.axis = axis;
                        event2.jaxis.value = game_ai.controller_axes[ji][axis];
                        sdlEventQueue.insert(&event2);
                        debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis ", axis);
                    }
                }

                if (game_info.joystick & GameInfo::SDL1) {
                    SDL1::SDL_Event event1;
                    event1.type = SDL1::SDL_JOYAXISMOTION;
                    event1.jaxis.which = ji;
                    event1.jaxis.axis = axis;
                    event1.jaxis.value = game_ai.controller_axes[ji][axis];
                    sdlEventQueue.insert(&event1);
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis ", axis);
                }

                if (game_info.joystick & GameInfo::JSDEV) {
                    struct js_event ev;
                    ev.time = timestamp;
                    ev.type = JS_EVENT_AXIS;
                    ev.number = SingleInput::toJsdevAxis(axis);
                    ev.value = game_ai.controller_axes[ji][axis];
                    write_jsdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis ", axis);
                }

                if (game_info.joystick & GameInfo::EVDEV) {
                    struct input_event ev;
                    ev.time.tv_sec = time.tv_sec;
                    ev.time.tv_usec = time.tv_nsec / 1000;
                    ev.type = EV_ABS;
                    ev.code = SingleInput::toEvdevAxis(axis);
                    ev.value = game_ai.controller_axes[ji][axis];
                    write_evdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis ", axis);
                }
            }
        }

        /* Check for button change */
        unsigned short buttons = game_ai.controller_buttons[ji];
        unsigned short old_buttons = old_game_ai.controller_buttons[ji];

        /* We generate the hat event separately from the buttons,
         * but we still check here if hat has changed */
        bool hatHasChanged = false;

        for (int bi=0; bi<16; bi++) {
            if (((buttons >> bi) & 0x1) != ((old_buttons >> bi) & 0x1)) {
                /* We got a change in a button state */

                if (game_info.joystick & GameInfo::SDL2) {
                    if (genGC) {
                        /* SDL2 controller button */
                        SDL_Event event2;
                        if ((buttons >> bi) & 0x1) {
                            event2.type = SDL_CONTROLLERBUTTONDOWN;
                            event2.cbutton.state = SDL_PRESSED;
                            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERBUTTONDOWN with button ", bi);
                        }
                        else {
                            event2.type = SDL_CONTROLLERBUTTONUP;
                            event2.cbutton.state = SDL_RELEASED;
                            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERBUTTONUP with button ", bi);
                        }
                        event2.cbutton.timestamp = timestamp;
                        event2.cbutton.which = ji;
                        event2.cbutton.button = SingleInput::toSDL2Button(bi);
                        sdlEventQueue.insert(&event2);
                    }

                    if (genJoy) {
                        if (bi < 11) {
                            /* SDL2 joystick button */
                            SDL_Event event2;
                            if ((buttons >> bi) & 0x1) {
                                event2.type = SDL_JOYBUTTONDOWN;
                                event2.jbutton.state = SDL_PRESSED;
                                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONDOWN with button ", bi);
                            }
                            else {
                                event2.type = SDL_JOYBUTTONUP;
                                event2.jbutton.state = SDL_RELEASED;
                                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONUP with button ", bi);
                            }
                            event2.jbutton.timestamp = timestamp;
                            event2.jbutton.which = ji;
                            event2.jbutton.button = bi;
                            sdlEventQueue.insert(&event2);
                        }

                        else {
                            hatHasChanged = true;
                        }
                    }
                }

                if (game_info.joystick & GameInfo::SDL1) {
                    if (bi < 11) {
                        /* SDL1 joystick button */
                        SDL1::SDL_Event event1;
                        if ((buttons >> bi) & 0x1) {
                            event1.type = SDL1::SDL_JOYBUTTONDOWN;
                            event1.jbutton.state = SDL_PRESSED;
                            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONDOWN with button ", bi);
                        }
                        else {
                            event1.type = SDL1::SDL_JOYBUTTONUP;
                            event1.jbutton.state = SDL_RELEASED;
                            debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONUP with button ", bi);
                        }
                        event1.jbutton.which = ji;
                        event1.jbutton.button = bi;
                        sdlEventQueue.insert(&event1);
                    }
                    else {
                        hatHasChanged = true;
                    }
                }

                if (game_info.joystick & GameInfo::JSDEV) {
                    if (bi < 11) { // JSDEV joystick only has 11 buttons
                        struct js_event ev;
                        ev.time = timestamp;
                        ev.type = JS_EVENT_BUTTON;
                        ev.number = SingleInput::toJsdevButton(bi);
                        ev.value = (buttons >> bi) & 0x1;
                        debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_BUTTON with button ", bi);
                        write_jsdev(ev, ji);
                    }
                    else {
                        hatHasChanged = true;
                    }
                }

                if (game_info.joystick & GameInfo::EVDEV) {
                    if (bi < 11) { // EVDEV joystick only has 11 buttons
                        struct input_event ev;
                        ev.time.tv_sec = time.tv_sec;
                        ev.time.tv_usec = time.tv_nsec / 1000;
                        ev.type = EV_KEY;
                        ev.code = SingleInput::toEvdevButton(bi);
                        ev.value = (buttons >> bi) & 0x1;
                        debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_KEY with button ", bi);
                        write_evdev(ev, ji);
                    }
                    else {
                        hatHasChanged = true;
                    }
                }
            }
        }

        /* Generate hat state */
        if (hatHasChanged) {

            if (game_info.joystick & GameInfo::SDL2) {
                /* SDL2 joystick hat */
                SDL_Event event2;
                event2.type = SDL_JOYHATMOTION;
                event2.jhat.timestamp = timestamp;
                event2.jhat.which = ji;
                event2.jhat.hat = 0;
                event2.jhat.value = SingleInput::toSDLHat(buttons);
                sdlEventQueue.insert(&event2);
                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYHATMOTION with hat ", (int)event2.jhat.value);
            }

            if (game_info.joystick & GameInfo::SDL1) {
                /* SDL1 joystick hat */
                SDL1::SDL_Event event1;
                event1.type = SDL1::SDL_JOYHATMOTION;
                event1.jhat.which = ji;
                event1.jhat.hat = 0;
                event1.jhat.value = SingleInput::toSDLHat(buttons);
                sdlEventQueue.insert(&event1);
                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYHATMOTION with hat ", (int)event1.jhat.value);
            }

            if (game_info.joystick & GameInfo::JSDEV) {
                /* Hat status is represented as 7th and 8th axes */

                int hatx = SingleInput::toDevHatX(buttons);
                int oldhatx = SingleInput::toDevHatX(old_buttons);
                if (hatx != oldhatx) {
                    struct js_event ev;
                    ev.time = timestamp;
                    ev.type = JS_EVENT_AXIS;
                    ev.number = 6;
                    ev.value = hatx;
                    write_jsdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis 6");
                }

                int haty = SingleInput::toDevHatY(buttons);
                int oldhaty = SingleInput::toDevHatY(old_buttons);
                if (haty != oldhaty) {
                    struct js_event ev;
                    ev.time = timestamp;
                    ev.type = JS_EVENT_AXIS;
                    ev.number = 7;
                    ev.value = haty;
                    write_jsdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis 7");
                }
            }

            if (game_info.joystick & GameInfo::EVDEV) {
                int hatx = SingleInput::toDevHatX(buttons);
                int oldhatx = SingleInput::toDevHatX(old_buttons);
                if (hatx != oldhatx) {
                    struct input_event ev;
                    ev.time.tv_sec = time.tv_sec;
                    ev.time.tv_usec = time.tv_nsec / 1000;
                    ev.type = EV_ABS;
                    ev.code = ABS_HAT0X;
                    ev.value = hatx;
                    write_evdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis ", ABS_HAT0X);
                }

                int haty = SingleInput::toDevHatY(buttons);
                int oldhaty = SingleInput::toDevHatY(old_buttons);
                if (haty != oldhaty) {
                    struct input_event ev;
                    ev.time.tv_sec = time.tv_sec;
                    ev.time.tv_usec = time.tv_nsec / 1000;
                    ev.type = EV_ABS;
                    ev.code = ABS_HAT0Y;
                    ev.value = haty;
                    write_evdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis ", ABS_HAT0Y);
                }
            }
        }
    }
}

void generateMouseMotionEvents(void)
{
    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    /* XIRAWEVENTS are special because they output raw pointer events */
    if ((game_info.mouse & GameInfo::XIRAWEVENTS) &&
        ((game_unclipped_ai.pointer_x != old_game_unclipped_ai.pointer_x) || (game_unclipped_ai.pointer_y != old_game_unclipped_ai.pointer_y))) {
        XEvent event;
        XIRawEvent *rev = static_cast<XIRawEvent*>(calloc(1, sizeof(XIRawEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        event.xcookie.evtype = XI_RawMotion;
        event.xcookie.data = rev;
        rev->evtype = XI_RawMotion;
        rev->time = timestamp;
        rev->raw_values = static_cast<double*>(malloc(2*sizeof(double)));
        rev->raw_values[0] = game_unclipped_ai.pointer_x - old_game_unclipped_ai.pointer_x;
        rev->raw_values[1] = game_unclipped_ai.pointer_y - old_game_unclipped_ai.pointer_y;
        rev->valuators.values = static_cast<double*>(malloc(2*sizeof(double)));
        rev->valuators.values[0] = game_unclipped_ai.pointer_x - old_game_unclipped_ai.pointer_x;
        rev->valuators.values[1] = game_unclipped_ai.pointer_y - old_game_unclipped_ai.pointer_y;
        rev->valuators.mask = static_cast<unsigned char*>(malloc(1*sizeof(unsigned char)));
        rev->valuators.mask[0] = 0;
        XISetMask(rev->valuators.mask, 0);
        XISetMask(rev->valuators.mask, 1);
        rev->valuators.mask_len = 1;
        xlibEventQueueList.insert(&event);

        debuglog(LCF_EVENTS | LCF_MOUSE, "Generate XIEvent XI_RawMotion");
    }

    /* Check if we got a change in mouse position */
    if ((game_ai.pointer_x == old_game_ai.pointer_x) && (game_ai.pointer_y == old_game_ai.pointer_y))
        return;

    if (game_info.mouse & GameInfo::SDL2) {
        SDL_Event event2;
        event2.type = SDL_MOUSEMOTION;
        event2.motion.timestamp = timestamp;
        event2.motion.windowID = 1;
        event2.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event2.motion.state = SingleInput::toSDL2PointerMask(game_ai.pointer_mask);

        /* Relative movement is not subject to window clipping */
        event2.motion.xrel = game_unclipped_ai.pointer_x - old_game_unclipped_ai.pointer_x;
        event2.motion.yrel = game_unclipped_ai.pointer_y - old_game_unclipped_ai.pointer_y;
        event2.motion.x = game_ai.pointer_x;
        event2.motion.y = game_ai.pointer_y;
        sdlEventQueue.insert(&event2);
        debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEMOTION with new position (", game_ai.pointer_x, ",", game_ai.pointer_y,")");
    }

    if (game_info.mouse & GameInfo::SDL1) {
        SDL1::SDL_Event event1;
        event1.type = SDL1::SDL_MOUSEMOTION;
        event1.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event1.motion.state = SingleInput::toSDL1PointerMask(game_ai.pointer_mask);

        /* Relative movement is not subject to window clipping */
        event1.motion.xrel = (Sint16)(game_unclipped_ai.pointer_x - old_game_unclipped_ai.pointer_x);
        event1.motion.yrel = (Sint16)(game_unclipped_ai.pointer_y - old_game_unclipped_ai.pointer_y);
        event1.motion.x = (Uint16) game_ai.pointer_x;
        event1.motion.y = (Uint16) game_ai.pointer_y;
        sdlEventQueue.insert(&event1);
        debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEMOTION with new position (", game_ai.pointer_x, ",", game_ai.pointer_y,")");
    }

    if ((game_info.mouse & GameInfo::XEVENTS) && !gameXWindows.empty()) {
        XEvent event;
        event.xmotion.type = MotionNotify;
        event.xmotion.state = SingleInput::toXlibPointerMask(game_ai.pointer_mask);
        event.xmotion.x = game_ai.pointer_x;
        event.xmotion.y = game_ai.pointer_y;
        event.xmotion.x_root = event.xmotion.x;
        event.xmotion.y_root = event.xmotion.y;
        if (pointer_grab_window != None)
            event.xmotion.window = pointer_grab_window;
        else
            event.xmotion.window = gameXWindows.front();
        event.xmotion.send_event = 0;
        event.xmotion.subwindow = 0;
        event.xmotion.root = rootWindow;
        event.xmotion.same_screen = 0;
        event.xmotion.time = timestamp;

        xlibEventQueueList.insert(&event);
        debuglog(LCF_EVENTS | LCF_MOUSE, "Generate Xlib event MotionNotify with new position (", game_ai.pointer_x, ",", game_ai.pointer_y,")");
    }

    if ((game_info.mouse & GameInfo::XCBEVENTS) && !gameXWindows.empty()) {
        xcb_motion_notify_event_t event;
        event.response_type = XCB_MOTION_NOTIFY;
        event.state = SingleInput::toXlibPointerMask(game_ai.pointer_mask);
        event.event_x = game_ai.pointer_x;
        event.event_y = game_ai.pointer_y;
        event.root_x = game_ai.pointer_x;
        event.root_y = game_ai.pointer_y;
        event.event = gameXWindows.front();
        event.time = timestamp;
        event.same_screen = 1;
        event.child = 0;
        event.root = rootWindow;

        xcbEventQueueList.insert(reinterpret_cast<xcb_generic_event_t*>(&event));
        debuglog(LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_MOTION_NOTIFY with new position (", game_ai.pointer_x, ",", game_ai.pointer_y,")");
    }

    if ((game_info.mouse & GameInfo::XIEVENTS) && !gameXWindows.empty()) {
        XEvent event;
        XIDeviceEvent *dev = static_cast<XIDeviceEvent*>(calloc(1, sizeof(XIDeviceEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        event.xcookie.evtype = XI_Motion;
        event.xcookie.data = dev;
        dev->evtype = XI_Motion;
        dev->event = gameXWindows.front();
        dev->time = timestamp;
        dev->event_x = game_ai.pointer_x;
        dev->event_y = game_ai.pointer_y;
        dev->root_x = dev->event_x;
        dev->root_y = dev->event_y;
        dev->detail = 0;
        for (int d=0; d<GAMEDISPLAYNUM; d++) {
            if (gameDisplays[d]) {
                dev->root = XRootWindow(gameDisplays[d], 0);
                xlibEventQueueList.insert(gameDisplays[d], &event);
            }
        }

        debuglog(LCF_EVENTS | LCF_MOUSE, "Generate XIEvent XI_Motion");
    }
}

void generateMouseButtonEvents(void)
{
    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    static int buttons[] = {SingleInput::POINTER_B1,
        SingleInput::POINTER_B2, SingleInput::POINTER_B3,
        SingleInput::POINTER_B4, SingleInput::POINTER_B5};

    for (int bi=0; bi<5; bi++) {
        if ((game_ai.pointer_mask ^ old_game_ai.pointer_mask) & (1 << buttons[bi])) {
            /* We got a change in a button state */

            /* Fill the event structure */
            if (game_info.mouse & GameInfo::SDL2) {
                SDL_Event event2;
                if (game_ai.pointer_mask & (1 << buttons[bi])) {
                    event2.type = SDL_MOUSEBUTTONDOWN;
                    event2.button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONDOWN with button ", SingleInput::toSDL2PointerButton(buttons[bi]));
                }
                else {
                    event2.type = SDL_MOUSEBUTTONUP;
                    event2.button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONUP with button ", SingleInput::toSDL2PointerButton(buttons[bi]));
                }
                event2.button.timestamp = timestamp;
                event2.button.windowID = 1;
                event2.button.which = 0; // TODO: Same as above...
                event2.button.button = SingleInput::toSDL2PointerButton(buttons[bi]);
                event2.button.clicks = 1;
                event2.button.x = game_ai.pointer_x;
                event2.button.y = game_ai.pointer_y;
                sdlEventQueue.insert(&event2);
            }

            if (game_info.mouse & GameInfo::SDL1) {
                SDL1::SDL_Event event1;
                if (game_ai.pointer_mask & (1 << buttons[bi])) {
                    event1.type = SDL1::SDL_MOUSEBUTTONDOWN;
                    event1.button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONDOWN with button ", SingleInput::toSDL1PointerButton(buttons[bi]));
                }
                else {
                    event1.type = SDL1::SDL_MOUSEBUTTONUP;
                    event1.button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONUP with button ", SingleInput::toSDL1PointerButton(buttons[bi]));
                }
                event1.button.which = 0; // TODO: Same as above...
                event1.button.button = SingleInput::toSDL1PointerButton(buttons[bi]);
                event1.button.x = (Uint16) game_ai.pointer_x;
                event1.button.y = (Uint16) game_ai.pointer_y;
                sdlEventQueue.insert(&event1);
            }

            if ((game_info.mouse & GameInfo::XEVENTS) && !gameXWindows.empty()) {
                XEvent event;
                if (game_ai.pointer_mask & (1 << buttons[bi])) {
                    event.xbutton.type = ButtonPress;
                    debuglog(LCF_EVENTS | LCF_MOUSE, "Generate Xlib event ButtonPress with button ", SingleInput::toXlibPointerButton(buttons[bi]));
                }
                else {
                    event.xbutton.type = ButtonRelease;
                    debuglog(LCF_EVENTS | LCF_MOUSE, "Generate Xlib event ButtonRelease with button ", SingleInput::toXlibPointerButton(buttons[bi]));
                }
                event.xbutton.state = SingleInput::toXlibPointerMask(game_ai.pointer_mask);
                event.xbutton.x = game_ai.pointer_x;
                event.xbutton.y = game_ai.pointer_y;
                event.xbutton.x_root = event.xbutton.x;
                event.xbutton.y_root = event.xbutton.y;
                event.xbutton.button = SingleInput::toXlibPointerButton(buttons[bi]);
                if (pointer_grab_window != None)
                    event.xbutton.window = pointer_grab_window;
                else
                    event.xbutton.window = gameXWindows.front();
                event.xbutton.same_screen = 1;
                event.xbutton.send_event = 0;
                event.xbutton.subwindow = 0;
                event.xbutton.root = rootWindow;

                xlibEventQueueList.insert(&event);
            }

            if ((game_info.mouse & GameInfo::XCBEVENTS) && !gameXWindows.empty()) {
                xcb_button_press_event_t event; // same as xcb_button_release_event_t
                if (game_ai.pointer_mask & (1 << buttons[bi])) {
                    event.response_type = XCB_BUTTON_PRESS;
                    debuglog(LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_BUTTON_PRESS with button ", SingleInput::toXlibPointerButton(buttons[bi]));
                }
                else {
                    event.response_type = XCB_BUTTON_RELEASE;
                    debuglog(LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_BUTTON_RELEASE with button ", SingleInput::toXlibPointerButton(buttons[bi]));
                }
                event.state = SingleInput::toXlibPointerMask(game_ai.pointer_mask);
                event.event_x = game_ai.pointer_x;
                event.event_y = game_ai.pointer_y;
                event.root_x = game_ai.pointer_x;
                event.root_y = game_ai.pointer_y;
                event.detail = SingleInput::toXlibPointerButton(buttons[bi]);
                event.event = gameXWindows.front();
                event.same_screen = 1;
                event.child = 0;
                event.root = rootWindow;

                xcbEventQueueList.insert(reinterpret_cast<xcb_generic_event_t*>(&event));
            }

            if ((game_info.mouse & GameInfo::XIEVENTS) && !gameXWindows.empty()) {
                XEvent event;
                XIDeviceEvent *dev = static_cast<XIDeviceEvent*>(calloc(1, sizeof(XIDeviceEvent)));
                event.xcookie.type = GenericEvent;
                event.xcookie.extension = xinput_opcode;
                if (game_ai.pointer_mask & (1 << buttons[bi])) {
                    debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_ButtonPress with button ", bi+1);
                    event.xcookie.evtype = XI_ButtonPress;
                    dev->evtype = XI_ButtonPress;
                }
                else {
                    debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_ButtonRelease with button ", bi+1);
                    event.xcookie.evtype = XI_ButtonRelease;
                    dev->evtype = XI_ButtonRelease;
                }
                event.xcookie.data = dev;
                dev->event = gameXWindows.front();
                dev->time = timestamp;
                dev->event_x = game_ai.pointer_x;
                dev->event_y = game_ai.pointer_y;
                dev->root_x = dev->event_x;
                dev->root_y = dev->event_y;
                dev->detail = bi+1;
                dev->buttons.mask = static_cast<unsigned char*>(malloc(1*sizeof(unsigned char)));
                dev->buttons.mask_len = 1;
                for (int bj=0; bj<5; bj++) {
                    if (game_ai.pointer_mask & (1 << buttons[bj])) {
                        XISetMask(dev->buttons.mask, bj);
                    }
                }
                for (int d=0; d<GAMEDISPLAYNUM; d++) {
                    if (gameDisplays[d]) {
                        dev->root = XRootWindow(gameDisplays[d], 0);
                        xlibEventQueueList.insert(gameDisplays[d], &event);
                    }
                }
            }

            if (game_info.mouse & GameInfo::XIRAWEVENTS) {
                XEvent event;
                XIRawEvent *rev = static_cast<XIRawEvent*>(calloc(1, sizeof(XIRawEvent)));
                event.xcookie.type = GenericEvent;
                event.xcookie.extension = xinput_opcode;
                if (game_ai.pointer_mask & (1 << buttons[bi])) {
                    debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_RawButtonPress with button ", bi+1);
                    event.xcookie.evtype = XI_RawButtonPress;
                    rev->evtype = XI_RawButtonPress;
                }
                else {
                    debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_RawButtonRelease with button ", bi+1);
                    event.xcookie.evtype = XI_RawButtonRelease;
                    rev->evtype = XI_RawButtonRelease;
                }
                event.xcookie.data = rev;
                rev->time = timestamp;
                rev->detail = bi+1;
                xlibEventQueueList.insert(&event);
            }
        }
    }
}

void syncControllerEvents()
{
    if (!(shared_config.async_events & (SharedConfig::ASYNC_JSDEV | SharedConfig::ASYNC_EVDEV)))
        return;

    if (!(game_info.joystick & (GameInfo::JSDEV | GameInfo::EVDEV)))
        return;

    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (int i = 0; i < shared_config.nb_controllers; i++) {
        if (shared_config.async_events & SharedConfig::ASYNC_JSDEV) {
            /* Send a synchronize report event */
            struct js_event ev;
            ev.time = timestamp;
            ev.type = 0;
            ev.number = 0;
            ev.value = 0;
            write_jsdev(ev, i);

            /* Wait for queue to become empty, ensuring that
             * the event have finished being processed. */
            sync_jsdev(i);
        }

        /* Same for evdev */
        if (shared_config.async_events & SharedConfig::ASYNC_EVDEV) {
            struct input_event ev;
            ev.time.tv_sec = time.tv_sec;
            ev.time.tv_usec = time.tv_nsec / 1000;
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            write_evdev(ev, i);

            sync_evdev(i);
        }
    }
}

}
