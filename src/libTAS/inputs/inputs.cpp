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
#include "sdlgamecontroller.h" // sdl_controller_event
#include "sdlpointer.h" // MASK constants

struct AllInputs ai;
struct AllInputs old_ai;

int generateKeyUpEvent(void *events, int num, int update)
{
    int evi = 0;
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
                SDL_Event* events2 = (SDL_Event*)events;
                events2[evi].type = SDL_KEYUP;
                events2[evi].key.state = SDL_RELEASED;
                events2[evi].key.windowID = SDL_GetWindowID_real(gameWindow);
				time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                events2[evi].key.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, old_ai.keyboard[i]);
                events2[evi].key.keysym = keysym;

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYUP with key ", events2[evi].key.keysym.sym);
            }

            if (SDLver == 1) {
                SDL1::SDL_Event* events1 = (SDL1::SDL_Event*)events;
                events1[evi].type = SDL1::SDL_KEYUP;
                events1[evi].key.which = 0; // FIXME: I don't know what is going here
                events1[evi].key.state = SDL_RELEASED;

                SDL1::SDL_keysym keysym;
                xkeysymToSDL1(&keysym, old_ai.keyboard[i]);
                events1[evi].key.keysym = keysym;

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL1 event KEYUP with key ", events1[evi].key.keysym.sym);
            }

            if (update) {
                /* Update old keyboard state so that this event won't trigger inifinitely */
                old_ai.keyboard[i] = XK_VoidSymbol;
            }
            evi++;

            /* If we reached the asked number of events, returning */
            if (evi == num)
                return evi;

        }
    }
    /* We did not reached the asked number of events, returning the number */
    return evi;
}


/* Generate pressed keyboard input events */
int generateKeyDownEvent(void *events, int num, int update)
{
    int evi = 0;
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
                SDL_Event* events2 = (SDL_Event*)events;
                events2[evi].type = SDL_KEYDOWN;
                events2[evi].key.state = SDL_PRESSED;
                events2[evi].key.windowID = SDL_GetWindowID_real(gameWindow);
				time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                events2[evi].key.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

                SDL_Keysym keysym;
                xkeysymToSDL(&keysym, ai.keyboard[i]);
                events2[evi].key.keysym = keysym;

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key ", events2[evi].key.keysym.sym);
            }

            if (SDLver == 1) {
                SDL1::SDL_Event* events1 = (SDL1::SDL_Event*)events;
                events1[evi].type = SDL1::SDL_KEYDOWN;
                events1[evi].key.which = 0; // FIXME: I don't know what is going here
                events1[evi].key.state = SDL_PRESSED;

                SDL1::SDL_keysym keysym;
                xkeysymToSDL1(&keysym, ai.keyboard[i]);
                events1[evi].key.keysym = keysym;

                debuglog(LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event KEYDOWN with key ", events1[evi].key.keysym.sym);
            }

            if (update) {
                /* Update old keyboard state so that this event won't trigger inifinitely */
                for (k=0; k<16; k++)
                    if (old_ai.keyboard[k] == XK_VoidSymbol) {
                        /* We found an empty space to put our key*/
                        old_ai.keyboard[k] = ai.keyboard[i];
                        break;
                    }
            }

            evi++;

            /* Returning if we reached the asked number of events */
            if (evi == num)
                return evi;

        }
    }

    /* Returning the number of generated events */
    return evi;
}


/* Generate SDL2 GameController events */

int generateControllerAdded(SDL_Event* events, int num, int update)
{
    struct timespec time;

    /* Number of controllers added in total */
    static int controllersAdded = 0;

    /* Number of controllers added during this function call */
    int curAdded = 0;

    while ((curAdded < num) && ((curAdded + controllersAdded) < tasflags.numControllers)) {
        events[curAdded].type = SDL_CONTROLLERDEVICEADDED;
        time = detTimer.getTicks(TIMETYPE_UNTRACKED);
        events[curAdded].cdevice.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
        events[curAdded].cdevice.which = curAdded + controllersAdded;
        curAdded++;
    }
    if (update)
        controllersAdded += curAdded;
    return curAdded;
}

int generateControllerEvent(SDL_Event* events, int num, int update)
{
    int evi = 0;
	struct timespec time;
    if (!sdl_controller_events)
        return 0;
    int ji = 0;
    for (ji=0; ji<tasflags.numControllers; ji++) {
        /* Check for axes change */
        int axis;
        for (axis=0; axis<6; axis++) {
            if (ai.controller_axes[ji][axis] != old_ai.controller_axes[ji][axis]) {
                /* We got a change in a controller axis value */

                /* Fill the event structure */
                events[evi].type = SDL_CONTROLLERAXISMOTION;
				time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                events[evi].caxis.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
                events[evi].caxis.which = ji;
                events[evi].caxis.axis = axis;
                events[evi].caxis.value = ai.controller_axes[ji][axis];
                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERAXISMOTION with axis ", axis);

                if (update) {
                    /* Upload the old AllInput struct */
                    old_ai.controller_axes[ji][axis] = ai.controller_axes[ji][axis];
                }

                evi++;

                if (evi == num) {
                    /* Return the number of events */
                    return evi;
                }
            }
        }

        /* Check for button change */
        int bi;
        unsigned short buttons = ai.controller_buttons[ji];
        unsigned short old_buttons = old_ai.controller_buttons[ji];

        for (bi=0; bi<16; bi++) {
            if (((buttons >> bi) & 0x1) != ((old_buttons >> bi) & 0x1)) {
                /* We got a change in a button state */

                /* Fill the event structure */
                if ((buttons >> bi) & 0x1) {
                    events[evi].type = SDL_CONTROLLERBUTTONDOWN;
                    events[evi].cbutton.state = SDL_PRESSED;
                }
                else {
                    events[evi].type = SDL_CONTROLLERBUTTONUP;
                    events[evi].cbutton.state = SDL_RELEASED;
                }
				time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                events[evi].cbutton.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
                events[evi].cbutton.which = ji;
                events[evi].cbutton.button = bi;
                debuglog(LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERBUTTONX with button ", bi);

                if (update) {
                    /* Upload the old AllInput struct */
                    old_ai.controller_buttons[ji] ^= (1 << bi);
                }

                if (evi == num) {
                    /* Return the number of events */
                    return evi;
                }
            }
        }
    }

    /* We did not reached the asked number of events, returning the number */
    return evi;
}

int generateMouseMotionEvent(void* event, int update)
{
    if ((ai.pointer_x == old_ai.pointer_x) && (ai.pointer_y == old_ai.pointer_y))
        return 0;

    /* We got a change in mouse position */

    /* Fill the event structure */
    /* TODO: Deal if pointer is out of screen */

    if (SDLver == 2) {
        SDL_Event* event2 = (SDL_Event*)event;
        event2->type = SDL_MOUSEMOTION;
        struct timespec time = detTimer.getTicks(TIMETYPE_UNTRACKED);
        event2->motion.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
        event2->motion.windowID = SDL_GetWindowID_real(gameWindow);
        event2->motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event2->motion.state = 0;
        if (ai.pointer_mask & Button1Mask)
            event2->motion.state |= SDL_BUTTON_LMASK;
        if (ai.pointer_mask & Button2Mask)
            event2->motion.state |= SDL_BUTTON_MMASK;
        if (ai.pointer_mask & Button3Mask)
            event2->motion.state |= SDL_BUTTON_RMASK;
        if (ai.pointer_mask & Button4Mask)
            event2->motion.state |= SDL_BUTTON_X1MASK;
        if (ai.pointer_mask & Button5Mask)
            event2->motion.state |= SDL_BUTTON_X2MASK;

        event2->motion.x = ai.pointer_x;
        event2->motion.y = ai.pointer_y;
        event2->motion.xrel = ai.pointer_x - old_ai.pointer_x;
        event2->motion.yrel = ai.pointer_y - old_ai.pointer_y;
    }
    if (SDLver == 1) {
        SDL1::SDL_Event* event1 = (SDL1::SDL_Event*)event;
        event1->type = SDL1::SDL_MOUSEMOTION;
        event1->motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event1->motion.state = 0;
        if (ai.pointer_mask & Button1Mask)
            event1->motion.state |= SDL1::SDL_BUTTON_LMASK;
        if (ai.pointer_mask & Button2Mask)
            event1->motion.state |= SDL1::SDL_BUTTON_MMASK;
        if (ai.pointer_mask & Button3Mask)
            event1->motion.state |= SDL1::SDL_BUTTON_RMASK;
        if (ai.pointer_mask & Button4Mask)
            event1->motion.state |= SDL1::SDL_BUTTON_X1MASK;
        if (ai.pointer_mask & Button5Mask)
            event1->motion.state |= SDL1::SDL_BUTTON_X2MASK;

        event1->motion.x = (Uint16) ai.pointer_x;
        event1->motion.y = (Uint16) ai.pointer_y;
        event1->motion.xrel = (Sint16)(ai.pointer_x - old_ai.pointer_x);
        event1->motion.yrel = (Sint16)(ai.pointer_y - old_ai.pointer_y);
    }
    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEMOTION with new position (", ai.pointer_x, ",", ai.pointer_y,")");

    if (update) {
        /* Upload the old AllInput struct */
        old_ai.pointer_x = ai.pointer_x;
        old_ai.pointer_y = ai.pointer_y;
    }

    return 1;
}

int generateMouseButtonEvent(void* events, int num, int update)
{
    int evi = 0;
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
                SDL_Event* events2 = (SDL_Event*)events;
                if (ai.pointer_mask & xbuttons[bi]) {
                    events2[evi].type = SDL_MOUSEBUTTONDOWN;
                    events2[evi].button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONDOWN with button ", sdlbuttons[bi]);
                }
                else {
                    events2[evi].type = SDL_MOUSEBUTTONUP;
                    events2[evi].button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONUP with button ", sdlbuttons[bi]);
                }
                time = detTimer.getTicks(TIMETYPE_UNTRACKED);
                events2[evi].button.timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;
                events2[evi].button.windowID = SDL_GetWindowID_real(gameWindow);
                events2[evi].button.which = 0; // TODO: Same as above...
                events2[evi].button.button = sdlbuttons[bi];
                events2[evi].button.clicks = 1;
                events2[evi].button.x = ai.pointer_x;
                events2[evi].button.y = ai.pointer_y;
            }
            if (SDLver == 1) {
                SDL1::SDL_Event* events1 = (SDL1::SDL_Event*)events;
                if (ai.pointer_mask & xbuttons[bi]) {
                    events1[evi].type = SDL1::SDL_MOUSEBUTTONDOWN;
                    events1[evi].button.state = SDL_PRESSED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONDOWN with button ", sdlbuttons[bi]);
                }
                else {
                    events1[evi].type = SDL1::SDL_MOUSEBUTTONUP;
                    events1[evi].button.state = SDL_RELEASED;
                    debuglog(LCF_SDL | LCF_EVENTS | LCF_MOUSE | LCF_UNTESTED, "Generate SDL event MOUSEBUTTONUP with button ", sdlbuttons[bi]);
                }
                events1[evi].button.which = 0; // TODO: Same as above...
                events1[evi].button.button = sdl1buttons[bi];
                events1[evi].button.x = (Uint16) ai.pointer_x;
                events1[evi].button.y = (Uint16) ai.pointer_y;
            }
            if (update) {
                /* Upload the old AllInput struct */
                old_ai.pointer_mask ^= xbuttons[bi];
            }

            if (evi == num) {
                /* Return the number of events */
                return evi;
            }
        }
    }

    /* We did not reached the asked number of events, returning the number */
    return evi;
}


