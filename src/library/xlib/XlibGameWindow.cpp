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
#include "XlibGameWindow.h"

#include "logging.h"
#include "screencapture/ScreenCapture.h"
#include "global.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"

namespace libtas {

/* Window identifiers */
std::list<Window> gameXWindows;

void XlibGameWindow::push(Window w)
{
    /* Saving top-level window */
    if (gameXWindows.empty())
        LOG(LL_DEBUG, LCF_WINDOW, "   set game window to %d", w);
    gameXWindows.push_back(w);
}

void XlibGameWindow::promote(Window w)
{
    for (auto iter = gameXWindows.begin(); iter != gameXWindows.end(); iter++) {
        if (w == *iter) {
            gameXWindows.erase(iter);
            gameXWindows.push_front(w);
            sendId(w);
            break;
        }
    }
}

Window XlibGameWindow::get()
{
    if (gameXWindows.empty())
        return 0;
    return gameXWindows.front();
}

void XlibGameWindow::pop(Window w)
{
    /* If current game window, switch to another one on the list */
    if (!gameXWindows.empty() && w == gameXWindows.front()) {
        ScreenCapture::fini();

        gameXWindows.pop_front();
        if (gameXWindows.empty()) {
            /* Tells the program we don't have a window anymore to gather inputs */
            sendId(0);
        }
        else if (!Global::is_exiting) {
            /* Switch to the next game window */
            LOG(LL_DEBUG, LCF_WINDOW, "   set game window to %d", gameXWindows.front());
            sendId(gameXWindows.front());
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
}

void XlibGameWindow::sendId(Window w)
{
    uint32_t i = (uint32_t)w;
    lockSocket();
    sendMessage(MSGB_WINDOW_ID);
    sendData(&i, sizeof(i));
    unlockSocket();
    LOG(LL_DEBUG, LCF_WINDOW, "Sent X11 window id %d", w);
}

bool XlibGameWindow::isRootWindow(Display *display, Window w)
{
    return w == DefaultRootWindow(display);
}

bool XlibGameWindow::isRootWindow(xcb_connection_t *c, Window w)
{
    xcb_screen_t *s = xcb_setup_roots_iterator (xcb_get_setup (c)).data;
    return s->root == w;
}

bool XlibGameWindow::isTopLevel(Display *display, Window w)
{
    Window parent_return = 0;
    Window *children_return = nullptr;
    unsigned int nchildren_return = 0;
    Window root_window;
    XQueryTree(display, w, &root_window, &parent_return, &children_return, &nchildren_return);
    if (children_return) XFree(children_return);
    
    return isRootWindow(display, parent_return);
}


}
