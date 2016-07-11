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
#include <ncurses.h>

void init_ui(void)
{
    initscr();
    raw();                 /* Line buffering disabled  */
    keypad(stdscr, TRUE);  /* We get F1, F2 etc..      */
    noecho();              /* Don't echo() while we do getch */
}

void update_ui(Context &context)
{
    mvprintw(0, 0, "Game path: %s", context.gamepath.c_str());
    mvprintw(1, 0, "Movie path: %s", context.moviefile.c_str());
    refresh();
}

