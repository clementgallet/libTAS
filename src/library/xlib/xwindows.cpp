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

#include "config.h"
#include "xwindows.h"
#include "xevents.h"
#include "xatom.h"
#include "xrandr.h"
#include "XlibEventQueueList.h"

#include "hook.h"
#include "logging.h"
#include "screencapture/ScreenCapture.h"
#include "WindowTitle.h"
#include "encoding/AVEncoder.h"
#include "backtrace.h"
#include "inputs/xinput.h"
#include "global.h"
#include "GlobalState.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include "../external/mwm.h"

#include <vector>

namespace libtas {

DEFINE_ORIG_POINTER(XCreateWindow)
DEFINE_ORIG_POINTER(XCreateSimpleWindow)
DEFINE_ORIG_POINTER(XMapWindow)
DEFINE_ORIG_POINTER(XMapRaised)
DEFINE_ORIG_POINTER(XStoreName)
DEFINE_ORIG_POINTER(XSetWMName)
DEFINE_ORIG_POINTER(XResizeWindow)
DEFINE_ORIG_POINTER(XConfigureWindow)
DEFINE_ORIG_POINTER(XGetWindowAttributes)
DEFINE_ORIG_POINTER(XQueryExtension)
DEFINE_ORIG_POINTER(XChangeProperty)

std::list<Window> x11::gameXWindows;
Window x11::rootWindow;

Bool XQueryExtension(Display* display, const char* name, int* major_opcode_return, int* first_event_return, int* first_error_return) {
    LOG(LL_TRACE, LCF_WINDOW, "%s called with name %s", __func__, name);
    LINK_NAMESPACE_GLOBAL(XQueryExtension);
    Bool ret = orig::XQueryExtension(display, name, major_opcode_return, first_event_return, first_error_return);

    /* Gather Xi opcode */
    if (ret && (0 == strcmp(name, "XInputExtension"))) {
        xinput_opcode = *major_opcode_return;
    }

    return ret;
}

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s call with dimensions %d x %d", __func__, width, height);
    LINK_NAMESPACE_GLOBAL(XCreateWindow);

    long event_mask = 0;

    Window w = orig::XCreateWindow(display, parent, x, y, width, height, border_width, depth, klass, visual, valuemask, attributes);
    LOG(LL_DEBUG, LCF_WINDOW, "   window id is %d", w);

    /* Add the mask in our event queue */
    if (valuemask & CWEventMask) {
        std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
        queue->setMask(w, event_mask);
        LOG(LL_DEBUG, LCF_WINDOW, "   event mask is %d", event_mask);
    }

    /* Don't save windows that has override-redirect (Wine invisible windows) */
    if ((valuemask & CWOverrideRedirect) && (attributes->override_redirect == True)) {
        return w;
    }

    /* Only save the Window identifier for top-level windows */
    Window parent_return = 0;
    Window *children_return = nullptr;
    unsigned int nchildren_return = 0;
    XQueryTree(display, w, &x11::rootWindow, &parent_return, &children_return, &nchildren_return);
    if (children_return) XFree(children_return);

    if (x11::rootWindow == parent) {
        /* Saving top-level window */
        if (x11::gameXWindows.empty())
            LOG(LL_DEBUG, LCF_WINDOW, "   set game window to %d", w);
        x11::gameXWindows.push_back(w);
    }

    return w;
}

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s call with dimensions %d x %d", __func__, width, height);
    LINK_NAMESPACE_GLOBAL(XCreateSimpleWindow);

    Window w = orig::XCreateSimpleWindow(display, parent, x, y, width, height, border_width, border, background);
    LOG(LL_DEBUG, LCF_WINDOW, "   window id is %d", w);

    /* Only save the Window identifier for top-level windows */
    Window parent_return = 0;
    Window *children_return = nullptr;
    unsigned int nchildren_return = 0;
    XQueryTree(display, w, &x11::rootWindow, &parent_return, &children_return, &nchildren_return);
    if (children_return) XFree(children_return);

    if (x11::rootWindow == parent) {
        /* Saving top-level window */
        if (x11::gameXWindows.empty())
            LOG(LL_DEBUG, LCF_WINDOW, "   set game window to %d", w);
        x11::gameXWindows.push_back(w);
    }

    return w;
}

static void sendXWindow(Window w)
{
    uint32_t i = (uint32_t)w;
    lockSocket();
    sendMessage(MSGB_WINDOW_ID);
    sendData(&i, sizeof(i));
    unlockSocket();
    LOG(LL_DEBUG, LCF_WINDOW, "Sent X11 window id %d", w);
}

int XDestroyWindow(Display *display, Window w)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);

    /* If current game window, switch to another one on the list */
    if (!x11::gameXWindows.empty() && w == x11::gameXWindows.front()) {
        ScreenCapture::fini();

        x11::gameXWindows.pop_front();
        if (x11::gameXWindows.empty()) {
            /* Tells the program we don't have a window anymore to gather inputs */
            sendXWindow(0);
        }
        else if (!Global::is_exiting) {
            /* Switch to the next game window */
            LOG(LL_DEBUG, LCF_WINDOW, "   set game window to %d", x11::gameXWindows.front());
            sendXWindow(x11::gameXWindows.front());
        }
    }
    else {
        /* If another game window, remove it from the list */
        for (auto iter = x11::gameXWindows.begin(); iter != x11::gameXWindows.end(); iter++) {
            if (w == *iter) {
                x11::gameXWindows.erase(iter);
                break;
            }
        }
    }

    RETURN_NATIVE(XDestroyWindow, (display, w), nullptr);
}

int XMapWindow(Display *display, Window w)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);
    LINK_NAMESPACE_GLOBAL(XMapWindow);

    int ret = orig::XMapWindow(display, w);

    /* We must wait until the window is mapped to send it to the program.
     * We are checking the content of x11::gameXWindows to see if we must send it */
    for (auto iter = x11::gameXWindows.begin(); iter != x11::gameXWindows.end(); iter++) {
        if (w == *iter) {
            x11::gameXWindows.erase(iter);
            x11::gameXWindows.push_front(w);
            sendXWindow(w);
            break;
        }
    }

    return ret;
}

int XUnmapWindow(Display *display, Window w)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);
    RETURN_NATIVE(XUnmapWindow, (display, w), nullptr);
}

int XMapRaised(Display *display, Window w)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);
    LINK_NAMESPACE_GLOBAL(XMapRaised);

    int ret = orig::XMapRaised(display, w);

    /* We must wait until the window is mapped to send it to the program.
     * We are checking the content of gameXWindow to see if we must send it
     */
    for (auto iter = x11::gameXWindows.begin(); iter != x11::gameXWindows.end(); iter++) {
        if (w == *iter) {
            x11::gameXWindows.erase(iter);
            x11::gameXWindows.push_front(w);
            sendXWindow(w);
            break;
        }
    }

    return ret;
}

int XStoreName(Display *display, Window w, const char *window_name)
{
    LOGTRACE(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(XStoreName);

    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w)) {
        WindowTitle::setOriginalTitle(window_name);
        WindowTitle::setUpdateFunc([display] (const char* t) {if (!x11::gameXWindows.empty()) orig::XStoreName(display, x11::gameXWindows.front(), t);});
    }

    return 1;
}

void XSetWMName(Display *display, Window w, XTextProperty *text_prop)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s call with name %s and format %d", __func__, text_prop->value, text_prop->format);
    LINK_NAMESPACE_GLOBAL(XSetWMName);

    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w)) {
        WindowTitle::setOriginalTitle(reinterpret_cast<const char*>(const_cast<const unsigned char*>(text_prop->value)));
        WindowTitle::setUpdateFunc([display] (const char* t) {
            if (!x11::gameXWindows.empty()) {
                XTextProperty prop;
                XStringListToTextProperty(const_cast<char**>(&t), 1, &prop);
                orig::XSetWMName(display, x11::gameXWindows.front(), &prop);
            }
        });
        return;
    }

    return orig::XSetWMName(display, w, text_prop);
}

int XSelectInput(Display *display, Window w, long event_mask)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);

    /* Add the mask in our event queue */
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    queue->setMask(w, event_mask);

    RETURN_NATIVE(XSelectInput, (display, w, event_mask), nullptr);
}

int XMoveWindow(Display* display, Window w, int x, int y)
{
    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);
    /* Preventing the game to change the game window position */
    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w)) {
        return 0;
    }

    RETURN_NATIVE(XMoveWindow, (display, w, x, y), nullptr);
}

int XResizeWindow(Display* display, Window w, unsigned int width, unsigned int height)
{
    RETURN_IF_NATIVE(XResizeWindow, (display, w, width, height), nullptr);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d, new size: %d x %d", __func__, w, width, height);
    ScreenCapture::resize(width, height);
    RETURN_NATIVE(XResizeWindow, (display, w, width, height), nullptr);
}

int XMoveResizeWindow(Display* display, Window w, int x, int y, unsigned int width, unsigned int height)
{
    RETURN_IF_NATIVE(XResizeWindow, (display, w, width, height), nullptr);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d, new position: %d - %d, new size: %d x %d", __func__, w, x, y, width, height);

    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w)) {
        ScreenCapture::resize(width, height);
    }
    RETURN_NATIVE(XResizeWindow, (display, w, width, height), nullptr);
}

int XConfigureWindow(Display* display, Window w, unsigned int value_mask, XWindowChanges* values)
{
    LINK_NAMESPACE_GLOBAL(XConfigureWindow);
    if (GlobalState::isNative())
        return orig::XConfigureWindow(display, w, value_mask, values);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);
    if ((value_mask & CWWidth) && (value_mask & CWHeight)) {
        LOG(LL_DEBUG, LCF_WINDOW, "    New size: %d x %d", values->width, values->height);
    }

    /* Disable window movement */
    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w))
        value_mask &= ~(CWX | CWY);

    int ret = orig::XConfigureWindow(display, w, value_mask, values);

    /* Check if size has changed */
    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w)) {
        if ((value_mask & CWWidth) && (value_mask & CWHeight)) {
            ScreenCapture::resize(values->width, values->height);
        }
    }
    return ret;
}

int XChangeProperty(Display* display, Window w, Atom property, Atom type, int format, int mode, const unsigned char* data, int nelements)
{
    LINK_NAMESPACE_GLOBAL(XChangeProperty);
    if (GlobalState::isNative())
        return orig::XChangeProperty(display, w, property, type, format, mode, data, nelements);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);

    /* Prevent games from intercepting ClientMessage focus events */
    if (property == x11_atom(_NET_WM_STATE)) {
        const Atom* atoms = reinterpret_cast<const Atom*>(data);
        std::vector<Atom> newatoms;
        for (int i=0; i<nelements; i++) {
            if (atoms[i] == x11_atom(_NET_WM_STATE_FULLSCREEN)) {
                LOG(LL_DEBUG, LCF_WINDOW, "   prevented fullscreen switching but resized the window");
                if (!x11::gameXWindows.empty() && (w != x11::gameXWindows.front())) {
                    LOG(LL_WARN, LCF_WINDOW, "   fullscreen window is not game window!");
                }

                /* Resize the window to the screen or fake resolution */
                if (Global::shared_config.screen_width) {
                    XResizeWindow(display, w, Global::shared_config.screen_width, Global::shared_config.screen_height);
                }
                else {
                    /* Change the window size to monitor size */
                    int fs_width, fs_height;
                    get_monitor_resolution(fs_width, fs_height);
                    XResizeWindow(display, w, fs_width, fs_height);
                }
            }
            else if (atoms[i] == x11_atom(_NET_WM_STATE_ABOVE)) {
                LOG(LL_DEBUG, LCF_WINDOW, "   prevented window always on top");
            }
            else {
                newatoms.push_back(atoms[i]);
            }
        }
        return orig::XChangeProperty(display, w, property, type, format, mode, reinterpret_cast<unsigned char*>(newatoms.data()), newatoms.size());
    }

    /* Detect and disable several window state changes */
    if (property == x11_atom(WM_PROTOCOLS)) {
        const Atom* atoms = reinterpret_cast<const Atom*>(data);
        for (int i=0; i<nelements; i++) {
            if (atoms[i] == x11_atom(WM_TAKE_FOCUS)) {
                LOG(LL_DEBUG, LCF_WINDOW, "   removing WM_TAKE_FOCUS protocol");
                std::vector<Atom> newatoms;
                for (int j=0; j<nelements-1; j++) {
                    if (j!=i) {
                        newatoms.push_back(atoms[j]);
                    }
                }
                return orig::XChangeProperty(display, w, property, type, format, mode, reinterpret_cast<unsigned char*>(newatoms.data()), newatoms.size());
            }
        }
    }

    /* Detect a title change */
    if (property == x11_atom(_NET_WM_NAME)) {
        LOG(LL_DEBUG, LCF_WINDOW, "   change title to %s", data);
        if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w)) {
            WindowTitle::setOriginalTitle(reinterpret_cast<const char*>(data));
            WindowTitle::setUpdateFunc([display] (const char* t) {
                if (!x11::gameXWindows.empty()) {
                    orig::XChangeProperty(display, x11::gameXWindows.front(), x11_atom(_NET_WM_NAME), x11_atom(UTF8_STRING), 8, PropModeReplace, reinterpret_cast<const unsigned char*>(t), strlen(t));
                }
            });
            return 1; // value from implementation apparently
        }
    }

    /* Always display window borders/title/menu/etc */
    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w) && (property == x11_atom(_MOTIF_WM_HINTS))) {
        MwmHints mwm_hints = *reinterpret_cast<const MwmHints*>(data);
        if (mwm_hints.decorations == 0) {
            LOG(LL_DEBUG, LCF_WINDOW, "   adding motif decorations");
            mwm_hints.decorations = MWM_DECOR_TITLE | MWM_DECOR_BORDER | MWM_DECOR_MENU | MWM_DECOR_MINIMIZE;
            return orig::XChangeProperty(display, w, property, type, format, mode, reinterpret_cast<unsigned char*>(&mwm_hints), nelements);
        }
    }

    return orig::XChangeProperty(display, w, property, type, format, mode, data, nelements);
}

int XSetWMHints(Display* display, Window w, XWMHints* wm_hints)
{
    RETURN_IF_NATIVE(XSetWMHints, (display, w, wm_hints), nullptr);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);

    if (!x11::gameXWindows.empty() && (x11::gameXWindows.front() == w) && (wm_hints->input == False)) {
        LOG(LL_DEBUG, LCF_WINDOW, "   switch input hint to True");
        wm_hints->input = True;
    }

    RETURN_NATIVE(XSetWMHints, (display, w, wm_hints), nullptr);
}

Bool XTranslateCoordinates(Display* display, Window src_w, Window dest_w, int src_x, int src_y, int* dest_x_return, int* dest_y_return, Window* child_return)
{
    RETURN_IF_NATIVE(XTranslateCoordinates, (display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, child_return), nullptr);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with src_w %d, dest_w %d, src_x %d, src_y %d", __func__, src_w, dest_w, src_x, src_y);

    if (dest_w == DefaultRootWindow(display)) {
        *dest_x_return = src_x;
        *dest_y_return = src_y;
        if (child_return) *child_return = src_w;
        return True;
    }
    RETURN_NATIVE(XTranslateCoordinates, (display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, child_return), nullptr);
}

Status XGetWindowAttributes(Display* display, Window w, XWindowAttributes* window_attributes_return)
{
    LINK_NAMESPACE_GLOBAL(XGetWindowAttributes);
    if (GlobalState::isNative())
        return orig::XGetWindowAttributes(display, w, window_attributes_return);

    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);
    Status ret = orig::XGetWindowAttributes(display, w, window_attributes_return);

    /* Change the window position to 0,0 */
    window_attributes_return->x = 0;
    window_attributes_return->y = 0;

    return ret;
}

int XChangeWindowAttributes(Display *display, Window w, unsigned long valuemask, XSetWindowAttributes *attributes)
{
    RETURN_IF_NATIVE(XChangeWindowAttributes, (display, w, valuemask, attributes), nullptr);
    LOG(LL_TRACE, LCF_WINDOW, "%s called with window %d", __func__, w);

    /* Add the mask in our event queue */
    if (valuemask & CWEventMask) {
        std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
        queue->setMask(w, attributes->event_mask);
    }

    RETURN_NATIVE(XChangeWindowAttributes, (display, w, valuemask, attributes), nullptr);
}

}
