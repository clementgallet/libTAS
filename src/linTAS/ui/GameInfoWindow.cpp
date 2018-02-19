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

#include <QFormLayout>
#include "GameInfoWindow.h"

GameInfoWindow::GameInfoWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Game information");

    videoLabel = new QLabel(tr("unknown"));
    audioLabel = new QLabel(tr("unknown"));
    keyboardLabel = new QLabel(tr("unknown"));
    mouseLabel = new QLabel(tr("unknown"));
    joystickLabel = new QLabel(tr("unknown"));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(new QLabel(tr("Video support:")), videoLabel);
    layout->addRow(new QLabel(tr("Audio support:")), audioLabel);
    layout->addRow(new QLabel(tr("Keyboard support:")), keyboardLabel);
    layout->addRow(new QLabel(tr("Mouse support:")), mouseLabel);
    layout->addRow(new QLabel(tr("Joystick support:")), joystickLabel);
    setLayout(layout);
}

void GameInfoWindow::update()
{
    QString videoStr;
    if (context->game_info.video & GameInfo::SDL1) {
        videoStr = "SDL 1";
    }
    else if (context->game_info.video & GameInfo::SDL2) {
        videoStr = "SDL 2";
    }
    else {
        videoStr = "unknown";
    }

    if (context->game_info.video & GameInfo::OPENGL) {
        QString profile;
        switch(context->game_info.opengl_profile) {
        case GameInfo::CORE:
            profile = "Core profile";
            break;
        case GameInfo::COMPATIBILITY:
            profile = "Compatibility profile";
            break;
        case GameInfo::ES:
            profile = "ES profile";
            break;
        default:
            break;
        }
        videoStr += QString(" (OpenGL %1.%2 %3)").arg(context->game_info.opengl_major).arg(context->game_info.opengl_minor).arg(profile);
    }
    videoLabel->setText(videoStr);

    if (context->game_info.audio & GameInfo::SDL1) {
        audioLabel->setText(tr("yes (SDL 1)"));
    }
    else if (context->game_info.audio & GameInfo::SDL2) {
        audioLabel->setText(tr("yes (SDL 2)"));
    }
    else if (context->game_info.audio & GameInfo::OPENAL) {
        audioLabel->setText(tr("yes (OpenAL)"));
    }
    else if (context->game_info.audio & GameInfo::PULSEAUDIO) {
        audioLabel->setText(tr("yes (PulseAudio)"));
    }
    else if (context->game_info.audio & GameInfo::ALSA) {
        audioLabel->setText(tr("yes (ALSA)"));
    }
    else {
        audioLabel->setText(tr("unknown"));
    }

    if (context->game_info.keyboard & GameInfo::SDL1) {
        keyboardLabel->setText(tr("yes (SDL 1)"));
    }
    else if (context->game_info.keyboard & GameInfo::SDL2) {
        keyboardLabel->setText(tr("yes (SDL 2)"));
    }
    else if (context->game_info.keyboard & GameInfo::XEVENTS) {
        keyboardLabel->setText(tr("default (xevents)"));
    }
    else {
        keyboardLabel->setText(tr("unknown"));
    }

    if (context->game_info.mouse & GameInfo::SDL1) {
        mouseLabel->setText(tr("yes (SDL 1)"));
    }
    else if (context->game_info.mouse & GameInfo::SDL2) {
        mouseLabel->setText(tr("yes (SDL 2)"));
    }
    else if (context->game_info.mouse & GameInfo::XEVENTS) {
        mouseLabel->setText(tr("default (xevents)"));
    }
    else {
        mouseLabel->setText(tr("unknown"));
    }

    if (context->game_info.joystick & GameInfo::SDL1) {
        joystickLabel->setText(tr("yes (SDL 1)"));
    }
    else if (context->game_info.joystick & GameInfo::SDL2) {
        joystickLabel->setText(tr("yes (SDL 2)"));
    }
    else if (context->game_info.joystick & GameInfo::JSDEV) {
        joystickLabel->setText(tr("yes (jsdev)"));
    }
    else {
        joystickLabel->setText(tr("unknown"));
    }
}
