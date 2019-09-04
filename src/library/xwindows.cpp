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
#include "xwindows.h"
#include "xevents.h"
#include "hook.h"
#include "logging.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include "ScreenCapture.h"
#include "WindowTitle.h"
#include "encoding/AVEncoder.h"
#include "backtrace.h"
#include "inputs/xinput.h"
#include "xatom.h"
#include "../external/mwm.h"
#include "XlibEventQueueList.h"

namespace libtas {

DEFINE_ORIG_POINTER(XCreateWindow);
DEFINE_ORIG_POINTER(XCreateSimpleWindow);
DEFINE_ORIG_POINTER(XDestroyWindow);
DEFINE_ORIG_POINTER(XMapWindow);
DEFINE_ORIG_POINTER(XUnmapWindow);
DEFINE_ORIG_POINTER(XMapRaised);
DEFINE_ORIG_POINTER(XStoreName);
DEFINE_ORIG_POINTER(XSetWMName);
DEFINE_ORIG_POINTER(XSelectInput);
DEFINE_ORIG_POINTER(XMoveWindow);
DEFINE_ORIG_POINTER(XResizeWindow);
DEFINE_ORIG_POINTER(XConfigureWindow);
DEFINE_ORIG_POINTER(XChangeWindowAttributes);
DEFINE_ORIG_POINTER(XQueryExtension);
DEFINE_ORIG_POINTER(XChangeProperty);
DEFINE_ORIG_POINTER(XSetWMHints);
DEFINE_ORIG_POINTER(XTranslateCoordinates);

Bool XQueryExtension(Display* display, const char* name, int* major_opcode_return, int* first_event_return, int* first_error_return) {
    debuglog(LCF_WINDOW, __func__, " called with name ", name);
    LINK_NAMESPACE_GLOBAL(XQueryExtension);
    Bool ret = orig::XQueryExtension(display, name, major_opcode_return, first_event_return, first_error_return);

#ifdef LIBTAS_HAS_XINPUT
    /* Gather Xi opcode */
    if (ret && (0 == strcmp(name, "XInputExtension"))) {
        xinput_opcode = *major_opcode_return;
    }
#endif

    return ret;
}

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
{
    debuglog(LCF_WINDOW, __func__, " call with dimensions ", width, "x", height);
    LINK_NAMESPACE_GLOBAL(XCreateWindow);

    Window w = orig::XCreateWindow(display, parent, x, y, width, height, border_width, depth, klass, visual, valuemask, attributes);
    debuglog(LCF_WINDOW, "   window id is ", w);

    /* Add the mask in our event queue */
    if (valuemask & CWEventMask) {
        std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
        queue->setMask(w, attributes->event_mask);
    }

    /* Only save the Window identifier for top-level windows */
    Window root_return = 0;
    Window parent_return = 0;
    Window *children_return = nullptr;
    unsigned int nchildren_return = 0;
    XQueryTree(display, w, &root_return, &parent_return, &children_return, &nchildren_return);
    if (children_return) XFree(children_return);

    if (root_return == parent) {
        /* Saving top-level window */
        if (gameXWindows.empty())
            debuglog(LCF_WINDOW, "   set game window to ", w);
        gameXWindows.push_back(w);
    }

    return w;
}

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
    debuglog(LCF_WINDOW, __func__, " call with dimensions ", width, "x", height);
    LINK_NAMESPACE_GLOBAL(XCreateSimpleWindow);

    Window w = orig::XCreateSimpleWindow(display, parent, x, y, width, height, border_width, border, background);
    debuglog(LCF_WINDOW, "   window id is ", w);

    /* Only save the Window identifier for top-level windows */
    Window root_return = 0;
    Window parent_return = 0;
    Window *children_return = nullptr;
    unsigned int nchildren_return = 0;
    XQueryTree(display, w, &root_return, &parent_return, &children_return, &nchildren_return);
    if (children_return) XFree(children_return);

    if (root_return == parent) {
        /* Saving top-level window */
        if (gameXWindows.empty())
            debuglog(LCF_WINDOW, "   set game window to ", w);
        gameXWindows.push_back(w);
    }

    return w;
}

void sendXWindow(Window w)
{
    uint32_t i = (uint32_t)w;
    sendMessage(MSGB_WINDOW_ID);
    sendData(&i, sizeof(i));
    debuglog(LCF_WINDOW, "Sent X11 window id ", w);
}

int XDestroyWindow(Display *display, Window w)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XDestroyWindow);

    /* If current game window, switch to another one on the list */
    if (!gameXWindows.empty() && w == gameXWindows.front()) {
        ScreenCapture::fini();

        gameXWindows.pop_front();
        if (gameXWindows.empty()) {
            /* Tells the program we don't have a window anymore to gather inputs */
            sendXWindow(0);
        }
        else {
            /* Switch to the next game window */
            debuglog(LCF_WINDOW, "   set game window to ", gameXWindows.front());
            sendXWindow(gameXWindows.front());
            ScreenCapture::init();
        }
    }
    else {
        /* If another game window, remove it from the list */
        for (auto iter = gameXWindows.begin(); iter != gameXWindows.end(); iter++) {
            if (w == *iter) {
                gameXWindows.erase(iter);
                break;
            }
        }
    }

    return orig::XDestroyWindow(display, w);
}

int XMapWindow(Display *display, Window w)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XMapWindow);

    int ret = orig::XMapWindow(display, w);

    /* We must wait until the window is mapped to send it to the program.
     * We are checking the content of gameXWindows to see if we must send it */
    for (auto iter = gameXWindows.begin(); iter != gameXWindows.end(); iter++) {
        if (w == *iter) {
            gameXWindows.erase(iter);
            gameXWindows.push_front(w);
            sendXWindow(w);
            break;
        }
    }

    return ret;
}

int XUnmapWindow(Display *display, Window w)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XUnmapWindow);

    int ret = orig::XUnmapWindow(display, w);

    return ret;
}

int XMapRaised(Display *display, Window w)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XMapRaised);

    int ret = orig::XMapRaised(display, w);

    /* We must wait until the window is mapped to send it to the program.
     * We are checking the content of gameXWindow to see if we must send it
     */
    for (auto iter = gameXWindows.begin(); iter != gameXWindows.end(); iter++) {
        if (w == *iter) {
            gameXWindows.erase(iter);
            gameXWindows.push_front(w);
            sendXWindow(w);
            break;
        }
    }

    return ret;
}

int XStoreName(Display *display, Window w, const char *window_name)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(XStoreName);

    if (!gameXWindows.empty() && (gameXWindows.front() == w)) {
        WindowTitle::setOriginalTitle(window_name);
        WindowTitle::setUpdateFunc([display] (const char* t) {if (!gameXWindows.empty()) orig::XStoreName(display, gameXWindows.front(), t);});
    }

    return orig::XStoreName(display, w, window_name);
}

void XSetWMName(Display *display, Window w, XTextProperty *text_prop)
{
    debuglog(LCF_WINDOW, __func__, " call with name ", text_prop->value, " and format ", text_prop->format);
    LINK_NAMESPACE_GLOBAL(XSetWMName);

    if (!gameXWindows.empty() && (gameXWindows.front() == w)) {
        Atom encoding = text_prop->encoding;
        WindowTitle::setOriginalTitle(reinterpret_cast<const char*>(const_cast<const unsigned char*>(text_prop->value)));
        WindowTitle::setUpdateFunc([display, encoding] (const char* t) {
            if (!gameXWindows.empty()) {
                XTextProperty prop;
                XStringListToTextProperty(const_cast<char**>(&t), 1, &prop);
                orig::XSetWMName(display, gameXWindows.front(), &prop);
            }
        });
    }

    return orig::XSetWMName(display, w, text_prop);
}

int XSelectInput(Display *display, Window w, long event_mask)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XSelectInput);

    /* Add the mask in our event queue */
    std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
    queue->setMask(w, event_mask);

    return orig::XSelectInput(display, w, event_mask);
}

int XMoveWindow(Display* display, Window w, int x, int y)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    /* Preventing the game to change the game window position */
    if (!gameXWindows.empty() && (gameXWindows.front() == w)) {
        return 0;
    }

    LINK_NAMESPACE_GLOBAL(XMoveWindow);
    return orig::XMoveWindow(display, w, x, y);
}

int XResizeWindow(Display* display, Window w, unsigned int width, unsigned int height)
{
    LINK_NAMESPACE_GLOBAL(XResizeWindow);
    int ret = orig::XResizeWindow(display, w, width, height);

    if (GlobalState::isNative())
        return ret;

    debuglog(LCF_WINDOW, __func__, " called with window ", w, ", new size: ", width, " x ", height);
    int old_width, old_height;
    ScreenCapture::getDimensions(old_width, old_height);
    if ((old_width != width) || (old_height != height)) {
        ScreenCapture::resize(width, height);

        /* We need to close the dumping if needed, and open a new one */
        if (shared_config.av_dumping) {
            debuglog(LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
            avencoder.reset(new AVEncoder());
        }
    }
    return ret;
}

int XMoveResizeWindow(Display* display, Window w, int x, int y, unsigned int width, unsigned int height)
{
    LINK_NAMESPACE_GLOBAL(XResizeWindow);
    int ret = orig::XResizeWindow(display, w, width, height);

    if (GlobalState::isNative())
        return ret;

    debuglog(LCF_WINDOW, __func__, " called with window ", w, ", new position: ", x, " - ", y, " new size: ", width, " x ", height);

    /* Check if size has changed */
    if (!gameXWindows.empty() && (gameXWindows.front() == w)) {
        int old_width, old_height;
        ScreenCapture::getDimensions(old_width, old_height);
        if ((old_width != width) || (old_height != height)) {
            ScreenCapture::resize(width, height);

            /* We need to close the dumping if needed, and open a new one */
            if (shared_config.av_dumping) {
                debuglog(LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
                avencoder.reset(new AVEncoder());
            }
        }
    }
    return ret;
}

int XConfigureWindow(Display* display, Window w, unsigned int value_mask, XWindowChanges* values)
{
    LINK_NAMESPACE_GLOBAL(XConfigureWindow);
    if (GlobalState::isNative())
        return orig::XConfigureWindow(display, w, value_mask, values);

    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    if ((value_mask & CWWidth) && (value_mask & CWHeight)) {
        debuglog(LCF_WINDOW, "    New size: ", values->width, " x ", values->height);
    }

    /* Disable window movement */
    if (!gameXWindows.empty() && (gameXWindows.front() == w))
        value_mask &= ~(CWX | CWY);

    int ret = orig::XConfigureWindow(display, w, value_mask, values);

    /* Check if size has changed */
    if (!gameXWindows.empty() && (gameXWindows.front() == w)) {
        int old_width, old_height;
        ScreenCapture::getDimensions(old_width, old_height);
        if ((value_mask & CWWidth) && (value_mask & CWHeight) && ((values->width != old_width) || (values->height != old_height))) {
            ScreenCapture::resize(values->width, values->height);

            /* We need to close the dumping if needed, and open a new one */
            if (shared_config.av_dumping) {
                debuglog(LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
                avencoder.reset(new AVEncoder());
            }
        }
    }
    return ret;
}

int XChangeProperty(Display* display, Window w, Atom property, Atom type, int format, int mode, const unsigned char* data, int nelements)
{
    LINK_NAMESPACE_GLOBAL(XChangeProperty);
    if (GlobalState::isNative())
        return orig::XChangeProperty(display, w, property, type, format, mode, data, nelements);

    debuglog(LCF_WINDOW, __func__, " called with window ", w);

    /* Prevent games from intercepting ClientMessage focus events */
    if (property == x11_atom(WM_PROTOCOLS)) {
        const Atom* atoms = reinterpret_cast<const Atom*>(data);
        for (int i=0; i<nelements; i++) {
            if (atoms[i] == x11_atom(WM_TAKE_FOCUS)) {
                debuglog(LCF_WINDOW, "   removing WM_TAKE_FOCUS protocol");
                Atom newatoms[nelements-1];
                for (int j=0; j<nelements-1; j++) {
                    if (j<i) {
                        newatoms[j] = atoms[j];
                    }
                    else {
                        newatoms[j] = atoms[j+1];
                    }
                }
                return orig::XChangeProperty(display, w, property, type, format, mode, reinterpret_cast<unsigned char*>(newatoms), nelements-1);
            }
        }
    }

    /* Always display window borders/title/menu/etc */
    if (!gameXWindows.empty() && (gameXWindows.front() == w) && (property == x11_atom(_MOTIF_WM_HINTS))) {
        MwmHints mwm_hints = *reinterpret_cast<const MwmHints*>(data);
        if (mwm_hints.decorations == 0) {
            debuglog(LCF_WINDOW, "   adding motif decorations");
            mwm_hints.decorations = MWM_DECOR_TITLE | MWM_DECOR_BORDER | MWM_DECOR_MENU | MWM_DECOR_MINIMIZE;
            return orig::XChangeProperty(display, w, property, type, format, mode, reinterpret_cast<unsigned char*>(&mwm_hints), nelements);
        }
    }

    return orig::XChangeProperty(display, w, property, type, format, mode, data, nelements);
}

int XSetWMHints(Display* display, Window w, XWMHints* wm_hints)
{
    LINK_NAMESPACE_GLOBAL(XSetWMHints);
    if (GlobalState::isNative())
        return orig::XSetWMHints(display, w, wm_hints);

    debuglog(LCF_WINDOW, __func__, " called with window ", w);

    if (!gameXWindows.empty() && (gameXWindows.front() == w) && (wm_hints->input == False)) {
        debuglog(LCF_WINDOW, "   switch input hint to True");
        wm_hints->input = True;
    }

    return orig::XSetWMHints(display, w, wm_hints);
}

Bool XTranslateCoordinates(Display* display, Window src_w, Window dest_w, int src_x, int src_y, int* dest_x_return, int* dest_y_return, Window* child_return)
{
    LINK_NAMESPACE_GLOBAL(XTranslateCoordinates);
    if (GlobalState::isNative())
        return orig::XTranslateCoordinates(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, child_return);

    debuglog(LCF_WINDOW, __func__, " called with src_w ", src_w, " dest_w ", dest_w, " src_x ", src_x, " src_y ", src_y);

    if (dest_w == DefaultRootWindow(display)) {
        *dest_x_return = src_x;
        *dest_y_return = src_y;
        if (child_return) *child_return = src_w;
        return True;
    }
    return orig::XTranslateCoordinates(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, child_return);
}

int XChangeWindowAttributes(Display *display, Window w, unsigned long valuemask, XSetWindowAttributes *attributes)
{
    LINK_NAMESPACE_GLOBAL(XChangeWindowAttributes);
    if (GlobalState::isNative())
        return orig::XChangeWindowAttributes(display, w, valuemask, attributes);

    debuglog(LCF_WINDOW, __func__, " called with window ", w);

    /* Add the mask in our event queue */
    if (valuemask & CWEventMask) {
        std::shared_ptr<XlibEventQueue> queue = xlibEventQueueList.getQueue(display);
        queue->setMask(w, attributes->event_mask);
    }

    return orig::XChangeWindowAttributes(display, w, valuemask, attributes);
}

}
