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

#include "inputs.h"
#include "keyboard_helper.h"
#include "../logging.h"
#include "../../shared/AllInputs.h"
#include "../../shared/tasflags.h"
#include <X11/keysym.h>
#include <stdlib.h>
#include "../DeterministicTimer.h"
#include "../windows.h" // for SDL_GetWindowId_real and gameWindow
#include "sdlgamecontroller.h" // sdl_controller_events
#include "sdljoystick.h" // sdl_joystick_event
#include "sdlpointer.h" // MASK constants
#include "../EventQueue.h"

struct AllInputs ai;
struct AllInputs old_ai;
struct AllInputs game_ai;

void generateKeyUpEvents(void)
{
    int i, j;
    struct timespec time;
    for (i=0; i<16; i++) { // TODO: Another magic number
        if (old_ai.keyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<16; j++) {
            if (old_ai.keyboard[i] == ai.keyboard[j]) {
                /* Key was not released */
                break;
            }
        }
        if (j == 16) {
            /* Key was released. Generate event */
            if (SDLver == 2) {
                SDL_Event event2;
                event2.type = SDL_KEYUP;
                event2.key.state = SDL_RELEASED;
                event2.key.windowID = SDL_GetWindowID_real(gameWindow);
                time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                event2.key.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, old_ai.keyboard[i]);
                event2.key.keysym = keysym;

                sdlEventQueue.insert(&event2);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYUP with key ", event2.key.keysym.sym);

            }

            if (SDLver == 1) {
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

            /* Update old keyboard state */
            old_ai.keyboard[i] = XK_VoidSymbol;
        }
    }
}


/* Generate pressed keyboard input events */
void generateKeyDownEvents(void)
{
    int i,j,k;
	struct timespec time;
    for (i=0; i<16; i++) { // TODO: Another magic number
        if (ai.keyboard[i] == XK_VoidSymbol) {
            continue;
        }
        for (j=0; j<16; j++) {
            if (ai.keyboard[i] == old_ai.keyboard[j]) {
                /* Key was not pressed */
                break;
            }
        }
        if (j == 16) {
            /* Key was pressed. Generate event */
            if (SDLver == 2) {
                SDL_Event event2;
                event2.type = SDL_KEYDOWN;
                event2.key.state = SDL_PRESSED;
                event2.key.windowID = SDL_GetWindowID_real(gameWindow);
                time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                event2.key.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, ai.keyboard[i]);
                event2.key.keysym = keysym;

                sdlEventQueue.insert(&event2);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key ", event2.key.keysym.sym);
            }

            if (SDLver == 1) {
                SDL1::SDL_Event event1;
                event1.type = SDL1::SDL_KEYDOWN;
                event1.key.which = 0; // FIXME: I don't know what is going here
                event1.key.state = SDL_PRESSED;

                SDL1::SDL_keysym keysym;
                xkeysymToSDL1(&keysym, ai.keyboard[i]);
                event1.key.keysym = keysym;

                sdlEventQueue.insert(&event1);

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key ", event1.key.keysym.sym);
            }

            /* Update old keyboard state */
            for (k=0; k<16; k++)
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
    if (SDLver == 1)
        return;

    if (!sdl_controller_events)
        return;

    struct timespec time = detTimer.getTicks(TIMETYPE_UNTRACKED);
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (int i = 0; i < tasflags.numControllers; i++) {
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
    /* Ok, doc says that:
     *   "If controller events are disabled, you must call SDL_GameControllerUpdate()
     *   yourself and check the state of the controller when you want controller
     *   information."
     *
     * What happens if GC events are disabled, but Joystick events are
     * enabled? If Joystick events are enabled, SDL_JoystickUpdate() is called
     * during SDL_PumpEvents(). However, Joysticks and GCs use the same storage,
     * GCs are just remapping of Joysticks. As a proof, SDL_GameControllerUpdate()
     * does nothing more than calling SDL_JoystickUpdate().
     *
     * It seems that disabling only SDL_GC events keep updating the joystick
     * states in the event loop. Or maybe when a joystick is detected as GC
     * compatible, it automatically disable Joystick events?
     *
     * For now, I'm going to remove Joy/GC update only when both GC and Joy
     * events are disabled.
     */

    if (!sdl_controller_events && !sdl_joystick_events)
        return;

    struct timespec time = detTimer.getTicks(TIMETYPE_UNTRACKED);
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (int ji=0; ji<tasflags.numControllers; ji++) {
        for (int axis=0; axis<6; axis++) {
            /* Update game_ai axes */
            game_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];

            /* Check for axes change */
            if (ai.controller_axes[ji][axis] != old_ai.controller_axes[ji][axis]) {
                /* We got a change in a controller axis value */

                /* Fill the event structure */
                if (SDLver == 2) {
                    SDL_Event event2;
                    if (sdl_controller_events) {
                        event2.type = SDL_CONTROLLERAXISMOTION;
                        event2.caxis.timestamp = timestamp;
                        event2.caxis.which = ji;
                        event2.caxis.axis = axis;
                        event2.caxis.value = ai.controller_axes[ji][axis];
                        sdlEventQueue.insert(&event2);
                        debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERAXISMOTION with axis ", axis);
                    }

                    event2.type = SDL_JOYAXISMOTION;
                    event2.jaxis.timestamp = timestamp;
                    event2.jaxis.which = ji;
                    event2.jaxis.axis = axis;
                    event2.jaxis.value = ai.controller_axes[ji][axis];
                    sdlEventQueue.insert(&event2);

                    debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis ", axis);
                }

                /* Fill the event structure */
                if (SDLver == 1) {
                    SDL1::SDL_Event event1;
                    event1.type = SDL1::SDL_JOYAXISMOTION;
                    event1.jaxis.which = ji;
                    event1.jaxis.axis = axis;
                    event1.jaxis.value = ai.controller_axes[ji][axis];
                    sdlEventQueue.insert(&event1);

                    debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis ", axis);
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

        for (int bi=0; bi<16; bi++) {
            if (((buttons >> bi) & 0x1) != ((old_buttons >> bi) & 0x1)) {
                /* We got a change in a button state */

                /* Fill the event structure */

                if (sdl_controller_events) {
                    if (SDLver == 2) {
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
                        event2.cbutton.button = bi;
                        sdlEventQueue.insert(&event2);
                    }
                }

                if (bi < 11) {
                    if (SDLver == 2) {
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

                    if (SDLver == 1) {
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
                }

                /* We take care of only generating the hat event once */
                static bool hatGenerated = false;

                if ((!hatGenerated) && (bi >= 11)) {
                    hatGenerated = true;

                    /* Fortunately, we use the fact that SDL_HAT_X constants
                     * are the same in SDL 1 and SDL 2
                     */
                    Uint8 hatState = SDL_HAT_CENTERED;
                    if (buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_UP))
                        hatState |= SDL_HAT_UP;
                    if (buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_DOWN))
                        hatState |= SDL_HAT_DOWN;
                    if (buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_LEFT))
                        hatState |= SDL_HAT_LEFT;
                    if (buttons & (1 << SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
                        hatState |= SDL_HAT_RIGHT;

                    debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYHATMOTION with hat ", hatState);

                    if (SDLver == 2) {
                        /* SDL2 joystick hat */
                        SDL_Event event2;
                        event2.type = SDL_JOYHATMOTION;
                        event2.jhat.timestamp = timestamp;
                        event2.jhat.which = ji;
                        event2.jhat.hat = 0;
                        event2.jhat.value = hatState;
                        sdlEventQueue.insert(&event2);
                    }
                    if (SDLver == 1) {
                        /* SDL1 joystick hat */
                        SDL1::SDL_Event event1;
                        event1.type = SDL1::SDL_JOYHATMOTION;
                        event1.jhat.which = ji;
                        event1.jhat.hat = 0;
                        event1.jhat.value = hatState;
                        sdlEventQueue.insert(&event1);
                    }
                }

                /* Upload the old AllInput struct */
                old_ai.controller_buttons[ji] ^= (1 << bi);
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

    if (SDLver == 2) {
        SDL_Event event2;
        event2.type = SDL_MOUSEMOTION;
        struct timespec time = detTimer.getTicks(TIMETYPE_UNTRACKED);
        event2.motion.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
        event2.motion.windowID = SDL_GetWindowID_real(gameWindow);
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

        event2.motion.x = ai.pointer_x;
        event2.motion.y = ai.pointer_y;
        event2.motion.xrel = ai.pointer_x - old_ai.pointer_x;
        event2.motion.yrel = ai.pointer_y - old_ai.pointer_y;
        sdlEventQueue.insert(&event2);
    }
    if (SDLver == 1) {
        SDL1::SDL_Event event1;
        event1.type = SDL1::SDL_MOUSEMOTION;
        event1.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event1.motion.state = 0;
        if (ai.pointer_mask & Button1Mask)
            event1.motion.state |= SDL1::SDL_BUTTON_LMASK;
        if (ai.pointer_mask & Button2Mask)
            event1.motion.state |= SDL1::SDL_BUTTON_MMASK;
        if (ai.pointer_mask & Button3Mask)
            event1.motion.state |= SDL1::SDL_BUTTON_RMASK;
        if (ai.pointer_mask & Button4Mask)
            event1.motion.state |= SDL1::SDL_BUTTON_X1MASK;
        if (ai.pointer_mask & Button5Mask)
            event1.motion.state |= SDL1::SDL_BUTTON_X2MASK;

        event1.motion.xrel = (Sint16)(ai.pointer_x - old_ai.pointer_x);
        event1.motion.yrel = (Sint16)(ai.pointer_y - old_ai.pointer_y);
        event1.motion.x = (Uint16) (game_ai.pointer_x + event1.motion.xrel);
        event1.motion.y = (Uint16) (game_ai.pointer_y + event1.motion.yrel);
        sdlEventQueue.insert(&event1);
    }
    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEMOTION with new position (", ai.pointer_x, ",", ai.pointer_y,")");

    /* Upload the old AllInput struct */
    old_ai.pointer_x = ai.pointer_x;
    old_ai.pointer_y = ai.pointer_y;
    game_ai.pointer_x += ai.pointer_x - old_ai.pointer_x;
    game_ai.pointer_y += ai.pointer_y - old_ai.pointer_y;
}

void generateMouseButtonEvents(void)
{
    struct timespec time;

    static int xbuttons[] = {SDL_BUTTON_LMASK,
        SDL_BUTTON_MMASK, SDL_BUTTON_RMASK,
        SDL_BUTTON_X1MASK, SDL_BUTTON_X2MASK};
    static int sdlbuttons[] = {SDL_BUTTON_LEFT,
        SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT,
        SDL_BUTTON_X1, SDL_BUTTON_X2};
    static int sdl1buttons[] = {SDL1::SDL_BUTTON_LEFT,
        SDL1::SDL_BUTTON_MIDDLE, SDL1::SDL_BUTTON_RIGHT,
        SDL1::SDL_BUTTON_X1, SDL1::SDL_BUTTON_X2};

    for (int bi=0; bi<5; bi++) {
        if ((ai.pointer_mask & xbuttons[bi]) != (old_ai.pointer_mask & xbuttons[bi])) {
            /* We got a change in a button state */

            /* Fill the event structure */
            if (SDLver == 2) {
                SDL_Event event2;
                if (ai.pointer_mask & xbuttons[bi]) {
                    event2.type = SDL_MOUSEBUTTONDOWN;
                    event2.button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONDOWN with button ", sdlbuttons[bi]);
                }
                else {
                    event2.type = SDL_MOUSEBUTTONUP;
                    event2.button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONUP with button ", sdlbuttons[bi]);
                }
                time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                event2.button.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
                event2.button.windowID = SDL_GetWindowID_real(gameWindow);
                event2.button.which = 0; // TODO: Same as above...
                event2.button.button = sdlbuttons[bi];
                event2.button.clicks = 1;
                event2.button.x = ai.pointer_x;
                event2.button.y = ai.pointer_y;
                sdlEventQueue.insert(&event2);
            }
            if (SDLver == 1) {
                SDL1::SDL_Event event1;
                if (ai.pointer_mask & xbuttons[bi]) {
                    event1.type = SDL1::SDL_MOUSEBUTTONDOWN;
                    event1.button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONDOWN with button ", sdlbuttons[bi]);
                }
                else {
                    event1.type = SDL1::SDL_MOUSEBUTTONUP;
                    event1.button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONUP with button ", sdlbuttons[bi]);
                }
                event1.button.which = 0; // TODO: Same as above...
                event1.button.button = sdl1buttons[bi];
                event1.button.x = (Uint16) ai.pointer_x;
                event1.button.y = (Uint16) ai.pointer_y;
                sdlEventQueue.insert(&event1);
            }

            /* Upload the old AllInput struct */
            old_ai.pointer_mask ^= xbuttons[bi];

        }
    }
}


