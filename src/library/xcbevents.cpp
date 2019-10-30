/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "xcbevents.h"
#include "logging.h"
#include "hook.h"
#include "XcbEventQueueList.h"
#include "xatom.h"

#ifdef LIBTAS_HAS_XCB_RANDR
#include <xcb/randr.h>
#endif

namespace libtas {

DEFINE_ORIG_POINTER(xcb_wait_for_event);
DEFINE_ORIG_POINTER(xcb_poll_for_event);
DEFINE_ORIG_POINTER(xcb_send_event_checked);
DEFINE_ORIG_POINTER(xcb_send_event);
DEFINE_ORIG_POINTER(xcb_flush);
#ifdef LIBTAS_HAS_XCB_RANDR
DEFINE_ORIG_POINTER(xcb_randr_get_screen_info_unchecked);
DEFINE_ORIG_POINTER(xcb_randr_get_screen_info_reply);
DEFINE_ORIG_POINTER(xcb_randr_get_screen_info_sizes);
#endif

/* Function to indicate if an event is filtered */
static bool isEventFiltered (xcb_generic_event_t *event) {
    switch (event->response_type) {
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
        case XCB_MOTION_NOTIFY:
        case XCB_ENTER_NOTIFY:
        case XCB_LEAVE_NOTIFY:
        case XCB_FOCUS_IN:
        case XCB_FOCUS_OUT:
        case XCB_KEYMAP_NOTIFY:
        case XCB_EXPOSE:
        // case XCB_GRAPHICS_EXPOSURE:
        case XCB_NO_EXPOSURE:
        case XCB_VISIBILITY_NOTIFY:
            return true;
        default:
            return False;
    }
}

void pushNativeXcbEvents(void)
{
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    if (!(game_info.keyboard & GameInfo::XCBEVENTS)) {
        return;
    }

    for (int i=0; i<GAMECONNECTIONNUM; i++)
        if (gameConnections[i])
            pushNativeXcbEvents(gameConnections[i]);
}

void pushNativeXcbEvents(xcb_connection_t *c)
{
    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        return;
    }

    if (!(game_info.keyboard & GameInfo::XCBEVENTS)) {
        return;
    }

    GlobalNative gn;

    xcb_generic_event_t *event;

    while ((event = xcb_poll_for_event (c))) {

        if (event->response_type == XCB_CLIENT_MESSAGE) {
            /* Catch the close event */
            xcb_client_message_event_t* client_event = reinterpret_cast<xcb_client_message_event_t*>(event);

            if (static_cast<Atom>(client_event->data.data32[0]) == x11_atom(WM_DELETE_WINDOW)) {
                debuglog(LCF_EVENTS | LCF_WINDOW, "    caught a window close event");
                is_exiting = true;
            }

            /* Catch a ping event */
            if ((client_event->type == x11_atom(WM_PROTOCOLS)) &&
                (static_cast<Atom>(client_event->data.data32[0]) == x11_atom(_NET_WM_PING))) {

                debuglog(LCF_EVENTS | LCF_WINDOW, "Answering a ping message");
                xcb_client_message_event_t reply = *client_event;
                xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (c));
                xcb_screen_t* screen = iter.data;
                reply.window = screen->root;

                xcb_send_event(c, false, screen->root,
XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
reinterpret_cast<char*>(&reply));
            }
        }

        if (!isEventFiltered(event)) {
            xcbEventQueueList.insert(c, event);
        }
    }
}

xcb_generic_event_t *xcb_wait_for_event(xcb_connection_t *c)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(xcb_wait_for_event);
        return orig::xcb_wait_for_event(c);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(xcb_wait_for_event);
        return orig::xcb_wait_for_event(c);
    }

    if (!(game_info.keyboard & GameInfo::XCBEVENTS)) {
        LINK_NAMESPACE_GLOBAL(xcb_wait_for_event);
        return orig::xcb_wait_for_event(c);
    }

    xcb_generic_event_t* event = nullptr;
    std::shared_ptr<XcbEventQueue> queue = xcbEventQueueList.getQueue(c);
    while (true) {
        event = queue->pop();
        if (event)
            break;
        struct timespec st = {0, 1000*1000};
        NATIVECALL(nanosleep(&st, NULL)); // Wait 1 ms before trying again
        pushNativeXcbEvents(c);
    }
    if (!event) {
        debuglog(LCF_EVENTS | LCF_ERROR, "    waited too long for an event");
    }
    return event;
}

xcb_generic_event_t *xcb_poll_for_event(xcb_connection_t *c)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(xcb_poll_for_event);
        return orig::xcb_poll_for_event(c);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(xcb_poll_for_event);
        return orig::xcb_poll_for_event(c);
    }

    if (!(game_info.keyboard & GameInfo::XCBEVENTS)) {
        LINK_NAMESPACE_GLOBAL(xcb_poll_for_event);
        return orig::xcb_poll_for_event(c);
    }

    std::shared_ptr<XcbEventQueue> queue = xcbEventQueueList.getQueue(c);
    return queue->pop();
}

xcb_void_cookie_t
xcb_send_event_checked (xcb_connection_t *c,
                        uint8_t           propagate,
                        xcb_window_t      destination,
                        uint32_t          event_mask,
                        const char       *event)
{
    LINK_NAMESPACE_GLOBAL(xcb_send_event_checked);

    if (GlobalState::isNative())
        return orig::xcb_send_event_checked(c, propagate, destination, event_mask, event);

    DEBUGLOGCALL(LCF_EVENTS);

    const xcb_generic_event_t* ev = reinterpret_cast<const xcb_generic_event_t*> (event);
    xcb_void_cookie_t cookie{0};

    /* Detect and disable several window state changes */
    if (ev->response_type == XCB_CLIENT_MESSAGE) {
        const xcb_client_message_event_t* client_event = reinterpret_cast<const xcb_client_message_event_t*>(event);

        if ((client_event->type == x11_atom(_NET_WM_STATE)) &&
            (client_event->data.data32[0] == 1 /*_NET_WM_STATE_ADD*/ )) {

            /* Detect and disable fullscreen switching */
            if (static_cast<Atom>(client_event->data.data32[1]) == x11_atom(_NET_WM_STATE_FULLSCREEN)) {
                debuglog(LCF_EVENTS | LCF_WINDOW, "   prevented fullscreen switching but resized the window");
                if (!gameXWindows.empty() && (client_event->window != gameXWindows.front())) {
                    debuglog(LCF_EVENTS | LCF_WINDOW | LCF_WARNING, "   fullscreen window is not game window!");
                }

                /* Resize the window to the screen or fake resolution */
                if (shared_config.screen_width) {
                    const static uint32_t values[] = { shared_config.screen_width, shared_config.screen_height };
                    xcb_configure_window (c, client_event->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                }
                else {
#ifdef LIBTAS_HAS_XCB_RANDR
                    /* Change the window size to monitor size */
                    LINK_NAMESPACE(xcb_randr_get_screen_info_unchecked, "xcb-randr");
                    LINK_NAMESPACE(xcb_randr_get_screen_info_reply, "xcb-randr");
                    LINK_NAMESPACE(xcb_randr_get_screen_info_sizes, "xcb-randr");

                    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (c));
                    xcb_screen_t* screen = iter.data;

                    xcb_randr_get_screen_info_cookie_t screen_info = orig::xcb_randr_get_screen_info_unchecked(c, screen->root);
                    xcb_randr_get_screen_info_reply_t *reply = orig::xcb_randr_get_screen_info_reply(c, screen_info, nullptr);
                    xcb_randr_screen_size_t *sizes = orig::xcb_randr_get_screen_info_sizes(reply);
                    const static uint32_t values[] = { sizes[0].width, sizes[0].height };
                    xcb_configure_window (c, client_event->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
#endif
                }
                return cookie;
            }

            /* Detect and disable window always on top */
            if (static_cast<Atom>(client_event->data.data32[1]) == x11_atom(_NET_WM_STATE_ABOVE)) {
                debuglog(LCF_EVENTS | LCF_WINDOW, "   prevented window always on top");
                return cookie;
            }
        }
    }

    return orig::xcb_send_event_checked(c, propagate, destination, event_mask, event);
}

xcb_void_cookie_t
xcb_send_event (xcb_connection_t *c,
                uint8_t           propagate,
                xcb_window_t      destination,
                uint32_t          event_mask,
                const char       *event)
{
    LINK_NAMESPACE_GLOBAL(xcb_send_event);

    if (GlobalState::isNative())
        return orig::xcb_send_event(c, propagate, destination, event_mask, event);

    DEBUGLOGCALL(LCF_EVENTS);

    const xcb_generic_event_t* ev = reinterpret_cast<const xcb_generic_event_t*> (event);
    xcb_void_cookie_t cookie{0};

    /* Detect and disable several window state changes */
    if (ev->response_type == XCB_CLIENT_MESSAGE) {
        const xcb_client_message_event_t* client_event = reinterpret_cast<const xcb_client_message_event_t*>(event);

        if ((client_event->type == x11_atom(_NET_WM_STATE)) &&
            (client_event->data.data32[0] == 1 /*_NET_WM_STATE_ADD*/ )) {

            /* Detect and disable fullscreen switching */
            if (static_cast<Atom>(client_event->data.data32[1]) == x11_atom(_NET_WM_STATE_FULLSCREEN)) {
                debuglog(LCF_EVENTS | LCF_WINDOW, "   prevented fullscreen switching but resized the window");
                if (!gameXWindows.empty() && (client_event->window != gameXWindows.front())) {
                    debuglog(LCF_EVENTS | LCF_WINDOW | LCF_WARNING, "   fullscreen window is not game window!");
                }

                /* Resize the window to the screen or fake resolution */
                if (shared_config.screen_width) {
                    const static uint32_t values[] = { shared_config.screen_width, shared_config.screen_height };
                    xcb_configure_window (c, client_event->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                }
                else {
#ifdef LIBTAS_HAS_XCB_RANDR
                    /* Change the window size to monitor size */
                    LINK_NAMESPACE(xcb_randr_get_screen_info_unchecked, "xcb-randr");
                    LINK_NAMESPACE(xcb_randr_get_screen_info_reply, "xcb-randr");
                    LINK_NAMESPACE(xcb_randr_get_screen_info_sizes, "xcb-randr");

                    xcb_screen_iterator_t iter = xcb_setup_roots_iterator (xcb_get_setup (c));
                    xcb_screen_t* screen = iter.data;

                    xcb_randr_get_screen_info_cookie_t screen_info = orig::xcb_randr_get_screen_info_unchecked(c, screen->root);
                    xcb_randr_get_screen_info_reply_t *reply = orig::xcb_randr_get_screen_info_reply(c, screen_info, nullptr);
                    xcb_randr_screen_size_t *sizes = orig::xcb_randr_get_screen_info_sizes(reply);
                    const static uint32_t values[] = { sizes[0].width, sizes[0].height };
                    xcb_configure_window (c, client_event->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
#endif
                }
                return cookie;
            }

            /* Detect and disable window always on top */
            if (static_cast<Atom>(client_event->data.data32[1]) == x11_atom(_NET_WM_STATE_ABOVE)) {
                debuglog(LCF_EVENTS | LCF_WINDOW, "   prevented window always on top");
                return cookie;
            }
        }
    }

    return orig::xcb_send_event(c, propagate, destination, event_mask, event);
}

int xcb_flush(xcb_connection_t *c)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(xcb_flush);
        return orig::xcb_flush(c);
    }

    DEBUGLOGCALL(LCF_EVENTS);

    if (shared_config.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS) {
        LINK_NAMESPACE_GLOBAL(xcb_flush);
        return orig::xcb_flush(c);
    }

    if (!(game_info.keyboard & GameInfo::XCBEVENTS)) {
        LINK_NAMESPACE_GLOBAL(xcb_flush);
        return orig::xcb_flush(c);
    }

    pushNativeXcbEvents(c);
    return 1; // Success
}

}
