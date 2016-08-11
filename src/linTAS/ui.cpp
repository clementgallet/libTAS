/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ui.h"
#include "main.h"
#include <ncurses.h>
#include <menu.h>
#include <cstdarg>
#include <pthread.h>
#include <signal.h>
#include <X11/Xlib.h>
#include "../shared/Config.h"
#include <X11/XKBlib.h>

Display *display;

void ui_init(void)
{
    initscr();
    raw();                 /* Line buffering disabled  */
    keypad(stdscr, TRUE);  /* We get F1, F2 etc..      */
    noecho();              /* Don't echo() while we do getch */

    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        ui_print("Cannot open display");
        exit(1);
    }
}

void ui_update_nogame(Context &context)
{
    pthread_t t_game;

    mvprintw(0, 0, "Game path: %s", context.gamepath.c_str());
    mvprintw(1, 0, "Movie path: %s", context.moviefile.c_str());

    const char * const menu_choices[] = {
        "Start",
        "Hotkeys",
        "Exit",
    };
    int n_items = 3;
    ITEM **menu_items = (ITEM**) calloc(n_items+1, sizeof(ITEM*));
    for (int i=0; i<n_items; i++)
        menu_items[i] = new_item(menu_choices[i], "");
    menu_items[n_items] = (ITEM*) NULL;

    MENU *menu = new_menu(menu_items);
    post_menu(menu);
    refresh();

    int c;
    bool end = false;
    while(!end) {
        c = getch();
        switch(c) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case 10:
                ITEM *cur = current_item(menu);
                int index = item_index(cur);
                if (index == 0) {
                    if (quit) {
                        pthread_create(&t_game,NULL,launchGame,NULL);
                    }
                    else {
                        quit = true;
                        pthread_join(t_game, NULL);
                    }
                }
                if (index == 1) {
                    unpost_menu(menu);
                    ui_hotkeys_menu();
                    post_menu(menu);
                    refresh();
                }
                if (index == 2) {
                    end = true;
                }
                break;
        }
    }

    unpost_menu(menu);
    free_menu(menu);
    for (int i=0; i<n_items; i++)
        free_item(menu_items[i]);

    ui_end();
}

static KeySym get_next_keypressed()
{
    Window window;
    XEvent event;
    int revert;
    XGetInputFocus(display, &window, &revert);
    XSelectInput(display, window, KeyPressMask);
    while (1) {
        XNextEvent(display, &event);
        if (event.type == KeyPress)
        {
            KeyCode kc = event.xkey.keycode;
            KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);
            return ks;
        }
    }
    return 0;
}

void ui_hotkeys_menu()
{

    std::string hotkey_names[HOTKEY_LEN];
    hotkey_names[HOTKEY_PLAYPAUSE] = "Play/Pause";
    hotkey_names[HOTKEY_FRAMEADVANCE] = "Frame Advance";
    hotkey_names[HOTKEY_FASTFORWARD] = "Fast-forward";
    hotkey_names[HOTKEY_READWRITE] = "Toggle ReadWrite/ReadOnly";
    hotkey_names[HOTKEY_SAVESTATE] = "Save State";
    hotkey_names[HOTKEY_LOADSTATE] = "Load State";

    const char * menu_choices[HOTKEY_LEN+2];
    for (int i=0; i<HOTKEY_LEN; i++)
        menu_choices[i] = hotkey_names[i].c_str();
    menu_choices[HOTKEY_LEN] = "Exit";

    ITEM **menu_items = (ITEM**) calloc(HOTKEY_LEN+1, sizeof(ITEM*));
    for (int i=0; i<HOTKEY_LEN; i++)
        menu_items[i] = new_item(menu_choices[i], XKeysymToString(config.hotkeys[i]));
    menu_items[HOTKEY_LEN] = new_item(menu_choices[HOTKEY_LEN], "");
    menu_items[HOTKEY_LEN+1] = (ITEM*) NULL;

    MENU *menu = new_menu(menu_items);
    post_menu(menu);
    refresh();

    int c;
    bool end = false;
    while(!end) {
        c = getch();
        switch(c) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case 10:
                ITEM *cur = current_item(menu);
                int index = item_index(cur);
                if (index == HOTKEY_LEN) {
                    end = true;
                    break;
                }

                free_item(menu_items[index]);
                menu_items[index] = new_item(menu_choices[index], "<press key>");
                KeySym ks = get_next_keypressed();
                free_item(menu_items[index]);
                config.hotkeys[index] = ks;
                menu_items[index] = new_item(menu_choices[index], XKeysymToString(ks));

        }
    }
    unpost_menu(menu);
    free_menu(menu);
    for (int i=0; i<HOTKEY_LEN+1; i++)
        free_item(menu_items[i]);

}


void ui_print(const char* msg, ...)
{
    int row, col;
    getmaxyx(stdscr, row, col);
    move(row-1, 0);

    va_list args;
    va_start(args, msg);
    vwprintw(stdscr, msg, args);
    va_end(args);
    refresh();
}

void ui_end(void)
{
    endwin();
}

