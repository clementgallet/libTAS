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

#include "xcbrequests.h"
#include "XcbEventQueueList.h"
#include "XcbEventQueue.h"
#include "inputs/inputs.h"

#include "hook.h"
#include "GlobalState.h"
#include "logging.h"
#include "DeterministicTimer.h"
#include "screencapture/ScreenCapture.h"
#include "xlib/xatom.h"
#include "xlib/xwindows.h" // x11::gameXWindows
#include "global.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

#include <functional>

#ifdef LIBTAS_HAS_XCB_RANDR
#include <xcb/randr.h>
#endif

namespace libtas {

DEFINE_ORIG_POINTER(xcb_send_request)
DEFINE_ORIG_POINTER(xcb_send_request_with_fds)
DEFINE_ORIG_POINTER(xcb_send_request64)
DEFINE_ORIG_POINTER(xcb_send_request_with_fds64)

// ruffle uses winit in order to create the x11 window
// winit uses x11rb, which uses xcb internally
// x11rb does not use xcb_create_window and friends
// rather, it uses xcb_send_request64 with XCB_REQUEST_RAW
// due to this, we need to hook onto xcb_send_request and friends

static void sendXWindow(xcb_window_t w)
{
    uint32_t i = (uint32_t)w;
    lockSocket();
    sendMessage(MSGB_WINDOW_ID);
    sendData(&i, sizeof(i));
    unlockSocket();
    LOG(LL_DEBUG, LCF_WINDOW, "Sent X11 window id %d", w);
}

// the handling here is assuming some relatively "standard" vectors sent
// i.e. main struct, padding to next uint32_t (possibly omitted if already uint32_t aligned), variable length list (if present)
static void handleRawRequest(xcb_connection_t* c, struct iovec* vector, std::function<void(struct iovec*)> send_request)
{
    // check the request opcode, it's always the first byte of the request
    auto major_opcode = *static_cast<uint8_t*>(vector->iov_base);
    switch (major_opcode)
    {
        case XCB_CREATE_WINDOW:
        {
            const auto* req = static_cast<const xcb_create_window_request_t*>(vector->iov_base);
            LOG(LL_TRACE, LCF_WINDOW, "XCB_CREATE_WINDOW raw request with id %d and dimensions %d x %d", req->wid, req->width, req->height);

            constexpr size_t padding_len = -sizeof(xcb_create_window_request_t) & 3;
            const uint32_t* values = reinterpret_cast<const uint32_t*>(vector[1].iov_len == padding_len ? vector[2].iov_base : vector[1].iov_base);

            send_request(vector);

            LOG(LL_DEBUG, LCF_KEYBOARD, "   selecting xcb keyboard events");
            LOG(LL_DEBUG, LCF_MOUSE, "   selecting xcb mouse events");
            if (req->width != 1 || req->height != 1) {
                Global::game_info.keyboard |= GameInfo::XCBEVENTS;
                Global::game_info.mouse |= GameInfo::XCBEVENTS;
                Global::game_info.tosend = true;
            }

            /* Add the mask in our event queue */
            if (req->value_mask & XCB_CW_EVENT_MASK) {
                /* Get the event mask value */
                uint32_t event_mask = 0;
                int i = 0;
                for (int k = 1; k <= XCB_CW_CURSOR; k <<= 1) {
                    if (k == XCB_CW_EVENT_MASK) {
                        event_mask = values[i];
                        break;
                    }

                    if (req->value_mask & k)
                        i++;
                }

                std::shared_ptr<XcbEventQueue> queue = xcbEventQueueList.getQueue(c);
                queue->setMask(req->wid, event_mask);
            }

            /* Only save the Window identifier for top-level windows */
            xcb_screen_t* s = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

            if (s->root == req->parent) {
                /* Saving top-level window */
                if (x11::gameXWindows.empty())
                    LOG(LL_DEBUG, LCF_WINDOW, "   set game window to %d", req->wid);
                x11::gameXWindows.push_back(req->wid);
            }

            break;
        }

        case XCB_DESTROY_WINDOW:
        {
            const auto* req = static_cast<const xcb_destroy_window_request_t*>(vector->iov_base);
            LOG(LL_TRACE, LCF_WINDOW, "XCB_DESTROY_WINDOW raw request with window %d", req->window);

            /* If current game window, switch to another one on the list */
            if (!x11::gameXWindows.empty() && req->window == x11::gameXWindows.front()) {
                ScreenCapture::fini();

                x11::gameXWindows.pop_front();
                if (x11::gameXWindows.empty()) {
                    /* Tells the program we don't have a window anymore to gather inputs */
                    sendXWindow(0);
                }
                else {
                    /* Switch to the next game window */
                    LOG(LL_DEBUG, LCF_WINDOW, "   set game window to %d", x11::gameXWindows.front());
                    sendXWindow(x11::gameXWindows.front());
                }
            }
            else {
                /* If another game window, remove it from the list */
                for (auto iter = x11::gameXWindows.begin(); iter != x11::gameXWindows.end(); iter++) {
                    if (req->window == *iter) {
                        x11::gameXWindows.erase(iter);
                        break;
                    }
                }
            }

            send_request(vector);
            break;
        }

        case XCB_MAP_WINDOW:
        {
            const auto* req = static_cast<const xcb_map_window_request_t*>(vector->iov_base);
            LOG(LL_TRACE, LCF_WINDOW, "XCB_MAP_WINDOW raw request called with window %d", req->window);

            send_request(vector);

            /* We must wait until the window is mapped to send it to the program.
             * We are checking the content of x11::gameXWindows to see if we must send it */
            for (auto iter = x11::gameXWindows.begin(); iter != x11::gameXWindows.end(); iter++) {
                if (req->window == *iter) {
                    x11::gameXWindows.erase(iter);
                    x11::gameXWindows.push_front(req->window);
                    sendXWindow(req->window);
                    break;
                }
            }

            break;
        }

        case XCB_UNMAP_WINDOW:
        {
            const auto* req = static_cast<const xcb_unmap_window_request_t*>(vector->iov_base);
            LOG(LL_TRACE, LCF_WINDOW, "XCB_UNMAP_WINDOW raw request called with window %d", req->window);
            send_request(vector);
            break;
        }

        case XCB_CONFIGURE_WINDOW:
        {
            const auto* req = static_cast<const xcb_configure_window_request_t*>(vector->iov_base);
            LOG(LL_TRACE, LCF_WINDOW, "XCB_CONFIGURE_WINDOW raw request called with window %d", req->window);

            constexpr size_t padding_len = -sizeof(xcb_configure_window_request_t) & 3;
            const uint32_t* value_list = reinterpret_cast<const uint32_t*>(vector[1].iov_len == padding_len ? vector[2].iov_base : vector[1].iov_base);

            /* Disable window movement and get new size */

            /* We need to create a new vector to store our new list */
            uint8_t new_vector_buffer[sizeof(xcb_configure_window_request_t) + padding_len + sizeof(uint32_t) * 16] {};

            /* Dumb xcb detail, 2 dummy vectors need to be allocated before the actual vector */
            struct iovec new_vector[5] {};
            new_vector[2].iov_base = new_vector_buffer;
            new_vector[2].iov_len = sizeof(xcb_configure_window_request_t);
            new_vector[3].iov_base = &new_vector_buffer[sizeof(xcb_configure_window_request_t)];
            new_vector[3].iov_len = padding_len;
            new_vector[4].iov_base = &new_vector_buffer[sizeof(xcb_configure_window_request_t) + padding_len];
            new_vector[4].iov_len = 0;

            auto* new_req = static_cast<xcb_configure_window_request_t*>(new_vector[2].iov_base);
            memcpy(new_req, req, sizeof(xcb_configure_window_request_t));
            new_req->length = (sizeof(new_vector_buffer) / sizeof(uint32_t)) - 16;

            uint32_t new_width = 0, new_height = 0;
            uint32_t* new_list = reinterpret_cast<uint32_t*>(new_vector[4].iov_base);

            if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == new_req->window)) {
                /* Search through the value list */
                int index = 0;
                int new_index = 0;
                for (int v = 0; v < 16; v++) {
                    uint16_t mask = (1 << v);
                    if (!(new_req->value_mask & mask))
                        continue;

                    /* Don't preserve window movement settings */
                    if ((mask == XCB_CONFIG_WINDOW_X) || (mask == XCB_CONFIG_WINDOW_Y)) {
                        index++;
                        continue;
                    }

                    if (mask == XCB_CONFIG_WINDOW_WIDTH)
                        new_width = value_list[index];

                    if (mask == XCB_CONFIG_WINDOW_HEIGHT)
                        new_height = value_list[index];

                    new_list[new_index] = value_list[index];
                    new_index++;
                    index++;
                }

                new_req->value_mask &= ~(XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT);
                new_req->length += new_index;
                new_vector[4].iov_len = new_index * 4;
            }

            send_request(&new_vector[2]);

            /* Check if size has changed */
            if (new_width && new_height) {
                LOG(LL_DEBUG, LCF_WINDOW, "    New size: %d x %d", new_width, new_height);
                ScreenCapture::resize(new_width, new_height);
            }

            break;
        }

        case XCB_SEND_EVENT:
        {
            const auto* req = static_cast<const xcb_send_event_request_t*>(vector->iov_base);
            LOG(LL_TRACE, LCF_EVENTS, "XCB_SEND_EVENT raw request call");

            const auto* ev = reinterpret_cast<const xcb_generic_event_t*>(req->event);

            /* Detect and disable several window state changes */
            if (ev->response_type == XCB_CLIENT_MESSAGE) {
                const auto* client_event = reinterpret_cast<const xcb_client_message_event_t*>(req->event);

                if ((client_event->type == x11_atom(_NET_WM_STATE)) &&
                    (client_event->data.data32[0] == 1 /*_NET_WM_STATE_ADD*/ )) {

                    /* Detect and disable fullscreen switching */
                    if (static_cast<Atom>(client_event->data.data32[1]) == x11_atom(_NET_WM_STATE_FULLSCREEN)) {
                        LOG(LL_DEBUG, LCF_EVENTS | LCF_WINDOW, "   prevented fullscreen switching but resized the window");
                        if (!x11::gameXWindows.empty() && (client_event->window != x11::gameXWindows.front())) {
                            LOG(LL_WARN, LCF_EVENTS | LCF_WINDOW, "   fullscreen window is not game window!");
                        }

                        /* Resize the window to the screen or fake resolution */
                        if (Global::shared_config.screen_width) {
                            const static uint32_t values[] = { static_cast<uint32_t>(Global::shared_config.screen_width), static_cast<uint32_t>(Global::shared_config.screen_height) };
                            xcb_configure_window(c, client_event->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
                        }
                        else {
#ifdef LIBTAS_HAS_XCB_RANDR
                            /* Change the window size to monitor size */
                            LINK_NAMESPACE(xcb_randr_get_screen_info_unchecked, "xcb-randr");
                            LINK_NAMESPACE(xcb_randr_get_screen_info_reply, "xcb-randr");
                            LINK_NAMESPACE(xcb_randr_get_screen_info_sizes, "xcb-randr");

                            xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(c));
                            xcb_screen_t* screen = iter.data;

                            xcb_randr_get_screen_info_cookie_t screen_info = orig::xcb_randr_get_screen_info_unchecked(c, screen->root);
                            xcb_randr_get_screen_info_reply_t* reply = orig::xcb_randr_get_screen_info_reply(c, screen_info, nullptr);
                            xcb_randr_screen_size_t* sizes = orig::xcb_randr_get_screen_info_sizes(reply);
                            const static uint32_t values[] = { sizes[0].width, sizes[0].height };
                            xcb_configure_window(c, client_event->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
#endif
                        }

                        break;
                    }

                    /* Detect and disable window always on top */
                    if (static_cast<Atom>(client_event->data.data32[1]) == x11_atom(_NET_WM_STATE_ABOVE)) {
                        LOG(LL_DEBUG, LCF_EVENTS | LCF_WINDOW, "   prevented window always on top");
                        break;
                    }
                }
            }

            send_request(vector);
            break;
        }

        case XCB_QUERY_KEYMAP:
        {
            LOG(LL_TRACE, LCF_KEYBOARD, "XCB_QUERY_KEYMAP raw request call");
            break;
        }

        case XCB_GRAB_KEYBOARD:
        {
            LOG(LL_TRACE, LCF_KEYBOARD, "XCB_GRAB_KEYBOARD raw request call");
            break;
        }

        case XCB_UNGRAB_KEYBOARD:
        {
            LOG(LL_TRACE, LCF_KEYBOARD, "XCB_UNGRAB_KEYBOARD raw request call");
            break;
        }

        case XCB_GRAB_KEY:
        {
            LOG(LL_TRACE, LCF_KEYBOARD, "XCB_GRAB_KEY raw request call");
            break;
        }

        case XCB_UNGRAB_KEY:
        {
            LOG(LL_TRACE, LCF_KEYBOARD, "XCB_UNGRAB_KEY raw request call");
            break;
        }

        case XCB_QUERY_POINTER:
        {
            LOG(LL_TRACE, LCF_MOUSE, "XCB_QUERY_POINTER raw request call");
            break;
        }

        case XCB_WARP_POINTER:
        {
            const auto* req = static_cast<const xcb_warp_pointer_request_t*>(vector->iov_base);
            LOG(LL_TRACE, LCF_MOUSE, "XCB_WARP_POINTER raw request called with dest_w %d and dest_x %d and dest_y %d", req->dst_window, req->dst_x, req->dst_y);

            /* Does this generate a XCB_MOTION_NOTIFY event? */
            if (!x11::gameXWindows.empty()) {
                xcb_motion_notify_event_t event;
                event.response_type = XCB_MOTION_NOTIFY;
                event.state = SingleInput::toXlibPointerMask(game_ai.pointer.mask);

                if (req->dst_window == XCB_NONE) {
                    /* Relative warp */
                    event.event_x = game_ai.pointer.x + req->dst_x;
                    event.event_y = game_ai.pointer.y + req->dst_y;
                }
                else {
                    /* Absolute warp */
                    event.event_x = req->dst_x;
                    event.event_y = req->dst_y;
                }
                event.root_x = event.event_x;
                event.root_y = event.event_y;
                event.event = x11::gameXWindows.front();

                struct timespec time = DeterministicTimer::get().getTicks();
                event.time = time.tv_sec * 1000 + time.tv_nsec / 1000000;

                xcbEventQueueList.insert(reinterpret_cast<xcb_generic_event_t*>(&event), false);
                LOG(LL_DEBUG, LCF_EVENTS | LCF_MOUSE, "Generate xcb event XCB_MOTION_NOTIFY with new position (%d,%d)", game_ai.pointer.x, game_ai.pointer.y);
            }

            /* Update the pointer coordinates */
            if (req->dst_window == XCB_NONE) {
                /* Relative warp */
                game_ai.pointer.x += req->dst_x;
                game_ai.pointer.y += req->dst_y;
            }
            else {
                /* Absolute warp */
                game_ai.pointer.x = req->dst_x;
                game_ai.pointer.y = req->dst_y;
            }

            if (Global::shared_config.mouse_prevent_warp) {
                break;
            }

            /* When warping cursor, real and game cursor position are now synced */
            if (Global::shared_config.mouse_support) {
                if (req->dst_window == XCB_NONE) {
                    /* Relative warp */
                    old_ai.pointer.x += req->dst_x;
                    old_ai.pointer.y += req->dst_y;
                }
                else {
                    /* Absolute warp */
                    old_ai.pointer.x = req->dst_x;
                    old_ai.pointer.y = req->dst_y;
                }
            }

            send_request(vector);
            break;
        }

        case XCB_GRAB_POINTER:
        {
            LOG(LL_TRACE, LCF_MOUSE, "XCB_GRAB_POINTER raw request call");
            break;
        }

        case XCB_UNGRAB_POINTER:
        {
            LOG(LL_TRACE, LCF_MOUSE, "XCB_UNGRAB_POINTER raw request call");
            break;
        }

        default:
        {
            send_request(vector);
            break;
        }
    }
}

unsigned int xcb_send_request(xcb_connection_t *c, int flags, struct iovec *vector, const xcb_protocol_request_t *req)
{
    LINK_NAMESPACE_GLOBAL(xcb_send_request);

    // xcb_send_request and friends just call into newer 64 bit and/or fds versions if available
    // so we need to guard against an unwanted double request handling

    // also, note that non-XCB_REQUEST_RAW calls are just used for normal helper request functions
    // therefore, we don't care about such calls

    if (GlobalState::isNative() || GlobalState::isOwnCode() || !(flags & XCB_REQUEST_RAW)) {
        return orig::xcb_send_request(c, flags, vector, req);
    }

    unsigned int ret = 0;
    handleRawRequest(c, vector, [&] (struct iovec *new_vector) {OWNCALL(ret = orig::xcb_send_request(c, flags, new_vector, req));});
    return ret;
}

unsigned int xcb_send_request_with_fds(xcb_connection_t *c, int flags, struct iovec *vector,
        const xcb_protocol_request_t *req, unsigned int num_fds, int *fds)
{
    LINK_NAMESPACE_GLOBAL(xcb_send_request_with_fds);

    if (GlobalState::isNative() || GlobalState::isOwnCode() || !(flags & XCB_REQUEST_RAW)) {
        return orig::xcb_send_request_with_fds(c, flags, vector, req, num_fds, fds);
    }

    unsigned int ret = 0;
    handleRawRequest(c, vector, [&] (struct iovec *new_vector) {OWNCALL(ret = orig::xcb_send_request_with_fds(c, flags, new_vector, req, num_fds, fds));});
    return ret;
}

uint64_t xcb_send_request64(xcb_connection_t *c, int flags, struct iovec *vector, const xcb_protocol_request_t *req)
{
    LINK_NAMESPACE_GLOBAL(xcb_send_request64);

    if (GlobalState::isNative() || GlobalState::isOwnCode() || !(flags & XCB_REQUEST_RAW)) {
        return orig::xcb_send_request64(c, flags, vector, req);
    }

    uint64_t ret = 0;
    handleRawRequest(c, vector, [&] (struct iovec *new_vector) {OWNCALL(ret = orig::xcb_send_request64(c, flags, new_vector, req));});
    return ret;
}

uint64_t xcb_send_request_with_fds64(xcb_connection_t *c, int flags, struct iovec *vector,
                const xcb_protocol_request_t *req, unsigned int num_fds, int *fds)
{
    LINK_NAMESPACE_GLOBAL(xcb_send_request_with_fds64);

    if (GlobalState::isNative() || GlobalState::isOwnCode() || !(flags & XCB_REQUEST_RAW)) {
        return orig::xcb_send_request_with_fds64(c, flags, vector, req, num_fds, fds);
    }

    uint64_t ret = 0;
    handleRawRequest(c, vector, [&] (struct iovec *new_vector) {OWNCALL(ret = orig::xcb_send_request_with_fds64(c, flags, new_vector, req, num_fds, fds));});
    return ret;
}

}
