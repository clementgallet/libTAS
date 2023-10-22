/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <QtWidgets/QFormLayout>
#include "GameInfoWindow.h"
#include "MainWindow.h"

GameInfoWindow::GameInfoWindow(QWidget *parent) : QDialog(parent)
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

    qRegisterMetaType<GameInfo>("GameInfo");

    /* We need connections to the game loop, so we access it through our parent */
    MainWindow *mw = qobject_cast<MainWindow*>(parent);
    if (mw) {
        connect(mw->gameLoop, &GameLoop::gameInfoChanged, this, &GameInfoWindow::update);
    }
}

void GameInfoWindow::update(GameInfo game_info)
{
    QString videoStr;
    if (game_info.video & GameInfo::SDL1) {
        videoStr = "SDL 1";
    }
    else if (game_info.video & GameInfo::SDL2_RENDERER) {
        videoStr = "SDL 2 renderer";
    }
    else if (game_info.video & GameInfo::SDL2_SURFACE) {
        videoStr = "SDL 2 surface";
    }
    else if (game_info.video & GameInfo::SDL2) {
        videoStr = "SDL 2";
    }
    else if (game_info.video & GameInfo::VDPAU) {
        videoStr = "VDPAU";
    }
    else {
        videoStr = "unknown";
    }

    if (game_info.video & GameInfo::EGL) {
        videoStr += " (EGL)";
    }

    if (game_info.video & GameInfo::OPENGL) {
        if (game_info.opengl_major > 0) {
            QString profile;
            switch(game_info.opengl_profile) {
            case GameInfo::CORE:
                profile = " Core profile";
                break;
            case GameInfo::COMPATIBILITY:
                profile = " Compatibility profile";
                break;
            case GameInfo::ES:
                profile = " ES profile";
                break;
            default:
                break;
            }
            videoStr += QString(" (OpenGL %1.%2%3)").arg(game_info.opengl_major).arg(game_info.opengl_minor).arg(profile);
        }
        else {
            videoStr += " (OpenGL)";
        }
    }
    videoLabel->setText(videoStr);

    if (game_info.audio & GameInfo::SDL1) {
        audioLabel->setText(tr("yes (SDL 1)"));
    }
    else if (game_info.audio & GameInfo::SDL2) {
        audioLabel->setText(tr("yes (SDL 2)"));
    }
    else if (game_info.audio & GameInfo::OPENAL) {
        audioLabel->setText(tr("yes (OpenAL)"));
    }
    else if (game_info.audio & GameInfo::PULSEAUDIO) {
        audioLabel->setText(tr("yes (PulseAudio)"));
    }
    else if (game_info.audio & GameInfo::ALSA) {
        audioLabel->setText(tr("yes (ALSA)"));
    }
    else {
        audioLabel->setText(tr("unknown"));
    }

    if (game_info.keyboard & GameInfo::SDL1) {
        keyboardLabel->setText(tr("yes (SDL 1)"));
    }
    else if (game_info.keyboard & GameInfo::SDL2) {
        keyboardLabel->setText(tr("yes (SDL 2)"));
    }
    else if (game_info.keyboard & GameInfo::XCBEVENTS) {
        keyboardLabel->setText(tr("yes (xcb)"));
    }
    else if (game_info.keyboard & GameInfo::XEVENTS) {
        keyboardLabel->setText(tr("default (xevents)"));
    }
    else {
        keyboardLabel->setText(tr("unknown"));
    }

    if (game_info.mouse & GameInfo::SDL1) {
        mouseLabel->setText(tr("yes (SDL 1)"));
    }
    else if (game_info.mouse & GameInfo::SDL2) {
        mouseLabel->setText(tr("yes (SDL 2)"));
    }
    else if (game_info.mouse & GameInfo::XCBEVENTS) {
        mouseLabel->setText(tr("yes (xcb)"));
    }
    else if (game_info.mouse & GameInfo::XEVENTS) {
        mouseLabel->setText(tr("default (xevents)"));
    }
    else {
        mouseLabel->setText(tr("unknown"));
    }

    if (game_info.joystick & GameInfo::SDL1) {
        joystickLabel->setText(tr("yes (SDL 1)"));
    }
    else if (game_info.joystick & GameInfo::SDL2) {
        joystickLabel->setText(tr("yes (SDL 2)"));
    }
    else if (game_info.joystick & GameInfo::JSDEV) {
        joystickLabel->setText(tr("yes (jsdev)"));
    }
    else {
        joystickLabel->setText(tr("unknown"));
    }
}
