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

#include "GameEventsXcb.h"
#include "GameEvents.h"
#include "movie/MovieFile.h"

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include <string>
#include <iostream>
#include <cerrno>
#include <unistd.h> // usleep()
#include <stdint.h>

GameEventsXcb::GameEventsXcb(Context* c, MovieFile* m) : GameEvents(c, m), keysyms(xcb_key_symbols_alloc(c->conn), xcb_key_symbols_free) {}

void GameEventsXcb::init()
{
    GameEvents::init();
    
    /* Clear the event queue and parameters */
    xcb_generic_event_t *event;
    do {
        event = xcb_poll_for_event(context->conn);
    } while (event);

    last_pressed_key = 0;
    next_event = nullptr;
}

void GameEventsXcb::registerGameWindow(uint32_t gameWindow)
{
    context->game_window = static_cast<xcb_window_t>(gameWindow);
    if (context->game_window != 0)
    {
        const static uint32_t values[] = { XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_EXPOSURE };
        xcb_void_cookie_t cwa_cookie = xcb_change_window_attributes (context->conn, context->game_window, XCB_CW_EVENT_MASK, values);
        xcb_generic_error_t *error = xcb_request_check(context->conn, cwa_cookie);
        if (error) {
            std::cerr << "error in xcb_change_window_attributes: " << error->error_code << std::endl;
        }

        /* Also get parent window of game window for focus */
        xcb_query_tree_cookie_t qt_cookie = xcb_query_tree(context->conn, context->game_window);
        xcb_query_tree_reply_t *reply = xcb_query_tree_reply(context->conn, qt_cookie, &error);

        if (error) {
            std::cerr << "Could not get xcb_query_tree, X error" << error->error_code << std::endl;
            return;
        }
        else {
            parent_game_window = reply->parent;
        }
        if (reply)
            free(reply);
    }
}

GameEventsXcb::EventType GameEventsXcb::nextEvent(struct HotKey &hk)
{
    while (true) {
        xcb_generic_event_t *event;

        if (next_event) {
            event = next_event;
            next_event = nullptr;
        }
        else {
            event = xcb_poll_for_event(context->conn);
        }

        if (!event) {

            if (!context->hotkey_pressed_queue.empty()) {
                /* Processing a pressed hotkey pushed by the UI */
                context->hotkey_pressed_queue.pop(hk.type);
                return EVENT_TYPE_PRESS;
            }
            else if (!context->hotkey_released_queue.empty()) {
                /* Processing a pressed hotkey pushed by the UI */
                context->hotkey_released_queue.pop(hk.type);
                return EVENT_TYPE_RELEASE;
            }
            else {
                return EVENT_TYPE_NONE;
            }
        }
        else {
            /* Processing a hotkey pressed by the user */
            uint8_t response_type = (event->response_type & ~0x80);

            if ((response_type == XCB_KEY_PRESS) || (response_type == XCB_KEY_RELEASE)) {
                /* Get the actual pressed/released key */
                xcb_key_press_event_t* key_event = reinterpret_cast<xcb_key_press_event_t*>(event);
                xcb_keycode_t kc = key_event->detail;

                /* Detecting auto-repeat, either if detectable auto-repeat is
                 * supported or not by the X server.
                 * If detectable auto-repeat is supported, we detect if pressed
                 * event is from the same key as the last pressed key, without
                 * a key release.
                 * If detectable auto-repeat is not supported, we detect if a
                 * released event is followed by a pressed event with the same
                 * sequence number. If not, we must keep the later event for
                 * the next call (xcb does not support peeking events).
                 */
                if (kc == last_pressed_key) {
                    if (response_type == XCB_KEY_RELEASE) {
                        /* Check the next event. We wait a bit for the next
                         * event to be generated if auto-repeat. There are still
                         * are few cases where we would have to wait a significantly
                         * longer time.
                         */
                        usleep(500);
                        next_event = xcb_poll_for_event(context->conn);
                        xcb_key_press_event_t* next_key_event = reinterpret_cast<xcb_key_press_event_t*>(next_event);

                        if (next_event &&
                           ((next_event->response_type & ~0x80) == XCB_KEY_PRESS) &&
                           (next_key_event->detail == kc) &&
                           ((next_key_event->time - key_event->time) < 5)) {

                            /* Found an auto-repeat sequence, discard both events */
                            free(event);
                            free(next_event);
                            next_event = nullptr;
                            continue;
                        }

                        /* Normal key release */
                        last_pressed_key = 0;
                    }
                    if (response_type == XCB_KEY_PRESS) {
                        /* Auto-repeat, must skip */
                        free(event);
                        continue;
                    }
                }
                if (response_type == XCB_KEY_PRESS) {
                    last_pressed_key = kc;
                }
                
                int key_state = key_event->state;

                free(event);

                /* Get keysym from keycode */
                xcb_keysym_t ks = xcb_key_symbols_get_keysym(keysyms.get(), kc, 0);

                /* If pressed a controller button, update the controller input window */
                if (context->config.km->input_mapping.find(ks) != context->config.km->input_mapping.end()) {
                    SingleInput si = context->config.km->input_mapping[ks];
                    if (si.inputTypeIsController())
                        emit controllerButtonToggled(si.inputTypeToControllerNumber(), si.inputTypeToInputNumber(), response_type == XCB_KEY_PRESS);
                }

                /* If the key is a modifier, skip it */
                if (context->config.km->is_modifier(ks))
                    continue;

                /* Build the modifier value */
                xcb_keysym_t modifiers = context->config.km->get_modifiers(key_state);

                /* Check if this KeySym with or without modifiers is mapped to a hotkey */
                if (context->config.km->hotkey_mapping.find(ks | modifiers) != context->config.km->hotkey_mapping.end()) {
                    hk = context->config.km->hotkey_mapping[ks | modifiers];
                    return (response_type==XCB_KEY_PRESS)?EVENT_TYPE_PRESS:EVENT_TYPE_RELEASE;
                }
                else if (context->config.km->hotkey_mapping.find(ks) != context->config.km->hotkey_mapping.end()) {
                    hk = context->config.km->hotkey_mapping[ks];
                    return (response_type==XCB_KEY_PRESS)?EVENT_TYPE_PRESS:EVENT_TYPE_RELEASE;
                }
                else
                    /* This input is not a hotkey, skipping to the next */
                    continue;
            }
            else {
                free(event);
                switch (response_type) {
                    case XCB_FOCUS_OUT:
                        return EVENT_TYPE_FOCUS_OUT;
                    case XCB_EXPOSE:
                        return EVENT_TYPE_EXPOSE;
                    default:
                        return EVENT_TYPE_NONE;
                }
            }
        }
    }
    return EVENT_TYPE_NONE;
}

bool GameEventsXcb::haveFocus()
{
    xcb_window_t window;

    xcb_generic_error_t* error;
    xcb_get_input_focus_cookie_t focus_cookie = xcb_get_input_focus(context->conn);
    xcb_get_input_focus_reply_t* focus_reply = xcb_get_input_focus_reply(context->conn, focus_cookie, &error);
    if (error) {
        std::cerr << "Could not get focussed window, X error" << error->error_code << std::endl;
    }

    window = focus_reply->focus;
    free(focus_reply);

    return (window == context->game_window) || (window == parent_game_window);
}
