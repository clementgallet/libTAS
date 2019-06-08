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
DEFINE_ORIG_POINTER(XResizeWindow);
DEFINE_ORIG_POINTER(XConfigureWindow);
DEFINE_ORIG_POINTER(XQueryExtension);
DEFINE_ORIG_POINTER(XChangeProperty);
DEFINE_ORIG_POINTER(XSetWMHints);

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
    debuglog(LCF_WINDOW, __func__, "   window id is ", w);

    /* Only save the Window identifier for real resolutions, because SDL creates
     * some dummy windows for testing.
     */
    if ((gameXWindow == 0) && (width > 32) && (height > 32)) {
        gameXWindow = w;
    }

    return w;
}

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
    debuglog(LCF_WINDOW, __func__, " call with dimensions ", width, "x", height);
    LINK_NAMESPACE_GLOBAL(XCreateSimpleWindow);

    Window w = orig::XCreateSimpleWindow(display, parent, x, y, width, height, border_width, border, background);
    debuglog(LCF_WINDOW, "   window id is ", w);

    /* Only save the Window identifier for real resolutions, because SDL creates
     * some dummy windows for testing.
     */
    if ((gameXWindow == 0) && (width > 32) && (height > 32)) {
        gameXWindow = w;
    }

    return w;
}

int XDestroyWindow(Display *display, Window w)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XDestroyWindow);

    ScreenCapture::fini();

    if (gameXWindow == w) {
        gameXWindow = 0;

        /* Tells the program we don't have a window anymore to gather inputs */
        sendMessage(MSGB_WINDOW_ID);
        sendData(&gameXWindow, sizeof(Window));
        debuglog(LCF_WINDOW, "Sent X11 window id 0");        
    }


    return orig::XDestroyWindow(display, w);
}

int XMapWindow(Display *display, Window w)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XMapWindow);

    int ret = orig::XMapWindow(display, w);

    /* We must wait until the window is mapped to send it to the program.
     * We are checking the content of gameXWindow to see if we must send it
     */
    if ((gameXWindow != 0) && (gameXWindow == w)) {
        sendMessage(MSGB_WINDOW_ID);
        sendData(&w, sizeof(Window));
        debuglog(LCF_WINDOW, "Sent X11 window id: ", w);
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
    if (gameXWindow != 0) {
        sendMessage(MSGB_WINDOW_ID);
        sendData(&w, sizeof(Window));
        debuglog(LCF_WINDOW, "Sent X11 window id: ", w);
    }

    return ret;
}

int XStoreName(Display *display, Window w, const char *window_name)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE_GLOBAL(XStoreName);

    WindowTitle::setOriginalTitle(window_name);
    WindowTitle::setUpdateFunc([display] (const char* t) {if (gameXWindow) orig::XStoreName(display, gameXWindow, t);});

    return orig::XStoreName(display, w, window_name);
}

void XSetWMName(Display *display, Window w, XTextProperty *text_prop)
{
    debuglog(LCF_WINDOW, __func__, " call with name ", text_prop->value, " and format ", text_prop->format);
    LINK_NAMESPACE_GLOBAL(XSetWMName);
    Atom encoding = text_prop->encoding;

    WindowTitle::setOriginalTitle(reinterpret_cast<const char*>(const_cast<const unsigned char*>(text_prop->value)));
    WindowTitle::setUpdateFunc([display, encoding] (const char* t) {
        if (gameXWindow) {
            XTextProperty prop;
            XStringListToTextProperty(const_cast<char**>(&t), 1, &prop);
            orig::XSetWMName(display, gameXWindow, &prop);            
        }
    });

    return orig::XSetWMName(display, w, text_prop);
}

int XSelectInput(Display *display, Window w, long event_mask)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XSelectInput);

    return orig::XSelectInput(display, w, event_mask);
}

int XMoveWindow(Display* display, Window w, int x, int y)
{
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    /* Preventing the game to change the window position */
    return 0;
}

int XResizeWindow(Display* display, Window w, unsigned int width, unsigned int height)
{
    LINK_NAMESPACE_GLOBAL(XResizeWindow);
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    int ret = orig::XResizeWindow(display, w, width, height);

    if (GlobalState::isNative())
        return ret;

    DEBUGLOGCALL(LCF_WINDOW);
    debuglog(LCF_WINDOW, "    New size: ", width, " x ", height);

    /* We assume that a resized window is the game window */
    if (gameXWindow == 0)
        gameXWindow = w;

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
    debuglog(LCF_WINDOW, __func__, " called with window ", w);
    LINK_NAMESPACE_GLOBAL(XResizeWindow);
    int ret = orig::XResizeWindow(display, w, width, height);

    if (GlobalState::isNative())
        return ret;

    DEBUGLOGCALL(LCF_WINDOW);
    debuglog(LCF_WINDOW, "    New position: ", x, " - ", y, " new size: ", width, " x ", height);

    /* We assume that a resized window is the game window */
    if (gameXWindow == 0)
        gameXWindow = w;

    /* Check if size has changed */
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
    value_mask &= ~(CWX | CWY);

    int ret = orig::XConfigureWindow(display, w, value_mask, values);

    /* Check if size has changed */
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
    return orig::XChangeProperty(display, w, property, type, format, mode, data, nelements);
}

int XSetWMHints(Display* display, Window w, XWMHints* wm_hints)
{
    LINK_NAMESPACE_GLOBAL(XSetWMHints);
    if (GlobalState::isNative())
        return orig::XSetWMHints(display, w, wm_hints);

    debuglog(LCF_WINDOW, __func__, " called with window ", w);

    if (wm_hints->input == False) {
        debuglog(LCF_WINDOW, "   switch input hint to True");
        wm_hints->input = True;
    }
    
    return orig::XSetWMHints(display, w, wm_hints);
}

}
