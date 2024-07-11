/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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
#ifdef __unix__
#include "config.h"
#endif
#include "inputs.h"
#include "keyboard_helper.h"
#include "sdlgamecontroller.h" // sdl_controller_events
#include "sdljoystick.h" // sdl_joystick_event
#include "sdltextinput.h" // SDL_EnableUNICODE
#ifdef __unix__
#include "xinput.h"
#include "xpointer.h"
#include "xkeyboardlayout.h"
#endif
#ifdef __linux__
#include "jsdev.h"
#include "evdev.h"
#endif

#include "logging.h"
#include "../shared/inputs/AllInputsFlat.h"
#include "../shared/inputs/SingleInput.h"
#include "../shared/SharedConfig.h"
#include "DeterministicTimer.h"
#include "sdl/SDLEventQueue.h"
#include "../external/SDL1.h"
#include "global.h" // Global::game_info
#include "GlobalState.h"
#ifdef __unix__
#include "xlib/XlibEventQueueList.h"
#include "xcb/XcbEventQueueList.h"
#include "xcb/xcbconnection.h" // x11::gameConnections
#include "xlib/xevents.h"
#include "xlib/xdisplay.h" // x11::gameDisplays
#include "xlib/xwindows.h" // x11::gameXWindows
#endif

#include <stdlib.h>
#include <SDL2/SDL.h>
#ifdef __linux__
#include <linux/joystick.h>
#include <linux/input.h>
#endif

namespace libtas {

/* Generate one keyreleased event */
static void generateKeyEvent(int event_key, bool pressed)
{
    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    /* Key was released. Generate event */
    if (Global::game_info.keyboard & GameInfo::SDL2) {
        SDL_Event event2;
        event2.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
        event2.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;
        event2.key.windowID = 1;
        event2.key.timestamp = timestamp;
        event2.key.repeat = 0;

        SDL_Keysym keysym;
        xkeysymToSDL(&keysym, event_key);
        keysym.mod = xkeyboardToSDLMod(Inputs::game_ai.keyboard);
        event2.key.keysym = keysym;

        sdlEventQueue.insert(&event2);

        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event %s with key %d", pressed?"KEYDOWN":"KEYUP", event2.key.keysym.sym);

        /* Generate a text input event if active */
        if (pressed) {
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
                
                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL event SDL_TEXTINPUT with text %s", event2.text.text);
            }
        }
    }

    if (Global::game_info.keyboard & GameInfo::SDL1) {
        SDL1::SDL_Event event1;
        event1.type = pressed ? SDL1::SDL_KEYDOWN : SDL1::SDL_KEYUP;
        event1.key.state = pressed ? SDL_PRESSED : SDL_RELEASED;            
        event1.key.which = 0; // FIXME: I don't know what is going here

        SDL1::SDL_keysym keysym;
        xkeysymToSDL1(&keysym, event_key);
        event1.key.keysym = keysym;

        int isUnicodeEnabled;
        NOLOGCALL(isUnicodeEnabled = SDL_EnableUNICODE(-1));
        if (isUnicodeEnabled) {
            /* Add an Unicode representation of the key */
            /* SDL keycode is identical to its char number for common chars */
            event1.key.keysym.unicode = static_cast<char>(event1.key.keysym.sym & 0xff);
        }

        sdlEventQueue.insert(&event1);

        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_KEYBOARD, "Generate SDL1 event %s with key %d", pressed?"KEYDOWN":"KEYUP", event1.key.keysym.sym);
    }

#ifdef __unix__
    if ((Global::game_info.keyboard & GameInfo::XEVENTS) && !x11::gameXWindows.empty()) {
        XEvent event;
        event.xkey.type = pressed ? KeyPress : KeyRelease;
        event.xkey.state = 0; // TODO: Do we have to set the key modifiers?
        event.xkey.window = x11::gameXWindows.front();
        event.xkey.time = timestamp; // TODO: Wrong! timestamp is from X server start
        event.xkey.same_screen = 1;
        event.xkey.send_event = 0;
        event.xkey.subwindow = 0;
        event.xkey.root = x11::rootWindow;
        NOLOGCALL(event.xkey.keycode = XKeysymToKeycode(nullptr, event_key));
        event.xkey.state = xkeyboardToXMod(Inputs::game_ai.keyboard);
        for (int d=0; d<GAMEDISPLAYNUM; d++) {
            if (x11::gameDisplays[d]) {
                event.xkey.root = XRootWindow(x11::gameDisplays[d], 0);
                xlibEventQueueList.insert(x11::gameDisplays[d], &event);
            }
        }

        LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate XEvent %s with keycode %d", pressed?"KeyPress":"KeyRelease", event.xkey.keycode);
    }

    if ((Global::game_info.keyboard & GameInfo::XCBEVENTS) && !x11::gameXWindows.empty()) {
        xcb_key_release_event_t event;
        event.response_type = pressed ? XCB_KEY_PRESS : XCB_KEY_RELEASE;
        event.state = 0; // TODO: Do we have to set the key modifiers?
        event.event = x11::gameXWindows.front();
        event.time = timestamp; // TODO: Wrong! timestamp is from X server start
        event.same_screen = 1;
        event.child = 0;
        event.root = x11::rootWindow;
        NOLOGCALL(event.detail = XKeysymToKeycode(nullptr, event_key));
        event.state = static_cast<uint16_t>(xkeyboardToXMod(Inputs::game_ai.keyboard));
        for (int c=0; c<GAMECONNECTIONNUM; c++) {
            if (x11::gameConnections[c]) {
                // event.root = XRootWindow(x11::gameConnections[c], 0);
                xcbEventQueueList.insert(x11::gameConnections[c], reinterpret_cast<xcb_generic_event_t*>(&event), false);
            }
        }

        LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate xcb %s with keycode %d", pressed?"XCB_KEY_PRESS":"XCB_KEY_RELEASE", event.detail);
    }

    if ((Global::game_info.keyboard & GameInfo::XIEVENTS) && !x11::gameXWindows.empty()) {
        XEvent event;
        XIDeviceEvent *dev = static_cast<XIDeviceEvent*>(calloc(1, sizeof(XIDeviceEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        event.xcookie.evtype = pressed ? XI_KeyPress : XI_KeyRelease;
        event.xcookie.data = dev;
        dev->evtype = event.xcookie.evtype;
        dev->event = x11::gameXWindows.front();
        dev->time = timestamp; // TODO: Wrong! timestamp is from X server start
        dev->deviceid = 3;
        dev->sourceid = 3;
        NOLOGCALL(dev->detail = XKeysymToKeycode(nullptr, event_key));
        dev->mods.effective = xkeyboardToXMod(Inputs::game_ai.keyboard); // We should not need to fill the other members
        for (int d=0; d<GAMEDISPLAYNUM; d++) {
            if (x11::gameDisplays[d]) {
                dev->root = XRootWindow(x11::gameDisplays[d], 0);
                xlibEventQueueList.insert(x11::gameDisplays[d], &event);
            }
        }

        LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent %s with keycode %d", pressed?"KeyPress":"KeyRelease", dev->detail);
    }

    if (Global::game_info.keyboard & GameInfo::XIRAWEVENTS) {
        XEvent event;
        XIRawEvent *rev = static_cast<XIRawEvent*>(calloc(1, sizeof(XIRawEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        event.xcookie.evtype = pressed ? XI_RawKeyPress : XI_RawKeyRelease;
        event.xcookie.data = rev;
        rev->evtype = event.xcookie.evtype;
        rev->time = timestamp; // TODO: Wrong! timestamp is from X server start
        NOLOGCALL(rev->detail = XKeysymToKeycode(nullptr, event_key));
        xlibEventQueueList.insert(&event);

        LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent %s with keycode %d", pressed?"RawKeyPress":"RawKeyRelease", rev->detail);
    }
#endif
}

/* Generate events of type SDL_KEYUP or KeyRelease */
static void generateKeyUpEvents(void)
{
    int i, j;

    for (i=0; i<AllInputsFlat::MAXKEYS; i++) {
        if (!Inputs::old_game_ai.keyboard[i]) {
            continue;
        }
        for (j=0; j<AllInputsFlat::MAXKEYS; j++) {
            if (Inputs::old_game_ai.keyboard[i] == Inputs::game_ai.keyboard[j]) {
                /* Key was not released */
                break;
            }
        }
        if (j == AllInputsFlat::MAXKEYS) {
            generateKeyEvent(Inputs::old_game_ai.keyboard[i], false);
        }
    }
}

/* Generate pressed keyboard input events */
static void generateKeyDownEvents(void)
{
    int i,j;

    for (i=0; i<AllInputsFlat::MAXKEYS; i++) {
        if (!Inputs::game_ai.keyboard[i]) {
            continue;
        }
        for (j=0; j<AllInputsFlat::MAXKEYS; j++) {
            if (Inputs::game_ai.keyboard[i] == Inputs::old_game_ai.keyboard[j]) {
                /* Key was not pressed */
                break;
            }
        }
        if (j == AllInputsFlat::MAXKEYS) {
            /* Key was pressed. Generate event */
            generateKeyEvent(Inputs::game_ai.keyboard[i], true);
        }
    }
}

/* Generate events indicating that a controller was plugged in */
static void generateControllerAdded(void)
{
    if (!(Global::game_info.joystick & GameInfo::SDL2))
        return;

    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    static bool init_added = false;

    if (!init_added) {
        init_added = true;
        for (int i = 0; i < Global::shared_config.nb_controllers; i++) {
            SDL_Event ev;
            ev.type = SDL_CONTROLLERDEVICEADDED;
            ev.cdevice.timestamp = timestamp;
            ev.cdevice.which = i;
            sdlEventQueue.insert(&ev);
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_CONTROLLERDEVICEADDED with joy %d", i);

            ev.type = SDL_JOYDEVICEADDED;
            ev.jdevice.timestamp = timestamp;
            ev.jdevice.which = i;
            sdlEventQueue.insert(&ev);
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_JOYDEVICEADDED with joy %d", i);
        }
    }

    if (!Inputs::game_ai.misc.flags) return;

    int changed_flags[4] = {
        SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED,
        SingleInput::FLAG_CONTROLLER2_ADDED_REMOVED,
        SingleInput::FLAG_CONTROLLER3_ADDED_REMOVED,
        SingleInput::FLAG_CONTROLLER4_ADDED_REMOVED,
    };

    for (int i=0; i<4; i++) {
        if ((Inputs::game_ai.misc.flags & (1<<changed_flags[i])) &&
            (Global::shared_config.nb_controllers >= i)) {
                
            bool attached = mySDL_GameControllerIsAttached(i);
            SDL_Event ev;
            ev.type = attached ? SDL_CONTROLLERDEVICEREMOVED : SDL_CONTROLLERDEVICEADDED;
            ev.cdevice.timestamp = timestamp;
            ev.cdevice.which = i;
            sdlEventQueue.insert(&ev);
            if (attached)
                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_CONTROLLERDEVICEREMOVED with joy %d", i);
            else
                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_CONTROLLERDEVICEADDED with joy %d", i);

            ev.type = attached ? SDL_JOYDEVICEADDED : SDL_JOYDEVICEREMOVED;
            ev.jdevice.timestamp = timestamp;
            ev.jdevice.which = i;
            sdlEventQueue.insert(&ev);
            if (attached)
                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_JOYDEVICEREMOVED with joy %d", i);
            else
                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event SDL_JOYDEVICEADDED with joy %d", i);
                
            /* Change the state of controller */
            mySDL_GameControllerChangeAttached(i);
        }
    }
}

/* Same as KeyUp/KeyDown functions but with controller events */
static void generateControllerEvents(void)
{
    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (int ji=0; ji<Global::shared_config.nb_controllers; ji++) {
        /* Check if we need to generate any joystick events for that
         * particular joystick. If not, we {continue;} here because
         * we must not update the joystick state (Inputs::game_ai) as specified
         * in the SDL documentation. The game must then call
         * SDL_[Joystick/GameController]Update to update the joystick state.
         */
        bool genGC = true, genJoy = true;

        if (Global::game_info.joystick & GameInfo::SDL2) {
            GlobalNoLog gnl;
            genGC = (SDL_GameControllerEventState(SDL_QUERY) == SDL_ENABLE) && SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&ji));
            //bool genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && SDL_JoystickGetAttached(&ji);
            /* I'm not sure this is the right thing to do, but enabling joystick events when only the GC is opened */
            genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && (SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&ji)) || SDL_GameControllerGetAttached(reinterpret_cast<SDL_GameController*>(&ji)));

            if (!genGC && !genJoy)
                continue;
        }

        if (Global::game_info.joystick & GameInfo::SDL1) {
            GlobalNoLog gnl;
            genJoy = (SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE) && SDL_JoystickGetAttached(reinterpret_cast<SDL_Joystick*>(&ji));

            if (!genJoy)
                continue;
        }

        for (int axis=0; axis<ControllerInputs::MAXAXES; axis++) {
            /* Check for axes change */
            if (Inputs::game_ai.controllers[ji].axes[axis] != Inputs::old_game_ai.controllers[ji].axes[axis]) {
                /* We got a change in a controller axis value */

                if (Global::game_info.joystick & GameInfo::SDL2) {
                    if (genGC) {
                        SDL_Event event2;
                        event2.type = SDL_CONTROLLERAXISMOTION;
                        event2.caxis.timestamp = timestamp;
                        event2.caxis.which = ji;
                        event2.caxis.axis = SingleInput::toSDL2Axis(axis);
                        event2.caxis.value = Inputs::game_ai.controllers[ji].axes[axis];
                        sdlEventQueue.insert(&event2);
                        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERAXISMOTION with axis %d", axis);
                    }
                    if (genJoy) {
                        SDL_Event event2;
                        event2.type = SDL_JOYAXISMOTION;
                        event2.jaxis.timestamp = timestamp;
                        event2.jaxis.which = ji;
                        event2.jaxis.axis = axis;
                        event2.jaxis.value = Inputs::game_ai.controllers[ji].axes[axis];
                        sdlEventQueue.insert(&event2);
                        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis %d", axis);
                    }
                }

                if (Global::game_info.joystick & GameInfo::SDL1) {
                    SDL1::SDL_Event event1;
                    event1.type = SDL1::SDL_JOYAXISMOTION;
                    event1.jaxis.which = ji;
                    event1.jaxis.axis = axis;
                    event1.jaxis.value = Inputs::game_ai.controllers[ji].axes[axis];
                    sdlEventQueue.insert(&event1);
                    LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYAXISMOTION with axis %d", axis);
                }

#ifdef __linux__
                if (Global::game_info.joystick & GameInfo::JSDEV) {
                    struct js_event ev;
                    ev.time = timestamp;
                    ev.type = JS_EVENT_AXIS;
                    ev.number = SingleInput::toJsdevAxis(axis);
                    ev.value = Inputs::game_ai.controllers[ji].axes[axis];
                    write_jsdev(ev, ji);
                    LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis %d", axis);
                }

                if (Global::game_info.joystick & GameInfo::EVDEV) {
                    struct input_event ev;
                    ev.time.tv_sec = time.tv_sec;
                    ev.time.tv_usec = time.tv_nsec / 1000;
                    ev.type = EV_ABS;
                    ev.code = SingleInput::toEvdevAxis(axis);
                    ev.value = Inputs::game_ai.controllers[ji].axes[axis];
                    write_evdev(ev, ji);
                    LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis %d", axis);
                }
#endif
            }
        }

        /* Check for button change */
        unsigned short buttons = Inputs::game_ai.controllers[ji].buttons;
        unsigned short old_buttons = Inputs::old_game_ai.controllers[ji].buttons;

        /* We generate the hat event separately from the buttons,
         * but we still check here if hat has changed */
        bool hatHasChanged = false;

        for (int bi=0; bi<16; bi++) {
            if (((buttons >> bi) & 0x1) != ((old_buttons >> bi) & 0x1)) {
                /* We got a change in a button state */

                if (Global::game_info.joystick & GameInfo::SDL2) {
                    if (genGC) {
                        /* SDL2 controller button */
                        SDL_Event event2;
                        if ((buttons >> bi) & 0x1) {
                            event2.type = SDL_CONTROLLERBUTTONDOWN;
                            event2.cbutton.state = SDL_PRESSED;
                            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERBUTTONDOWN with button %d", bi);
                        }
                        else {
                            event2.type = SDL_CONTROLLERBUTTONUP;
                            event2.cbutton.state = SDL_RELEASED;
                            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event CONTROLLERBUTTONUP with button %d", bi);
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
                                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONDOWN with button %d", bi);
                            }
                            else {
                                event2.type = SDL_JOYBUTTONUP;
                                event2.jbutton.state = SDL_RELEASED;
                                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONUP with button %d", bi);
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

                if (Global::game_info.joystick & GameInfo::SDL1) {
                    if (bi < 11) {
                        /* SDL1 joystick button */
                        SDL1::SDL_Event event1;
                        if ((buttons >> bi) & 0x1) {
                            event1.type = SDL1::SDL_JOYBUTTONDOWN;
                            event1.jbutton.state = SDL_PRESSED;
                            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONDOWN with button %d", bi);
                        }
                        else {
                            event1.type = SDL1::SDL_JOYBUTTONUP;
                            event1.jbutton.state = SDL_RELEASED;
                            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYBUTTONUP with button %d", bi);
                        }
                        event1.jbutton.which = ji;
                        event1.jbutton.button = bi;
                        sdlEventQueue.insert(&event1);
                    }
                    else {
                        hatHasChanged = true;
                    }
                }

#ifdef __linux__
                if (Global::game_info.joystick & GameInfo::JSDEV) {
                    if (bi < 11) { // JSDEV joystick only has 11 buttons
                        struct js_event ev;
                        ev.time = timestamp;
                        ev.type = JS_EVENT_BUTTON;
                        ev.number = SingleInput::toJsdevButton(bi);
                        ev.value = (buttons >> bi) & 0x1;
                        LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_BUTTON with button %d", bi);
                        write_jsdev(ev, ji);
                    }
                    else {
                        hatHasChanged = true;
                    }
                }

                if (Global::game_info.joystick & GameInfo::EVDEV) {
                    if (bi < 11) { // EVDEV joystick only has 11 buttons
                        struct input_event ev;
                        ev.time.tv_sec = time.tv_sec;
                        ev.time.tv_usec = time.tv_nsec / 1000;
                        ev.type = EV_KEY;
                        ev.code = SingleInput::toEvdevButton(bi);
                        ev.value = (buttons >> bi) & 0x1;
                        LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_KEY with button %d", bi);
                        write_evdev(ev, ji);
                    }
                    else {
                        hatHasChanged = true;
                    }
                }
#endif
            }
        }

        /* Generate hat state */
        if (hatHasChanged) {

            if (Global::game_info.joystick & GameInfo::SDL2) {
                /* SDL2 joystick hat */
                SDL_Event event2;
                event2.type = SDL_JOYHATMOTION;
                event2.jhat.timestamp = timestamp;
                event2.jhat.which = ji;
                event2.jhat.hat = 0;
                event2.jhat.value = SingleInput::toSDLHat(buttons);
                sdlEventQueue.insert(&event2);
                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYHATMOTION with hat %d", (int)event2.jhat.value);
            }

            if (Global::game_info.joystick & GameInfo::SDL1) {
                /* SDL1 joystick hat */
                SDL1::SDL_Event event1;
                event1.type = SDL1::SDL_JOYHATMOTION;
                event1.jhat.which = ji;
                event1.jhat.hat = 0;
                event1.jhat.value = SingleInput::toSDLHat(buttons);
                sdlEventQueue.insert(&event1);
                LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_JOYSTICK, "Generate SDL event JOYHATMOTION with hat %d", (int)event1.jhat.value);
            }

#ifdef __linux__
            if (Global::game_info.joystick & GameInfo::JSDEV) {
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
                    LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis 6");
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
                    LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate jsdev event JS_EVENT_AXIS with axis 7");
                }
            }

            if (Global::game_info.joystick & GameInfo::EVDEV) {
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
                    LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis %d", ABS_HAT0X);
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
                    LOG(LL_DEBUG, LCF_EVENTS | LCF_JOYSTICK, "Generate evdev event EV_ABS with axis %d", ABS_HAT0Y);
                }
            }
#endif
        }
    }
}

/* Same as above with MouseMotion event */
static void generateMouseMotionEvents(void)
{
    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

#ifdef __unix__
    /* XIRAWEVENTS are special because they output raw pointer events */
    if ((Global::game_info.mouse & GameInfo::XIRAWEVENTS) &&
        ((Inputs::game_unclipped_pointer.x != Inputs::old_game_unclipped_pointer.x) || (Inputs::game_unclipped_pointer.y != Inputs::old_game_unclipped_pointer.y))) {
        XEvent event;
        XIRawEvent *rev = static_cast<XIRawEvent*>(calloc(1, sizeof(XIRawEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        event.xcookie.evtype = XI_RawMotion;
        event.xcookie.data = rev;
        rev->evtype = XI_RawMotion;
        rev->time = timestamp;
        rev->raw_values = static_cast<double*>(malloc(2*sizeof(double)));
        rev->raw_values[0] = Inputs::game_unclipped_pointer.x - Inputs::old_game_unclipped_pointer.x;
        rev->raw_values[1] = Inputs::game_unclipped_pointer.y - Inputs::old_game_unclipped_pointer.y;
        rev->valuators.values = static_cast<double*>(malloc(2*sizeof(double)));
        rev->valuators.values[0] = Inputs::game_unclipped_pointer.x - Inputs::old_game_unclipped_pointer.x;
        rev->valuators.values[1] = Inputs::game_unclipped_pointer.y - Inputs::old_game_unclipped_pointer.y;
        rev->valuators.mask = static_cast<unsigned char*>(malloc(1*sizeof(unsigned char)));
        rev->valuators.mask[0] = 0;
        XISetMask(rev->valuators.mask, 0);
        XISetMask(rev->valuators.mask, 1);
        rev->valuators.mask_len = 1;
        xlibEventQueueList.insert(&event);

        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate XIEvent XI_RawMotion");
    }
#endif

    /* Check if we got a change in mouse position */
    if ((Inputs::game_ai.pointer.x == Inputs::old_game_ai.pointer.x) && (Inputs::game_ai.pointer.y == Inputs::old_game_ai.pointer.y))
        return;

    if (Global::game_info.mouse & GameInfo::SDL2) {
        SDL_Event event2;
        event2.type = SDL_MOUSEMOTION;
        event2.motion.timestamp = timestamp;
        event2.motion.windowID = 1;
        event2.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event2.motion.state = SingleInput::toSDL2PointerMask(Inputs::game_ai.pointer.mask);

        /* Relative movement is not subject to window clipping */
        event2.motion.xrel = Inputs::game_unclipped_pointer.x - Inputs::old_game_unclipped_pointer.x;
        event2.motion.yrel = Inputs::game_unclipped_pointer.y - Inputs::old_game_unclipped_pointer.y;
        event2.motion.x = Inputs::game_ai.pointer.x;
        event2.motion.y = Inputs::game_ai.pointer.y;
        sdlEventQueue.insert(&event2);
        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEMOTION with new position (%d,%d)", Inputs::game_ai.pointer.x, Inputs::game_ai.pointer.y);
    }

    if (Global::game_info.mouse & GameInfo::SDL1) {
        SDL1::SDL_Event event1;
        event1.type = SDL1::SDL_MOUSEMOTION;
        event1.motion.which = 0; // TODO: Mouse instance id. No idea what to put here...

        /* Build up mouse state */
        event1.motion.state = SingleInput::toSDL1PointerMask(Inputs::game_ai.pointer.mask);

        /* Relative movement is not subject to window clipping */
        event1.motion.xrel = (Sint16)(Inputs::game_unclipped_pointer.x - Inputs::old_game_unclipped_pointer.x);
        event1.motion.yrel = (Sint16)(Inputs::game_unclipped_pointer.y - Inputs::old_game_unclipped_pointer.y);
        event1.motion.x = (Uint16) Inputs::game_ai.pointer.x;
        event1.motion.y = (Uint16) Inputs::game_ai.pointer.y;
        sdlEventQueue.insert(&event1);
        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEMOTION with new position (%d,%d)", Inputs::game_ai.pointer.x, Inputs::game_ai.pointer.y);
    }

#ifdef __unix__
    if ((Global::game_info.mouse & GameInfo::XEVENTS) && !x11::gameXWindows.empty()) {
        XEvent event;
        event.xmotion.type = MotionNotify;
        event.xmotion.state = SingleInput::toXlibPointerMask(Inputs::game_ai.pointer.mask);
        event.xmotion.x = Inputs::game_ai.pointer.x;
        event.xmotion.y = Inputs::game_ai.pointer.y;
        event.xmotion.x_root = event.xmotion.x;
        event.xmotion.y_root = event.xmotion.y;
        if (pointer_grab_window != None)
            event.xmotion.window = pointer_grab_window;
        else
            event.xmotion.window = x11::gameXWindows.front();
        event.xmotion.send_event = 0;
        event.xmotion.subwindow = 0;
        event.xmotion.root = x11::rootWindow;
        event.xmotion.same_screen = 1;
        event.xmotion.time = timestamp;
        event.xmotion.is_hint = 0;

        xlibEventQueueList.insert(&event);
        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate Xlib event MotionNotify with new position (%d,%d)", Inputs::game_ai.pointer.x, Inputs::game_ai.pointer.y);
    }

    if ((Global::game_info.mouse & GameInfo::XCBEVENTS) && !x11::gameXWindows.empty()) {
        xcb_motion_notify_event_t event;
        event.response_type = XCB_MOTION_NOTIFY;
        event.state = SingleInput::toXlibPointerMask(Inputs::game_ai.pointer.mask);
        event.event_x = Inputs::game_ai.pointer.x;
        event.event_y = Inputs::game_ai.pointer.y;
        event.root_x = Inputs::game_ai.pointer.x;
        event.root_y = Inputs::game_ai.pointer.y;
        event.event = x11::gameXWindows.front();
        event.time = timestamp;
        event.same_screen = 1;
        event.child = 0;
        event.root = x11::rootWindow;

        xcbEventQueueList.insert(reinterpret_cast<xcb_generic_event_t*>(&event), false);
        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_MOTION_NOTIFY with new position (%d,%d)", Inputs::game_ai.pointer.x, Inputs::game_ai.pointer.y);
    }

    if ((Global::game_info.mouse & GameInfo::XIEVENTS) && !x11::gameXWindows.empty()) {
        XEvent event;
        XIDeviceEvent *dev = static_cast<XIDeviceEvent*>(calloc(1, sizeof(XIDeviceEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        event.xcookie.evtype = XI_Motion;
        event.xcookie.data = dev;
        dev->evtype = XI_Motion;
        dev->event = x11::gameXWindows.front();
        dev->time = timestamp;
        dev->deviceid = 2;
        dev->sourceid = 2;
        dev->event_x = Inputs::game_ai.pointer.x;
        dev->event_y = Inputs::game_ai.pointer.y;
        dev->root_x = dev->event_x;
        dev->root_y = dev->event_y;
        dev->detail = 0;
        for (int d=0; d<GAMEDISPLAYNUM; d++) {
            if (x11::gameDisplays[d]) {
                dev->root = XRootWindow(x11::gameDisplays[d], 0);
                xlibEventQueueList.insert(x11::gameDisplays[d], &event);
            }
        }

        LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate XIEvent XI_Motion");
    }
#endif
}

/* Same as above with the MouseButton event */
static void generateMouseButtonEvent(int button, bool pressed)
{
    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    /* Fill the event structure */
    if (Global::game_info.mouse & GameInfo::SDL2) {
        SDL_Event event2;
        if (pressed) {
            event2.type = SDL_MOUSEBUTTONDOWN;
            event2.button.state = SDL_PRESSED;
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONDOWN with button %d", SingleInput::toSDL2PointerButton(button));
        }
        else {
            event2.type = SDL_MOUSEBUTTONUP;
            event2.button.state = SDL_RELEASED;
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONUP with button %d", SingleInput::toSDL2PointerButton(button));
        }
        event2.button.timestamp = timestamp;
        event2.button.windowID = 1;
        event2.button.which = 0; // TODO: Same as above...
        event2.button.button = SingleInput::toSDL2PointerButton(button);
        event2.button.clicks = 1;
        event2.button.x = Inputs::game_ai.pointer.x;
        event2.button.y = Inputs::game_ai.pointer.y;
        sdlEventQueue.insert(&event2);
    }

    if (Global::game_info.mouse & GameInfo::SDL1) {
        SDL1::SDL_Event event1;
        if (pressed) {
            event1.type = SDL1::SDL_MOUSEBUTTONDOWN;
            event1.button.state = SDL_PRESSED;
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONDOWN with button %d", SingleInput::toSDL1PointerButton(button));
        }
        else {
            event1.type = SDL1::SDL_MOUSEBUTTONUP;
            event1.button.state = SDL_RELEASED;
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEBUTTONUP with button %d", SingleInput::toSDL1PointerButton(button));
        }
        event1.button.which = 0; // TODO: Same as above...
        event1.button.button = SingleInput::toSDL1PointerButton(button);
        event1.button.x = (Uint16) Inputs::game_ai.pointer.x;
        event1.button.y = (Uint16) Inputs::game_ai.pointer.y;
        sdlEventQueue.insert(&event1);
    }

#ifdef __unix__
    if ((Global::game_info.mouse & GameInfo::XEVENTS) && !x11::gameXWindows.empty()) {
        XEvent event;
        if (pressed) {
            event.xbutton.type = ButtonPress;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate Xlib event ButtonPress with button %d", SingleInput::toXlibPointerButton(button));
        }
        else {
            event.xbutton.type = ButtonRelease;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate Xlib event ButtonRelease with button %d", SingleInput::toXlibPointerButton(button));
        }
        event.xbutton.state = SingleInput::toXlibPointerMask(Inputs::game_ai.pointer.mask);
        event.xbutton.x = Inputs::game_ai.pointer.x;
        event.xbutton.y = Inputs::game_ai.pointer.y;
        event.xbutton.x_root = event.xbutton.x;
        event.xbutton.y_root = event.xbutton.y;
        event.xbutton.button = SingleInput::toXlibPointerButton(button);
        if (pointer_grab_window != None)
            event.xbutton.window = pointer_grab_window;
        else
            event.xbutton.window = x11::gameXWindows.front();
        event.xbutton.same_screen = 1;
        event.xbutton.send_event = 0;
        event.xbutton.subwindow = 0;
        event.xbutton.root = x11::rootWindow;

        xlibEventQueueList.insert(&event);
    }

    if ((Global::game_info.mouse & GameInfo::XCBEVENTS) && !x11::gameXWindows.empty()) {
        xcb_button_press_event_t event; // same as xcb_button_release_event_t
        if (pressed) {
            event.response_type = XCB_BUTTON_PRESS;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_BUTTON_PRESS with button %d", SingleInput::toXlibPointerButton(button));
        }
        else {
            event.response_type = XCB_BUTTON_RELEASE;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_BUTTON_RELEASE with button %d", SingleInput::toXlibPointerButton(button));
        }
        event.state = SingleInput::toXlibPointerMask(Inputs::game_ai.pointer.mask);
        event.event_x = Inputs::game_ai.pointer.x;
        event.event_y = Inputs::game_ai.pointer.y;
        event.root_x = Inputs::game_ai.pointer.x;
        event.root_y = Inputs::game_ai.pointer.y;
        event.detail = SingleInput::toXlibPointerButton(button);
        event.event = x11::gameXWindows.front();
        event.same_screen = 1;
        event.child = 0;
        event.root = x11::rootWindow;

        xcbEventQueueList.insert(reinterpret_cast<xcb_generic_event_t*>(&event), false);
    }

    if ((Global::game_info.mouse & GameInfo::XIEVENTS) && !x11::gameXWindows.empty()) {
        XEvent event;
        XIDeviceEvent *dev = static_cast<XIDeviceEvent*>(calloc(1, sizeof(XIDeviceEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        if (pressed) {
            LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_ButtonPress with button %d", button+1);
            event.xcookie.evtype = XI_ButtonPress;
            dev->evtype = XI_ButtonPress;
        }
        else {
            LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_ButtonRelease with button %d", button+1);
            event.xcookie.evtype = XI_ButtonRelease;
            dev->evtype = XI_ButtonRelease;
        }
        event.xcookie.data = dev;
        dev->event = x11::gameXWindows.front();
        dev->time = timestamp;
        dev->deviceid = 2;
        dev->sourceid = 2;
        dev->event_x = Inputs::game_ai.pointer.x;
        dev->event_y = Inputs::game_ai.pointer.y;
        dev->root_x = dev->event_x;
        dev->root_y = dev->event_y;
        dev->detail = button+1;
        dev->buttons.mask = static_cast<unsigned char*>(malloc(1*sizeof(unsigned char)));
        dev->buttons.mask_len = 1;
        for (int bj=0; bj<5; bj++) {
            if (Inputs::game_ai.pointer.mask & (1 << bj)) {
                XISetMask(dev->buttons.mask, bj);
            }
        }
        for (int d=0; d<GAMEDISPLAYNUM; d++) {
            if (x11::gameDisplays[d]) {
                dev->root = XRootWindow(x11::gameDisplays[d], 0);
                xlibEventQueueList.insert(x11::gameDisplays[d], &event);
            }
        }
    }

    if (Global::game_info.mouse & GameInfo::XIRAWEVENTS) {
        XEvent event;
        XIRawEvent *rev = static_cast<XIRawEvent*>(calloc(1, sizeof(XIRawEvent)));
        event.xcookie.type = GenericEvent;
        event.xcookie.extension = xinput_opcode;
        if (pressed) {
            LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_RawButtonPress with button %d", button+1);
            event.xcookie.evtype = XI_RawButtonPress;
            rev->evtype = XI_RawButtonPress;
        }
        else {
            LOG(LL_DEBUG, LCF_EVENTS | LCF_KEYBOARD, "Generate XIEvent XI_RawButtonRelease with button %d", button+1);
            event.xcookie.evtype = XI_RawButtonRelease;
            rev->evtype = XI_RawButtonRelease;
        }
        event.xcookie.data = rev;
        rev->time = timestamp;
        rev->detail = button+1;
        xlibEventQueueList.insert(&event);
    }
#endif
}

/* Same as above with the MouseButton event */
static void generateMouseButtonEvents(void)
{
    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    static int buttons[] = {SingleInput::POINTER_B1,
        SingleInput::POINTER_B2, SingleInput::POINTER_B3,
        SingleInput::POINTER_B4, SingleInput::POINTER_B5};

    for (int bi=0; bi<5; bi++) {
        if ((Inputs::game_ai.pointer.mask ^ Inputs::old_game_ai.pointer.mask) & (1 << buttons[bi])) {
            /* We got a change in a button state */
            generateMouseButtonEvent(buttons[bi], Inputs::game_ai.pointer.mask & (1 << buttons[bi]));
        }
    }
    
    /* Check if we got a change in mouse wheel */
    if (!Inputs::game_ai.pointer.wheel)
        return;

    if (Global::game_info.mouse & GameInfo::SDL2) {
        SDL_Event event2;
        event2.type = SDL_MOUSEWHEEL;
        event2.wheel.timestamp = timestamp;
        event2.wheel.windowID = 1;
        event2.wheel.which = 0; // TODO: Mouse instance id. No idea what to put here...
        event2.wheel.x = 0; // Only vertical wheel is supported
        event2.wheel.y = Inputs::game_ai.pointer.wheel;
        event2.wheel.direction = SDL_MOUSEWHEEL_FLIPPED;
        sdlEventQueue.insert(&event2);
        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_MOUSE, "Generate SDL event MOUSEWHEEL with new value (%d)", Inputs::game_ai.pointer.wheel);
    }
}

/* Generate focus/unfocus event */
static void generateFocusEvents(void)
{
    /* Keep here the status of window focus */
    static bool win_focused = true;
    
    /* Check the focus flag */
    if (!(Inputs::game_ai.misc.flags & (1<<SingleInput::FLAG_FOCUS_UNFOCUS)))
        return;
    
    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    if (Global::game_info.keyboard & GameInfo::SDL2) {
        SDL_Event event2;
        event2.type = SDL_WINDOWEVENT;
        if (win_focused) {
            event2.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_WINDOW, "Generate SDL event SDL_WINDOWEVENT_FOCUS_LOST");
        }
        else {
            event2.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
            LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_WINDOW, "Generate SDL event SDL_WINDOWEVENT_FOCUS_GAINED");                        
        }
        event2.window.timestamp = timestamp;
        event2.window.windowID = 1;
        sdlEventQueue.insert(&event2);
    }

    if (Global::game_info.keyboard & GameInfo::SDL1) {
        SDL1::SDL_Event event1;
        event1.type = SDL1::SDL_ACTIVEEVENT;
        event1.active.gain = !win_focused;
        event1.active.state = SDL1::SDL_APPINPUTFOCUS;
        LOG(LL_DEBUG, LCF_SDL | LCF_EVENTS | LCF_WINDOW, "Generate SDL event SDL_ACTIVEEVENT with state SDL_APPINPUTFOCUS to %d", event1.active.gain);
        sdlEventQueue.insert(&event1);
    }

#ifdef __unix__
    if ((Global::game_info.keyboard & GameInfo::XEVENTS) && !x11::gameXWindows.empty()) {
        XEvent event;
        if (win_focused) {
            event.type = FocusOut;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate Xlib event FocusOut");
        }
        else {
            event.type = FocusIn;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate Xlib event FocusIn");            
        }
        event.xfocus.window = x11::gameXWindows.front();
        event.xfocus.mode = NotifyNormal; // TODO
        event.xfocus.send_event = 0;
        event.xfocus.detail = NotifyDetailNone; // TODO
        xlibEventQueueList.insert(&event);
    }

    if ((Global::game_info.keyboard & GameInfo::XCBEVENTS) && !x11::gameXWindows.empty()) {
        
        if (win_focused) {
            xcb_focus_out_event_t event;
            event.response_type = XCB_FOCUS_OUT;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_FOCUS_OUT");
            event.event = x11::gameXWindows.front();
            xcbEventQueueList.insert(reinterpret_cast<xcb_generic_event_t*>(&event), false);
        }
        else {
            xcb_focus_in_event_t event;
            event.response_type = XCB_FOCUS_IN;
            LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_FOCUS_IN");
            event.event = x11::gameXWindows.front();
            xcbEventQueueList.insert(reinterpret_cast<xcb_generic_event_t*>(&event), false);
        }
    }
#endif

    /* Change state */
    win_focused = !win_focused;
}

static void generateEventList()
{
    for (const auto& event : Inputs::game_ai.events) {
        switch (event.type) {
            case SingleInput::IT_KEYBOARD:
                generateKeyEvent(event.which, event.value);
                break;
            case SingleInput::IT_POINTER_BUTTON:
                generateMouseButtonEvent(event.which, event.value);
                break;
        }
    }
}

void generateInputEvents(void)
{
    /* If we have input events, only handle those */
    if (!Inputs::game_ai.events.empty()) {
        generateEventList();
        return;
    }
    generateKeyUpEvents();
    generateKeyDownEvents();
    generateControllerAdded();
    generateControllerEvents();
    generateMouseMotionEvents();
    generateMouseButtonEvents();
    generateFocusEvents();    
}

void syncControllerEvents()
{
    if (!(Global::shared_config.async_events & (SharedConfig::ASYNC_JSDEV | SharedConfig::ASYNC_EVDEV)))
        return;

    if (!(Global::game_info.joystick & (GameInfo::JSDEV | GameInfo::EVDEV)))
        return;

#ifdef __linux__
    struct timespec time = DeterministicTimer::get().getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    for (int i = 0; i < Global::shared_config.nb_controllers; i++) {
        if (Global::shared_config.async_events & SharedConfig::ASYNC_JSDEV) {
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
        if (Global::shared_config.async_events & SharedConfig::ASYNC_EVDEV) {
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
#endif
}

}
