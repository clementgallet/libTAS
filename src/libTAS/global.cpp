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

#include "global.h"

SharedConfig libtas::shared_config;
GameInfo libtas::game_info;
volatile bool libtas::is_exiting = false;
bool libtas::skipping_draw = false;
Display* libtas::gameDisplay = nullptr;
Window libtas::gameXWindow = 0;
SDL_Window* libtas::gameSDLWindow = nullptr;
