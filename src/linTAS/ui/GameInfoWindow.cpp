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

#include "GameInfoWindow.h"

GameInfoWindow::GameInfoWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(300, 150, "Game information");

    video_box = new Fl_Box(10, 10, 200, 30);
    video_box->box(FL_NO_BOX);
    video_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    audio_box = new Fl_Box(10, 50, 200, 30);
    audio_box->box(FL_NO_BOX);
    audio_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    joystick_box = new Fl_Box(10, 90, 200, 30);
    joystick_box->box(FL_NO_BOX);
    joystick_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    window->end();
}

void GameInfoWindow::update()
{
    std::string videostr = "Video support: ";
    if (context->game_info.video & GameInfo::SDL1) {
        videostr += "SDL 1";
    }
    else if (context->game_info.video & GameInfo::SDL2) {
        videostr += "SDL 2";
    }
    else {
        videostr += "unknown";
    }
    video_box->copy_label(videostr.c_str());

    std::string audiostr = "Audio support: ";
    if (context->game_info.audio & GameInfo::SDL1) {
        audiostr += "yes (SDL 1)";
    }
    else if (context->game_info.audio & GameInfo::SDL2) {
        audiostr += "yes (SDL 2)";
    }
    else if (context->game_info.audio & GameInfo::OPENAL) {
        audiostr += "yes (OpenAL)";
    }
    else if (context->game_info.audio & GameInfo::PULSEAUDIO) {
        audiostr += "yes (PulseAudio)";
    }
    else {
        audiostr += "unknown";
    }
    audio_box->copy_label(audiostr.c_str());

    std::string joystickstr = "Joystick support: ";
    if (context->game_info.joystick & GameInfo::SDL1) {
        joystickstr += "yes (SDL 1)";
    }
    else if (context->game_info.joystick & GameInfo::SDL2) {
        joystickstr += "yes (SDL 2)";
    }
    else if (context->game_info.joystick & GameInfo::NO_SDL) {
        joystickstr += "no";
    }
    else {
        joystickstr += "unknown";
    }
    joystick_box->copy_label(joystickstr.c_str());

}
