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
#include "GameEvents.h"
// #include "GameThread.h"
#include "utils.h"
// #include "AutoSave.h"
#include "SaveState.h"
#include "SaveStateList.h"
#include "movie/MovieFile.h"

#include "../shared/sockethelpers.h"
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include <string>
#include <iostream>
#include <cerrno>
#include <unistd.h> // usleep()
#include <future>
// #include <csignal> // kill
// #include <sys/stat.h> // stat
// #include <sys/wait.h> // waitpid
// #include <X11/X.h>
#include <stdint.h>

GameEvents::GameEvents(Context* c, MovieFile* m) : context(c), movie(m), keysyms(xcb_key_symbols_alloc(c->conn), xcb_key_symbols_free) {}

void GameEvents::init()
{
    clearEventQueue();
    
    ar_ticks = -1;
    ar_delay = 50;
    ar_freq = 2;
}

void GameEvents::clearEventQueue()
{
    /* Clear the event queue and parameters */
    xcb_generic_event_t *event;
    do {
        event = xcb_poll_for_event(context->conn);
    } while (event);

    last_pressed_key = 0;
    next_event = nullptr;
}

void GameEvents::registerGameWindow(uint32_t gameWindow)
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

GameEvents::EventType GameEvents::nextEvent(struct HotKey &hk)
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

                free(event);

                /* Get keysym from keycode */
                xcb_keysym_t ks = xcb_key_symbols_get_keysym(keysyms.get(), kc, 0);

                /* If pressed a controller button, update the controller input window */
                notifyControllerEvent(ks, response_type == XCB_KEY_PRESS);

                /* If the key is a modifier, skip it */
                if (is_modifier(ks))
                    continue;

                /* Build the modifier value */
                xcb_generic_error_t* error;
                xcb_query_keymap_cookie_t keymap_cookie = xcb_query_keymap(context->conn);
                xcb_query_keymap_reply_t* keymap_reply = xcb_query_keymap_reply(context->conn, keymap_cookie, &error);

                xcb_keysym_t modifiers = 0;
                if (error) {
                    std::cerr << "Could not get xcb_query_keymap, X error" << error->error_code << std::endl;
                }
                else {
                    modifiers = build_modifiers(keymap_reply->keys, keysyms.get());
                }
                free(keymap_reply);

                /* Check if this KeySym with or without modifiers is mapped to a hotkey */
                if (context->config.km.hotkey_mapping.find(ks | modifiers) != context->config.km.hotkey_mapping.end()) {
                    hk = context->config.km.hotkey_mapping[ks | modifiers];
                    return (response_type==XCB_KEY_PRESS)?EVENT_TYPE_PRESS:EVENT_TYPE_RELEASE;
                }
                else if (context->config.km.hotkey_mapping.find(ks) != context->config.km.hotkey_mapping.end()) {
                    hk = context->config.km.hotkey_mapping[ks];
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

void GameEvents::notifyControllerEvent(xcb_keysym_t ks, bool pressed)
{
    if (context->config.km.input_mapping.find(ks) != context->config.km.input_mapping.end()) {
        SingleInput si = context->config.km.input_mapping[ks];
        if (si.inputTypeIsController())
            emit controllerButtonToggled(si.inputTypeToControllerNumber(), si.inputTypeToInputNumber(), pressed);
    }
}

bool GameEvents::processEvent(GameEvents::EventType type, struct HotKey &hk)
{
    switch (type) {

    case EVENT_TYPE_FOCUS_OUT:
        ar_ticks = -1; // Deactivate auto-repeat
        return false;

    case EVENT_TYPE_EXPOSE:
        /* Send an expose message to the game so that he can redrawn the screen */
        if (!context->config.sc.running)
            sendMessage(MSGN_EXPOSE);
        return false;

    case EVENT_TYPE_PRESS:

        switch(hk.type) {

        case HOTKEY_FRAMEADVANCE:
            /* Advance a frame */
            if (context->config.sc.running) {
                context->config.sc.running = false;
                emit sharedConfigChanged();
                context->config.sc_modified = true;
            }
            ar_ticks = 0; // Activate auto-repeat
            return true;

        case HOTKEY_PLAYPAUSE:
            /* Toggle between play and pause */
            context->config.sc.running = !context->config.sc.running;
            emit sharedConfigChanged();
            context->config.sc_modified = true;
            return false;

        case HOTKEY_FASTFORWARD:
            /* Enable fastforward */
            context->config.sc.fastforward = true;
            emit sharedConfigChanged();
            context->config.sc_modified = true;

            /* Make frame advance auto-repeat faster */
            ar_freq = 1;

            return false;

        case HOTKEY_SAVESTATE1:
        case HOTKEY_SAVESTATE2:
        case HOTKEY_SAVESTATE3:
        case HOTKEY_SAVESTATE4:
        case HOTKEY_SAVESTATE5:
        case HOTKEY_SAVESTATE6:
        case HOTKEY_SAVESTATE7:
        case HOTKEY_SAVESTATE8:
        case HOTKEY_SAVESTATE9:
        case HOTKEY_SAVESTATE_BACKTRACK:
        {
            /* Perform a savestate:
             * - save the moviefile if we are recording
             * - tell the game to save its state
             */

            /* Saving is not allowed if currently encoding */
            if (context->config.sc.av_dumping) {
                emit alertToShow(QString("Saving is not allowed when in the middle of video encoding"));
                return false;
            }

            /* Slot number */
            int statei = hk.type - HOTKEY_SAVESTATE1 + 1;

            /* Perform savestate */
            int message = SaveStateList::save(statei, context, *movie);

            /* Checking that saving succeeded */
            if (message == MSGB_SAVING_SUCCEEDED) {
                emit savestatePerformed(statei, context->framecount);
            }

            return false;
        }

        case HOTKEY_LOADSTATE1:
        case HOTKEY_LOADSTATE2:
        case HOTKEY_LOADSTATE3:
        case HOTKEY_LOADSTATE4:
        case HOTKEY_LOADSTATE5:
        case HOTKEY_LOADSTATE6:
        case HOTKEY_LOADSTATE7:
        case HOTKEY_LOADSTATE8:
        case HOTKEY_LOADSTATE9:
        case HOTKEY_LOADSTATE_BACKTRACK:
        case HOTKEY_LOADBRANCH1:
        case HOTKEY_LOADBRANCH2:
        case HOTKEY_LOADBRANCH3:
        case HOTKEY_LOADBRANCH4:
        case HOTKEY_LOADBRANCH5:
        case HOTKEY_LOADBRANCH6:
        case HOTKEY_LOADBRANCH7:
        case HOTKEY_LOADBRANCH8:
        case HOTKEY_LOADBRANCH9:
        case HOTKEY_LOADBRANCH_BACKTRACK:

            /* Load a savestate:
             * - check for an existing savestate in the slot
             * - if in read-only move, we must check that the movie
                 associated with the savestate must be a prefix of the
                 current movie
             * - tell the game to load its state
             * - if loading succeeded:
             * -- send the shared config
             * -- increment the rerecord count
             * -- receive the frame count and the current time
             */
        {

            /* Loading is not allowed if currently encoding */
            if (context->config.sc.av_dumping) {
                emit alertToShow(QString("Loading is not allowed when in the middle of video encoding"));
                return false;
            }

            /* Loading branch? */
            bool load_branch = (hk.type >= HOTKEY_LOADBRANCH1) && (hk.type <= HOTKEY_LOADBRANCH_BACKTRACK);

            /* Slot number */
            int statei = hk.type - (load_branch?HOTKEY_LOADBRANCH1:HOTKEY_LOADSTATE1) + 1;

            /* Perform state loading */
            int error = SaveStateList::load(statei, context, *movie, load_branch);

            /* Handle errors */
            if (error == SaveState::ENOSTATEMOVIEPREFIX) {
                /* Ask the user if they want to load the movie, and get the answer.
                 * Prompting a alert window must be done by the UI thread, so we are
                 * using std::future/std::promise mechanism.
                 */
                std::promise<bool> answer;
                std::future<bool> future = answer.get_future();
                emit askToShow(QString("There is a savestate in that slot from a previous game iteration. Do you want to load the associated movie?"), &answer);

                if (! future.get()) {
                    /* User answered no */
                    return false;
                }

                /* Loading the movie */
                emit inputsToBeChanged();
                movie->loadSavestateMovie(SaveStateList::get(statei).getMoviePath());
                emit inputsChanged();

                /* Return if we already are on the correct frame */
                if (context->framecount == movie->header->savestate_framecount)
                    return false;

                /* Fast-forward to savestate frame */
                context->config.sc.recording = SharedConfig::RECORDING_READ;
                context->config.sc.movie_framecount = movie->inputs->nbFrames();
                context->movie_time_sec = movie->header->length_sec;
                context->movie_time_nsec = movie->header->length_nsec;
                context->pause_frame = movie->header->savestate_framecount;
                context->config.sc.running = true;
                context->config.sc_modified = true;

                emit sharedConfigChanged();

                return false;
            }

            if (error == SaveState::ENOSTATE) {
                if (!(context->config.sc.osd & SharedConfig::OSD_MESSAGES))
                    emit alertToShow(QString("There is no savestate to load in this slot"));
                return false;
            }

            if (error == SaveState::ENOMOVIE) {
                emit alertToShow(QString("Could not load the moviefile associated with the savestate"));
                return false;                
            }

            if (error == SaveState::EINPUTMISMATCH) {
                if (!(context->config.sc.osd & SharedConfig::OSD_MESSAGES)) {
                    emit alertToShow(QString("Trying to load a state in read-only but the inputs mismatch"));
                }
                return false;                
            }

            emit inputsToBeChanged();

            /* Processing after state loading */
            int message = SaveStateList::postLoad(statei, context, *movie, load_branch);

            /* Handle errors and return values */
            if (message == SaveState::ENOLOAD) {
                if (!context->config.sc.opengl_soft) {
                    emit alertToShow(QString("Crash after loading the savestate. Savestates are unstable unless you check Video>Force software rendering"));
                }

                return false;
            }

            if (message == MSGB_LOADING_SUCCEEDED) {
                emit savestatePerformed(statei, 0);
            }

            emit inputsChanged();

            return false;
        }

        case HOTKEY_READWRITE:
            /* Switch between movie write and read-only */
            switch (context->config.sc.recording) {
            case SharedConfig::RECORDING_WRITE:
                context->config.sc.recording = SharedConfig::RECORDING_READ;
                context->config.sc.movie_framecount = movie->inputs->nbFrames();
                {
                    std::string msg = "Switched to playback mode";
                    sendMessage(MSGN_OSD_MSG);
                    sendString(msg);
                }
                break;
            case SharedConfig::RECORDING_READ:
                /* Check if we reached the end of the movie already. */
                if (context->framecount > context->config.sc.movie_framecount) {
                    emit alertToShow(QString("Cannot write to a movie after its end"));
                }
                else {
                    emit inputsToBeChanged();
                    context->config.sc.recording = SharedConfig::RECORDING_WRITE;
                    emit inputsChanged();
                    {
                        std::string msg = "Switched to recording mode";
                        sendMessage(MSGN_OSD_MSG);
                        sendString(msg);
                    }
                }
                break;
            default:
                break;
            }
            context->config.sc_modified = true;
            emit sharedConfigChanged();
            return false;

        /* Start or stop a video encode */
        case HOTKEY_TOGGLE_ENCODE:
            if (!context->config.sc.av_dumping) {

                context->config.sc.av_dumping = true;
                context->config.sc_modified = true;
                context->config.dumpfile_modified = true;
            }
            else {
                context->config.sc.av_dumping = false;
                context->config.sc_modified = true;

                /* Tells the game to immediately stop the encode,
                 * so we don't have to advance a frame. This also
                 * allows to start a new encode on the same frame
                 */
                sendMessage(MSGN_STOP_ENCODE);
            }
            emit sharedConfigChanged();
            return false;

        } /* switch(hk.type) */
        break;

    case EVENT_TYPE_RELEASE:

        switch (hk.type) {
        case HOTKEY_FASTFORWARD:
            context->config.sc.fastforward = false;
            emit sharedConfigChanged();
            context->config.sc_modified = true;

            /* Recover normal frame-advance auto-repeat */
            ar_freq = 1;

            return false;
        case HOTKEY_FRAMEADVANCE:
            ar_ticks = -1; // Deactivate auto-repeat
            return false;
        }
    default:
        return false;
    } /* switch (type) */
    return false;
}

bool GameEvents::handleEvent()
{
    /* Implement frame-advance auto-repeat */
    bool ar_advance = false;
    if (ar_ticks >= 0) {
        ar_ticks++;
        if ((ar_ticks > ar_delay) && !(ar_ticks % ar_freq))
            /* Trigger auto-repeat */
            ar_advance = true;
    }

    struct HotKey hk;
    EventType eventType = nextEvent(hk);

    if (eventType) {
        return ar_advance | processEvent(eventType, hk);
    }
    return ar_advance;
}

bool GameEvents::haveFocus()
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
