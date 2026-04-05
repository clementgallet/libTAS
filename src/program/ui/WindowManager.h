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

#ifndef LIBTAS_WINDOWMANAGER_H_INCLUDED
#define LIBTAS_WINDOWMANAGER_H_INCLUDED

#include "MainWindow.h"
#include "EncodeWindow.h"
#include "ExecutableWindow.h"
#include "InputWindow.h"
#include "ControllerTabWindow.h"
#include "GameInfoWindow.h"
#include "RamSearchWindow.h"
#include "RamWatchWindow.h"
#include "HexViewWindow.h"
#include "InputEditorWindow.h"
#include "InputEditorView.h"
#include "AnnotationsWindow.h"
#include "TimeTraceWindow.h"
#include "LuaConsoleWindow.h"
#include "MovieSettingsWindow.h"
#include "settings/SettingsWindow.h"
#include "GameEvents.h"

#include <QtCore/QObject>
#include <QtWidgets/QWidget>

class WindowManager {
public:
    WindowManager(MainWindow *owner, Context *context, GameLoop *gameLoop)
        : owner(owner), context(context), gameLoop(gameLoop)
    {
    }

    SettingsWindow *settingsWindow()
    {
        if (!settingsWindow_) {
            settingsWindow_ = new SettingsWindow(context, owner);
            settingsWindow_->loadConfig();
            settingsWindow_->update(context->status);
        }

        return settingsWindow_;
    }

    EncodeWindow *encodeWindow()
    {
        if (!encodeWindow_) {
            encodeWindow_ = new EncodeWindow(context, owner);
            encodeWindow_->update_config();
        }

        return encodeWindow_;
    }

    InputWindow *inputWindow()
    {
        if (!inputWindow_) {
            inputWindow_ = new InputWindow(context, owner);
            inputWindow_->update();
        }

        return inputWindow_;
    }

    ExecutableWindow *executableWindow()
    {
        if (!executableWindow_) {
            executableWindow_ = new ExecutableWindow(context, owner);
            executableWindow_->update_config();
        }

        return executableWindow_;
    }

    ControllerTabWindow *controllerTabWindow()
    {
        if (!controllerTabWindow_) {
            controllerTabWindow_ = new ControllerTabWindow(context, owner);
            QObject::connect(gameLoop->gameEvents, &GameEvents::controllerButtonToggled,
                     controllerTabWindow_, &ControllerTabWindow::slotButtonToggle,
                     Qt::QueuedConnection);
            QObject::connect(gameLoop, &GameLoop::fillControllerInputs,
                     controllerTabWindow_, &ControllerTabWindow::slotSetInputs,
                     Qt::DirectConnection);
            QObject::connect(gameLoop, &GameLoop::showControllerInputs,
                     controllerTabWindow_, &ControllerTabWindow::slotGetInputs,
                     Qt::QueuedConnection);
        }

        return controllerTabWindow_;
    }

    GameInfoWindow *gameInfoWindow()
    {
        if (!gameInfoWindow_) {
            gameInfoWindow_ = new GameInfoWindow(owner);
            QObject::connect(gameLoop, &GameLoop::gameInfoChanged,
                             gameInfoWindow_, &GameInfoWindow::update,
                             Qt::QueuedConnection);
        }

        return gameInfoWindow_;
    }

    HexViewWindow *hexViewWindow()
    {
        if (!hexViewWindow_) {
            hexViewWindow_ = new HexViewWindow(owner);
            if (context->status != Context::INACTIVE) {
                hexViewWindow_->start();
            }
        }

        return hexViewWindow_;
    }

    RamWatchWindow *ramWatchWindow()
    {
        if (!ramWatchWindow_) {
            ramWatchWindow_ = new RamWatchWindow(context, hexViewWindow(), owner);
        }

        return ramWatchWindow_;
    }

    RamSearchWindow *ramSearchWindow()
    {
        if (!ramSearchWindow_) {
            ramSearchWindow_ = new RamSearchWindow(context, hexViewWindow(), ramWatchWindow(), owner);
        }

        return ramSearchWindow_;
    }

    InputEditorWindow *inputEditorWindow()
    {
        if (!inputEditorWindow_) {
            inputEditorWindow_ = new InputEditorWindow(context, &gameLoop->movie, owner);
            QObject::connect(inputEditorWindow_->inputEditorView, &InputEditorView::saveMovieRequested,
                             owner, &MainWindow::slotSaveMovie,
                             Qt::UniqueConnection);
            inputEditorWindow_->resetInputs();
            inputEditorWindow_->update();
        }

        return inputEditorWindow_;
    }

    AnnotationsWindow *annotationsWindow()
    {
        if (!annotationsWindow_) {
            annotationsWindow_ = new AnnotationsWindow(context, &gameLoop->movie, owner);
            if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
                annotationsWindow_->update();
            }
        }

        return annotationsWindow_;
    }

    TimeTraceWindow *timeTraceWindow()
    {
        if (!timeTraceWindow_) {
            timeTraceWindow_ = new TimeTraceWindow(context, owner);
        }

        return timeTraceWindow_;
    }

    LuaConsoleWindow *luaConsoleWindow()
    {
        if (!luaConsoleWindow_) {
            luaConsoleWindow_ = new LuaConsoleWindow(context, owner);
        }

        return luaConsoleWindow_;
    }

    MovieSettingsWindow *movieSettingsWindow()
    {
        if (!movieSettingsWindow_) {
            movieSettingsWindow_ = new MovieSettingsWindow(context, &gameLoop->movie, owner);
            movieSettingsWindow_->loadConfig();
        }

        return movieSettingsWindow_;
    }

    SettingsWindow *existingSettingsWindow() const { return settingsWindow_; }
    EncodeWindow *existingEncodeWindow() const { return encodeWindow_; }
    InputWindow *existingInputWindow() const { return inputWindow_; }
    ExecutableWindow *existingExecutableWindow() const { return executableWindow_; }
    RamSearchWindow *existingRamSearchWindow() const { return ramSearchWindow_; }
    RamWatchWindow *existingRamWatchWindow() const { return ramWatchWindow_; }
    InputEditorWindow *existingInputEditorWindow() const { return inputEditorWindow_; }
    AnnotationsWindow *existingAnnotationsWindow() const { return annotationsWindow_; }
    HexViewWindow *existingHexViewWindow() const { return hexViewWindow_; }

    void showWindow(QWidget *window)
    {
        window->show();
        window->raise();
        window->activateWindow();
    }

    void showInputEditorWindow()
    {
        showWindow(inputEditorWindow());
    }

    void showRamSearchWindow()
    {
        showWindow(ramSearchWindow());
    }

    void showRamWatchWindow()
    {
        showWindow(ramWatchWindow());
    }

    void showHexViewWindow()
    {
        showWindow(hexViewWindow());
    }

    void showLuaConsoleWindow()
    {
        showWindow(luaConsoleWindow());
    }

    void showTimeTraceWindow()
    {
        showWindow(timeTraceWindow());
    }

    void showControllerTabWindow()
    {
        showWindow(controllerTabWindow());
    }

    void showAnnotationsWindow()
    {
        showWindow(annotationsWindow());
    }

    bool isInputEditorVisible() const
    {
        return inputEditorWindow_ && inputEditorWindow_->isVisible();
    }

    void getRamWatch(std::string &watch) const
    {
        if (ramWatchWindow_) {
            ramWatchWindow_->ramWatchView->slotGet(watch);
        }
        else {
            watch.clear();
        }
    }

    void getMarkerText(std::string &text) const
    {
        if (inputEditorWindow_) {
            inputEditorWindow_->inputEditorView->getCurrentMarkerText(text);
        }
        else {
            text.clear();
        }
    }

    void registerSavestate(int slot, unsigned long long frame)
    {
        if (inputEditorWindow_) {
            inputEditorWindow_->inputEditorView->inputEditorModel->registerSavestate(slot, frame);
        }
    }

    void addTimeTrace(int type, unsigned long long hash, std::string stacktrace)
    {
        if (timeTraceWindow_) {
            timeTraceWindow_->timeTraceModel->addCall(type, hash, stacktrace);
        }
    }

    void updateSettingsWindow(int status)
    {
        if (settingsWindow_) {
            settingsWindow_->update(status);
        }
    }

private:
    MainWindow *owner;
    Context *context;
    GameLoop *gameLoop;

    SettingsWindow *settingsWindow_ = nullptr;
    EncodeWindow *encodeWindow_ = nullptr;
    InputWindow *inputWindow_ = nullptr;
    ExecutableWindow *executableWindow_ = nullptr;
    ControllerTabWindow *controllerTabWindow_ = nullptr;
    GameInfoWindow *gameInfoWindow_ = nullptr;
    RamSearchWindow *ramSearchWindow_ = nullptr;
    RamWatchWindow *ramWatchWindow_ = nullptr;
    InputEditorWindow *inputEditorWindow_ = nullptr;
    AnnotationsWindow *annotationsWindow_ = nullptr;
    TimeTraceWindow *timeTraceWindow_ = nullptr;
    LuaConsoleWindow *luaConsoleWindow_ = nullptr;
    MovieSettingsWindow *movieSettingsWindow_ = nullptr;
    HexViewWindow *hexViewWindow_ = nullptr;
};

#endif