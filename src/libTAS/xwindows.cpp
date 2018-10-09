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

#include "xwindows.h"
#include "hook.h"
#include "logging.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include "ScreenCapture.h"
#include "WindowTitle.h"
#include "encoding/AVEncoder.h"


namespace libtas {

DEFINE_ORIG_POINTER(XCreateWindow);
DEFINE_ORIG_POINTER(XCreateSimpleWindow);
DEFINE_ORIG_POINTER(XDestroyWindow);
DEFINE_ORIG_POINTER(XMapWindow);
DEFINE_ORIG_POINTER(XMapRaised);
DEFINE_ORIG_POINTER(XStoreName);
DEFINE_ORIG_POINTER(XSetWMName);
DEFINE_ORIG_POINTER(XInternAtom);
DEFINE_ORIG_POINTER(XSelectInput);
DEFINE_ORIG_POINTER(XResizeWindow);
DEFINE_ORIG_POINTER(XConfigureWindow);

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int klass, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
{
    debuglog(LCF_WINDOW, __func__, " call with dimensions ", width, "x", height);
    LINK_NAMESPACE_GLOBAL(XCreateWindow);

    Window w = orig::XCreateWindow(display, parent, x, y, width, height, border_width, depth, klass, visual, valuemask, attributes);

    /* Only save the Window identifier for real resolutions, because SDL creates
     * some dummy windows for testing.
     */
    if ((gameXWindow == 0) && (width > 32) && (height > 32)) {
        gameDisplay = display;
        gameXWindow = w;
    }

    return w;
}

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
    debuglog(LCF_WINDOW, __func__, " call with dimensions ", width, "x", height);
    LINK_NAMESPACE(XCreateSimpleWindow, nullptr);

    Window w = orig::XCreateSimpleWindow(display, parent, x, y, width, height, border_width, border, background);

    /* Only save the Window identifier for real resolutions, because SDL creates
     * some dummy windows for testing.
     */
    if ((gameXWindow == 0) && (width > 32) && (height > 32)) {
        gameDisplay = display;
        gameXWindow = w;
    }

    return w;
}

int XDestroyWindow(Display *display, Window w)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XDestroyWindow, nullptr);

    ScreenCapture::fini();

    gameXWindow = 0;

    /* Tells the program we don't have a window anymore to gather inputs */
    sendMessage(MSGB_WINDOW_ID);
    sendData(&gameXWindow, sizeof(Window));
    debuglog(LCF_WINDOW, "Sent X11 window id 0");

    return orig::XDestroyWindow(display, w);
}

int XMapWindow(Display *display, Window w)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XMapWindow, nullptr);

    int ret = orig::XMapWindow(display, w);

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

int XMapRaised(Display *display, Window w)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XMapRaised, nullptr);

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
    LINK_NAMESPACE(XStoreName, nullptr);

    WindowTitle::setOriginalTitle(window_name);
    WindowTitle::setUpdateFunc([display, w] (const char* t) {orig::XStoreName(display, w, t);});

    return orig::XStoreName(display, w, window_name);
}

void XSetWMName(Display *display, Window w, XTextProperty *text_prop)
{
    debuglog(LCF_WINDOW, __func__, " call with name ", text_prop->value, " and format ", text_prop->format);
    LINK_NAMESPACE(XSetWMName, nullptr);
    Atom encoding = text_prop->encoding;

    WindowTitle::setOriginalTitle(reinterpret_cast<const char*>(const_cast<const unsigned char*>(text_prop->value)));
    WindowTitle::setUpdateFunc([display, w, encoding] (const char* t) {
        XTextProperty prop;
        XStringListToTextProperty(const_cast<char**>(&t), 1, &prop);
        orig::XSetWMName(display, w, &prop);
    });

    return orig::XSetWMName(display, w, text_prop);
}

Atom XInternAtom(Display* display, const char* atom_name, Bool only_if_exists)
{
    debuglog(LCF_WINDOW, __func__, " call with atom ", atom_name);
    LINK_NAMESPACE(XInternAtom, nullptr);
    return orig::XInternAtom(display, atom_name, only_if_exists);
}

int XSelectInput(Display *display, Window w, long event_mask)
{
    DEBUGLOGCALL(LCF_WINDOW);
    LINK_NAMESPACE(XSelectInput, nullptr);

    return orig::XSelectInput(display, w, event_mask);
}

int XMoveWindow(Display* display, Window w, int x, int y)
{
    DEBUGLOGCALL(LCF_WINDOW);
    /* Preventing the game to change the window position */
    return 0;
}

int XResizeWindow(Display* display, Window w, unsigned int width, unsigned int height)
{
    LINK_NAMESPACE_GLOBAL(XResizeWindow);
    int ret = orig::XResizeWindow(display, w, width, height);

    if (GlobalState::isNative())
        return ret;

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    debuglog(LCF_SDL | LCF_WINDOW, "    New size: ", width, " x ", height);

    int old_width, old_height;
    ScreenCapture::getDimensions(old_width, old_height);
    if ((old_width != width) || (old_height != height)) {
        ScreenCapture::reinit();

        /* We need to close the dumping if needed, and open a new one */
        if (shared_config.av_dumping) {
            debuglog(LCF_SDL | LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
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

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    debuglog(LCF_SDL | LCF_WINDOW, "    New position: ", x, " - ", y, " new size: ", width, " x ", height);

    /* Check if size has changed */
    int old_width, old_height;
    ScreenCapture::getDimensions(old_width, old_height);
    if ((old_width != width) || (old_height != height)) {
        ScreenCapture::reinit();

        /* We need to close the dumping if needed, and open a new one */
        if (shared_config.av_dumping) {
            debuglog(LCF_SDL | LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
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

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    if ((value_mask & CWWidth) && (value_mask & CWHeight)) {
        debuglog(LCF_SDL | LCF_WINDOW, "    New size: ", values->width, " x ", values->height);
    }

    /* Disable window movement */
    value_mask &= ~(CWX | CWY);

    int ret = orig::XConfigureWindow(display, w, value_mask, values);

    /* Check if size has changed */
    int old_width, old_height;
    ScreenCapture::getDimensions(old_width, old_height);
    if ((value_mask & CWWidth) && (value_mask & CWHeight) && ((values->width != old_width) || (values->height != old_height))) {
        ScreenCapture::reinit();

        /* We need to close the dumping if needed, and open a new one */
        if (shared_config.av_dumping) {
            debuglog(LCF_SDL | LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
            avencoder.reset(new AVEncoder());
        }
    }
    return ret;
}


}
