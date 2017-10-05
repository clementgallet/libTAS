/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "inputevents.h"
#include "inputs.h"
#include "keyboard_helper.h"
#include "../logging.h"
#include "../../shared/AllInputs.h"
#include "../../shared/SharedConfig.h"
#include <X11/keysym.h>
#include <stdlib.h>
#include "../DeterministicTimer.h"
#include "../sdlwindows.h" // for gameWindow
#include "sdlgamecontroller.h" // sdl_controller_events
#include "sdljoystick.h" // sdl_joystick_event
#include "sdlpointer.h" // MASK constants
#include "../EventQueue.h"
#include <SDL2/SDL.h>
#include "../../external/SDL1.h"
#include <linux/joystick.h>
#include <linux/input.h>
#include "jsdev.h"
#include "evdev.h"
#include "../global.h" // game_info
#include "../hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(SDL_GetWindowID);

void generateKeyUpEvents(void)
{
    int i, j;

    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (i=0; i<AllInputs::MAXKEYS; i++) {
        if (old_ai.keyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<AllInputs::MAXKEYS; j++) {
            if (old_ai.keyboard[i] == ai.keyboard[j]) {
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
                LINK_NAMESPACE_SDL2(SDL_GetWindowID);
                event2.key.windowID = orig::SDL_GetWindowID(gameWindow);
                event2.key.timestamp = timestamp;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, old_ai.keyboard[i]);
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
                xkeysymToSDL1(&keysym, old_ai.keyboard[i]);
                event1.key.keysym = keysym;

                sdlEventQueue.insert(&event1);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL1 event KEYUP with key ", event1.key.keysym.sym);
            }

            if (game_info.keyboard & GameInfo::XEVENTS) {
                XEvent event;
                event.xkey.type = KeyRelease;
                event.xkey.state = 0; // TODO: Do we have to set the key modifiers?
                event.xkey.keycode = XKeysymToKeycode(gameDisplay, old_ai.keyboard[i]);
                event.xkey.window = gameXWindow;
                event.xkey.time = timestamp;
                XSendEvent(gameDisplay, gameXWindow, False, 0, &event);

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XEvent KeyRelease with keycode ", event.xkey.keycode);
            }

            /* Update old keyboard state */
            old_ai.keyboard[i] = XK_VoidSymbol;
        }
    }
}


/* Generate pressed keyboard input events */
void generateKeyDownEvents(void)
{
    int i,j,k;

    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (i=0; i<AllInputs::MAXKEYS; i++) {
        game_ai.keyboard[i] = ai.keyboard[i];

        if (ai.keyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<AllInputs::MAXKEYS; j++) {
            if (ai.keyboard[i] == old_ai.keyboard[j]) {
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
                LINK_NAMESPACE_SDL2(SDL_GetWindowID);
                event2.key.windowID = orig::SDL_GetWindowID(gameWindow);
                event2.key.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, ai.keyboard[i]);
                event2.key.keysym = keysym;

                sdlEventQueue.insert(&event2);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key ", event2.key.keysym.sym);
            }

            if (game_info.keyboard & GameInfo::SDL1) {
                SDL1::SDL_Event event1;
                event1.type = SDL1::SDL_KEYDOWN;
                event1.key.which = 0; // FIXME: I don't know what is going here
                event1.key.state = SDL_PRESSED;

                SDL1::SDL_keysym keysym;
                xkeysymToSDL1(&keysym, ai.keyboard[i]);
                event1.key.keysym = keysym;

                sdlEventQueue.insert(&event1);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL1 event KEYDOWN with key ", event1.key.keysym.sym);
            }

            if (game_info.keyboard & GameInfo::XEVENTS) {
                XEvent event;
                event.xkey.type = KeyPress;
                event.xkey.state = 0; // TODO: Do we have to set the key modifiers?
                event.xkey.keycode = XKeysymToKeycode(gameDisplay, ai.keyboard[i]);
                event.xkey.window = gameXWindow;
                event.xkey.time = timestamp;
                XSendEvent(gameDisplay, gameXWindow, False, 0, &event);

                debuglog(LCF_EVENTS | LCF_KEYBOARD, "Generate XEvent KeyPress with keycode ", event.xkey.keycode);
            }

            /* Update old keyboard state */
            for (k=0; k<AllInputs::MAXKEYS; k++)
                if (old_ai.keyboard[k] == XK_VoidSymbol) {
                    /* We found an empty space to put our key*/
                    old_ai.keyboard[k] = ai.keyboard[i];
                    break;
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

    for (int i = 0; i < shared_config.nb_controllers; i++) {
        SDL_Event ev;
        ev.type = SDL_CONTROLLERDEVICEADDED;
        ev.cdevice.timestamp = timestamp;
        ev.cdevice.which = i;
        sdlEventQueue.insert(&ev);

        ev.type = SDL_JOYDEVICEADDED;
        ev.jdevice.timestamp = timestamp;
        ev.jdevice.which = i;
        sdlEventQueue.insert(&ev);
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
        bool genGC, genJoy = true;

        if (game_info.keyboard & GameInfo::SDL2) {
            GlobalOwnCode toc;
            genGC = (SDL_GameControllerEventState(SDL_QUERY) == SDL_ENABLE) && SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&ji));
            //bool genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && SDL_JoystickGetAttached(&ji);
            /* I'm not sure this is the right thing to do, but enabling joystick events when only the GC is opened */
            genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && (SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&ji)) || SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&ji)));

            if (!genGC && !genJoy)
                continue;
        }

        if (game_info.keyboard & GameInfo::SDL1) {
            GlobalOwnCode toc;
            genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&ji));

            if (!genJoy)
                continue;
        }

        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            /* Update game_ai axes */
            game_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];

            /* Check for axes change */
            if (ai.controller_axes[ji][axis] != old_ai.controller_axes[ji][axis]) {
                /* We got a change in a controller axis value */

                if (game_info.joystick & GameInfo::SDL2) {
                    if (genGC) {
                        SDL_Event event2;
                        event2.type = SDL_CONTROLLERAXISMOTION;
                        event2.caxis.timestamp = timestamp;
                        event2.caxis.which = ji;
                        event2.caxis.axis = AllInputs::toSDL2Axis(axis);
                        event2.caxis.value = ai.controller_axes[ji][axis];
                        sdlEventQueue.insert(&event2);
                        debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERAXISMOTION with axis ", axis);
                    }
                    if (genJoy) {
                        SDL_Event event2;
                        event2.type = SDL_JOYAXISMOTION;
                        event2.jaxis.timestamp = timestamp;
                        event2.jaxis.which = ji;
                        event2.jaxis.axis = axis;
                        event2.jaxis.value = ai.controller_axes[ji][axis];
                        sdlEventQueue.insert(&event2);
                        debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis ", axis);
                    }
                }

                if (game_info.joystick & GameInfo::SDL1) {
                    SDL1::SDL_Event event1;
                    event1.type = SDL1::SDL_JOYAXISMOTION;
                    event1.jaxis.which = ji;
                    event1.jaxis.axis = axis;
                    event1.jaxis.value = ai.controller_axes[ji][axis];
                    sdlEventQueue.insert(&event1);
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis ", axis);
                }

                if (game_info.joystick & GameInfo::JSDEV) {
                    struct js_event ev;
                    ev.time = timestamp;
                    ev.type = JS_EVENT_AXIS;
                    ev.number = AllInputs::toJsdevAxis(axis);
                    ev.value = ai.controller_axes[ji][axis];
                    write_jsdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis ", axis);
                }

                if (game_info.joystick & GameInfo::EVDEV) {
                    struct input_event ev;
                    ev.time.tv_sec = time.tv_sec;
                    ev.time.tv_usec = time.tv_nsec / 1000;
                    ev.type = EV_ABS;
                    ev.code = AllInputs::toEvdevAxis(axis);
                    ev.value = ai.controller_axes[ji][axis];
                    write_evdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis ", axis);
                }

                /* Upload the old AllInput struct */
                old_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];
            }
        }

        /* Update game_ai buttons */
        game_ai.controller_buttons[ji] = ai.controller_buttons[ji];

        /* Check for button change */
        unsigned short buttons = ai.controller_buttons[ji];
        unsigned short old_buttons = old_ai.controller_buttons[ji];

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
                        event2.cbutton.button = AllInputs::toSDL2Button(bi);
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
                        ev.number = AllInputs::toJsdevButton(bi);
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
                        ev.code = AllInputs::toEvdevButton(bi);
                        ev.value = (buttons >> bi) & 0x1;
                        debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_KEY with button ", bi);
                        write_evdev(ev, ji);
                    }
                    else {
                        hatHasChanged = true;
                    }
                }

                /* Upload the old AllInput struct */
                old_ai.controller_buttons[ji] ^= (1 << bi);
            }
        }

        /* Generate hat state */
        if (hatHasChanged) {

            /* Fortunately, we use the fact that SDL_HAT_X constants
             * are the same in SDL 1 and SDL 2
             */
            Uint8 hatState = SDL_HAT_CENTERED;
            if (buttons & (1 << AllInputs::BUTTON_DPAD_UP))
                hatState |= SDL_HAT_UP;
            if (buttons & (1 << AllInputs::BUTTON_DPAD_DOWN))
                hatState |= SDL_HAT_DOWN;
            if (buttons & (1 << AllInputs::BUTTON_DPAD_LEFT))
                hatState |= SDL_HAT_LEFT;
            if (buttons & (1 << AllInputs::BUTTON_DPAD_RIGHT))
                hatState |= SDL_HAT_RIGHT;

            if (game_info.joystick & GameInfo::SDL2) {
                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYHATMOTION with hat ", (int)hatState);
                /* SDL2 joystick hat */
                SDL_Event event2;
                event2.type = SDL_JOYHATMOTION;
                event2.jhat.timestamp = timestamp;
                event2.jhat.which = ji;
                event2.jhat.hat = 0;
                event2.jhat.value = hatState;
                sdlEventQueue.insert(&event2);
            }

            if (game_info.joystick & GameInfo::SDL1) {
                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYHATMOTION with hat ", (int)hatState);
                /* SDL1 joystick hat */
                SDL1::SDL_Event event1;
                event1.type = SDL1::SDL_JOYHATMOTION;
                event1.jhat.which = ji;
                event1.jhat.hat = 0;
                event1.jhat.value = hatState;
                sdlEventQueue.insert(&event1);
            }

            if (game_info.joystick & GameInfo::JSDEV) {
                /* Hat status is represented as 7th and 8th axes */

                int hatx = 0;
                if (buttons & (1 << AllInputs::BUTTON_DPAD_LEFT))
                    hatx = -1;
                else if (buttons & (1 << AllInputs::BUTTON_DPAD_RIGHT))
                    hatx = 1;
                if (hatx != 0) {
                    struct js_event ev;
                    ev.time = timestamp;
                    ev.type = JS_EVENT_AXIS;
                    ev.number = 6;
                    ev.value = hatx;
                    write_jsdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis 6");
                }

                int haty = 0;
                if (buttons & (1 << AllInputs::BUTTON_DPAD_UP))
                    haty = 1;
                else if (buttons & (1 << AllInputs::BUTTON_DPAD_DOWN))
                    haty = -1;

                if (haty != 0) {
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
                /* Hat status is represented as 7th and 8th axes */

                int hatx = 0;
                if (buttons & (1 << AllInputs::BUTTON_DPAD_LEFT))
                    hatx = -1;
                else if (buttons & (1 << AllInputs::BUTTON_DPAD_RIGHT))
                    hatx = 1;
                if (hatx != 0) {
                    struct input_event ev;
                    ev.time.tv_sec = time.tv_sec;
                    ev.time.tv_usec = time.tv_nsec / 1000;
                    ev.type = EV_ABS;
                    ev.code = ABS_HAT0X;
                    ev.value = hatx;
                    write_evdev(ev, ji);
                    debuglog(LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis ", ABS_HAT0X);
                }

                int haty = 0;
                if (buttons & (1 << AllInputs::BUTTON_DPAD_UP))
                    haty = 1;
                else if (buttons & (1 << AllInputs::BUTTON_DPAD_DOWN))
                    haty = -1;

                if (haty != 0) {
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
    if ((ai.pointer_x == old_ai.pointer_x) && (ai.pointer_y == old_ai.pointer_y))
        return;

    /* Check if cursor is out of window. Don't return an event for now */
    if (ai.pointer_x < 0)
        return;

    /* We got a change in mouse position */

    /* Fill the event structure */
    /* TODO: Deal if pointer is out of screen */

    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    if (game_info.mouse & GameInfo::SDL2) {
        SDL_Event event2;
        event2.type = SDL_MOUSEMOTION;
        event2.motion.timestamp = timestamp;
        LINK_NAMESPACE_SDL2(SDL_GetWindowID);
        event2.motion.windowID = orig::SDL_GetWindowID(gameWindow);
        event2.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event2.motion.state = 0;
        if (ai.pointer_mask & Button1Mask)
            event2.motion.state |= SDL_BUTTON_LMASK;
        if (ai.pointer_mask & Button2Mask)
            event2.motion.state |= SDL_BUTTON_MMASK;
        if (ai.pointer_mask & Button3Mask)
            event2.motion.state |= SDL_BUTTON_RMASK;
        if (ai.pointer_mask & Button4Mask)
            event2.motion.state |= SDL_BUTTON_X1MASK;
        if (ai.pointer_mask & Button5Mask)
            event2.motion.state |= SDL_BUTTON_X2MASK;

        event2.motion.xrel = ai.pointer_x - old_ai.pointer_x;
        event2.motion.yrel = ai.pointer_y - old_ai.pointer_y;
        event2.motion.x = game_ai.pointer_x + event2.motion.xrel;
        event2.motion.y = game_ai.pointer_y + event2.motion.yrel;
        sdlEventQueue.insert(&event2);
        debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEMOTION with new position (", ai.pointer_x, ",", ai.pointer_y,")");
    }

    if (game_info.mouse & GameInfo::SDL1) {
        SDL1::SDL_Event event1;
        event1.type = SDL1::SDL_MOUSEMOTION;
        event1.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event1.motion.state = 0;
        if (ai.pointer_mask & Button1Mask)
            event1.motion.state |= SDL1::SDL1_BUTTON_LMASK;
        if (ai.pointer_mask & Button2Mask)
            event1.motion.state |= SDL1::SDL1_BUTTON_MMASK;
        if (ai.pointer_mask & Button3Mask)
            event1.motion.state |= SDL1::SDL1_BUTTON_RMASK;
        if (ai.pointer_mask & Button4Mask)
            event1.motion.state |= SDL1::SDL1_BUTTON_X1MASK;
        if (ai.pointer_mask & Button5Mask)
            event1.motion.state |= SDL1::SDL1_BUTTON_X2MASK;

        event1.motion.xrel = (Sint16)(ai.pointer_x - old_ai.pointer_x);
        event1.motion.yrel = (Sint16)(ai.pointer_y - old_ai.pointer_y);
        event1.motion.x = (Uint16) (game_ai.pointer_x + event1.motion.xrel);
        event1.motion.y = (Uint16) (game_ai.pointer_y + event1.motion.yrel);
        sdlEventQueue.insert(&event1);
        debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEMOTION with new position (", ai.pointer_x, ",", ai.pointer_y,")");
    }

    if (game_info.mouse & GameInfo::XEVENTS) {
        XEvent event;
        event.xmotion.type = MotionNotify;
        event.xmotion.state = ai.pointer_mask;
        event.xmotion.x = game_ai.pointer_x + ai.pointer_x - old_ai.pointer_x;
        event.xmotion.y = game_ai.pointer_y + ai.pointer_y - old_ai.pointer_y;
        event.xmotion.x_root = event.xmotion.x;
        event.xmotion.y_root = event.xmotion.y;
        event.xmotion.window = gameXWindow;
        event.xmotion.time = timestamp;

        XSendEvent(gameDisplay, gameXWindow, False, 0, &event);
    }

    /* Upload the old AllInput struct */
    game_ai.pointer_x += ai.pointer_x - old_ai.pointer_x;
    game_ai.pointer_y += ai.pointer_y - old_ai.pointer_y;
    old_ai.pointer_x = ai.pointer_x;
    old_ai.pointer_y = ai.pointer_y;
}

void generateMouseButtonEvents(void)
{
    struct timespec time = detTimer.getTicks();

    static int xbuttons[] = {Button1,
        Button2, Button3,
        Button4, Button5};
    static int xbuttonmasks[] = {Button1Mask,
        Button2Mask, Button3Mask,
        Button4Mask, Button5Mask};
    static int sdlbuttons[] = {SDL_BUTTON_LEFT,
        SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT,
        SDL_BUTTON_X1, SDL_BUTTON_X2};
    static int sdl1buttons[] = {SDL1::SDL1_BUTTON_LEFT,
        SDL1::SDL1_BUTTON_MIDDLE, SDL1::SDL1_BUTTON_RIGHT,
        SDL1::SDL1_BUTTON_X1, SDL1::SDL1_BUTTON_X2};

    for (int bi=0; bi<5; bi++) {
        if ((ai.pointer_mask & xbuttonmasks[bi]) != (old_ai.pointer_mask & xbuttonmasks[bi])) {
            /* We got a change in a button state */

            /* Fill the event structure */
            if (game_info.mouse & GameInfo::SDL2) {
                SDL_Event event2;
                if (ai.pointer_mask & xbuttonmasks[bi]) {
                    event2.type = SDL_MOUSEBUTTONDOWN;
                    event2.button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONDOWN with button ", sdlbuttons[bi]);
                }
                else {
                    event2.type = SDL_MOUSEBUTTONUP;
                    event2.button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONUP with button ", sdlbuttons[bi]);
                }
                event2.button.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
                LINK_NAMESPACE_SDL2(SDL_GetWindowID);
                event2.button.windowID = orig::SDL_GetWindowID(gameWindow);
                event2.button.which = 0; // TODO: Same as above...
                event2.button.button = sdlbuttons[bi];
                event2.button.clicks = 1;
                event2.button.x = game_ai.pointer_x;
                event2.button.y = game_ai.pointer_y;
                sdlEventQueue.insert(&event2);
            }

            if (game_info.mouse & GameInfo::SDL1) {
                SDL1::SDL_Event event1;
                if (ai.pointer_mask & xbuttonmasks[bi]) {
                    event1.type = SDL1::SDL_MOUSEBUTTONDOWN;
                    event1.button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONDOWN with button ", sdl1buttons[bi]);
                }
                else {
                    event1.type = SDL1::SDL_MOUSEBUTTONUP;
                    event1.button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONUP with button ", sdl1buttons[bi]);
                }
                event1.button.which = 0; // TODO: Same as above...
                event1.button.button = sdl1buttons[bi];
                event1.button.x = (Uint16) game_ai.pointer_x;
                event1.button.y = (Uint16) game_ai.pointer_y;
                sdlEventQueue.insert(&event1);
            }

            if (game_info.mouse & GameInfo::XEVENTS) {
                XEvent event;
                if (ai.pointer_mask & xbuttonmasks[bi]) {
                    event.xbutton.type = ButtonPress;
                }
                else {
                    event.xbutton.type = ButtonRelease;
                }
                event.xbutton.state = ai.pointer_mask;
                event.xbutton.x = game_ai.pointer_x;
                event.xbutton.y = game_ai.pointer_y;
                event.xbutton.x_root = event.xbutton.x;
                event.xbutton.y_root = event.xbutton.y;
                event.xbutton.button = xbuttons[bi];

                XSendEvent(gameDisplay, gameXWindow, False, 0, &event);
            }

            /* Upload the old AllInput struct */
            old_ai.pointer_mask ^= xbuttonmasks[bi];
        }
    }
    game_ai.pointer_mask = ai.pointer_mask;
}

}
