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
        "Inputs",
        "Exit",
    };
    int n_items = 4;
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
                    unpost_menu(menu);
                    ui_inputs_menu();
                    post_menu(menu);
                    refresh();
                }
                if (index == 3) {
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
    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);

    /* Empty event queue */
    while (XPending (display)) {
        XNextEvent(display, &event);
    }

    while (1) {
        XNextEvent(display, &event);
        if (event.type == KeyPress)
        {
            ui_print("KeyPress event");
            KeyCode kc = event.xkey.keycode;
            ui_print("KeyCode is %d", kc);
            KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);
            return ks;
        }
    }
    return 0;
}

void ui_rebuild_menu(MENU *menu, ITEM **menu_items, ITEM *cur)
{
    unpost_menu(menu);
    set_menu_items(menu, menu_items);
    post_menu(menu);
    set_current_item(menu, cur);
    refresh();
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

    const char * menu_choices[HOTKEY_LEN+1];
    for (int i=0; i<HOTKEY_LEN; i++)
        menu_choices[i] = hotkey_names[i].c_str();
    menu_choices[HOTKEY_LEN] = "Exit";

    ITEM **menu_items = (ITEM**) calloc(HOTKEY_LEN+2, sizeof(ITEM*));
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
                ui_rebuild_menu(menu, menu_items, menu_items[index]);
                ui_print("Waiting for key pressed");
                KeySym ks = get_next_keypressed();
                free_item(menu_items[index]);
                config.hotkeys[index] = ks;
                menu_items[index] = new_item(menu_choices[index], XKeysymToString(ks));
                ui_rebuild_menu(menu, menu_items, menu_items[index]);
                break;

        }
    }
    unpost_menu(menu);
    free_menu(menu);
    for (int i=0; i<HOTKEY_LEN+1; i++)
        free_item(menu_items[i]);

}

void ui_inputs_menu()
{

    int n_items = config.input_list.size();
    const char * menu_choices[n_items+1];
    for (int i=0; i<n_items; i++)
        menu_choices[i] = config.input_list[i].description.c_str();
    menu_choices[n_items] = "Exit";

    ITEM **menu_items = (ITEM**) calloc(n_items+2, sizeof(ITEM*));
    for (int i=0; i<n_items; i++) {
        const char* mapstr = "<none>";
        for (std::map<KeySym,SingleInput>::iterator iter = config.input_mapping.begin(); iter != config.input_mapping.end(); ++iter) {
            KeySym ks = iter->first;
            SingleInput si = iter->second;
            if (si == config.input_list[i]) {
                mapstr = XKeysymToString(ks);
                break;
            }
            if ((si.type == IT_ID) && (config.input_list[i].type == IT_KEYBOARD) && (config.input_list[i].value == ks)) {
                mapstr = "<self>";
                break;
            }

        }
        menu_items[i] = new_item(menu_choices[i], mapstr);
    }
    menu_items[n_items] = new_item("Exit", "");
    menu_items[n_items+1] = (ITEM*) NULL;

    MENU *menu = new_menu(menu_items);
    post_menu(menu);
    refresh();

    int c;
    bool end = false;
    ITEM *cur;
    int index;
    KeySym ks;
    SingleInput si;
    while(!end) {
        c = getch();
        switch(c) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case KEY_NPAGE:
                menu_driver(menu, REQ_SCR_DPAGE);
                break;
            case KEY_PPAGE:
                menu_driver(menu, REQ_SCR_UPAGE);
                break;
            case KEY_END:
                menu_driver(menu, REQ_LAST_ITEM);
                break;
            case KEY_HOME:
                menu_driver(menu, REQ_FIRST_ITEM);
                break;
            case KEY_F(1): // self
                cur = current_item(menu);
                index = item_index(cur);
                if (index == n_items) {
                    break;
                }
                ks = config.input_list[index].value;
                config.input_mapping.erase(ks);
                si.type = IT_ID;
                config.input_mapping[ks] = si;
                free_item(menu_items[index]);
                menu_items[index] = new_item(menu_choices[index], "<self>");
                ui_rebuild_menu(menu, menu_items, menu_items[index]);

                break;
            case KEY_F(2): // none
                cur = current_item(menu);
                index = item_index(cur);
                if (index == n_items) {
                    break;
                }

                for (std::map<KeySym,SingleInput>::iterator iter = config.input_mapping.begin(); iter != config.input_mapping.end(); ++iter) {
                    ks = iter->first;
                    si = iter->second;
                    if (si == config.input_list[index]) {
                        config.input_mapping.erase(iter);
                        free_item(menu_items[index]);
                        menu_items[index] = new_item(menu_choices[index], "<none>");
                        ui_rebuild_menu(menu, menu_items, menu_items[index]);
                        break;
                    }
                }
                break;
            case 10:
                cur = current_item(menu);
                index = item_index(cur);
                if (index == n_items) {
                    end = true;
                    break;
                }

                free_item(menu_items[index]);
                menu_items[index] = new_item(menu_choices[index], "<press key>");
                ui_rebuild_menu(menu, menu_items, menu_items[index]);
                ui_print("Waiting for key pressed");
                ks = get_next_keypressed();
                free_item(menu_items[index]);
                config.input_mapping[ks] = config.input_list[index];
                menu_items[index] = new_item(menu_choices[index], XKeysymToString(ks));
                ui_rebuild_menu(menu, menu_items, menu_items[index]);
                break;
        }
    }
    unpost_menu(menu);
    free_menu(menu);
    for (int i=0; i<n_items+1; i++)
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

