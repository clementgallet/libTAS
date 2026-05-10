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

#include "WindowManager.h"

#include "MainWindow.h"
#include "EncodeWindow.h"
#include "ExecutableWindow.h"
#include "InputWindow.h"
#include "ControllerTabWindow.h"
#include "GameInfoWindow.h"
#include "RamSearchWindow.h"
#include "RamWatchWindow.h"
#include "RamWatchView.h"
#include "HexViewWindow.h"
#include "InputEditorWindow.h"
#include "InputEditorView.h"
#include "InputEditorModel.h"
#include "AnnotationsWindow.h"
#include "TimeTraceWindow.h"
#include "TimeTraceModel.h"
#include "LuaConsoleWindow.h"
#include "MovieSettingsWindow.h"
#include "settings/SettingsWindow.h"

#include "GameEvents.h"
#include "GameLoop.h"
#include "Context.h"
#include "movie/MovieFile.h"
#include "../shared/SharedConfig.h"

#include <QtCore/QObject>
#include <QtWidgets/QWidget>

WindowManager::WindowManager(MainWindow *owner, Context *context, GameLoop *gameLoop)
    : owner(owner), context(context), gameLoop(gameLoop)
{
}

SettingsWindow *WindowManager::settingsWindow()
{
    if (!settingsWindow_) {
        settingsWindow_ = new SettingsWindow(context, owner);
        settingsWindow_->loadConfig();
        settingsWindow_->update(context->status);
    }

    return settingsWindow_;
}

EncodeWindow *WindowManager::encodeWindow()
{
    if (!encodeWindow_) {
        encodeWindow_ = new EncodeWindow(context, owner);
        encodeWindow_->update_config();
    }

    return encodeWindow_;
}

InputWindow *WindowManager::inputWindow()
{
    if (!inputWindow_) {
        inputWindow_ = new InputWindow(context, owner);
        inputWindow_->update();
    }

    return inputWindow_;
}

ExecutableWindow *WindowManager::executableWindow()
{
    if (!executableWindow_) {
        executableWindow_ = new ExecutableWindow(context, owner);
        executableWindow_->update_config();
    }

    return executableWindow_;
}

ControllerTabWindow *WindowManager::controllerTabWindow()
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

GameInfoWindow *WindowManager::gameInfoWindow()
{
    if (!gameInfoWindow_) {
        gameInfoWindow_ = new GameInfoWindow(owner);
        QObject::connect(gameLoop, &GameLoop::gameInfoChanged,
                         gameInfoWindow_, &GameInfoWindow::update,
                         Qt::QueuedConnection);
    }

    return gameInfoWindow_;
}

HexViewWindow *WindowManager::hexViewWindow()
{
    if (!hexViewWindow_) {
        hexViewWindow_ = new HexViewWindow(owner);
        if (context->status != Context::INACTIVE) {
            hexViewWindow_->start();
        }
    }

    return hexViewWindow_;
}

RamWatchWindow *WindowManager::ramWatchWindow()
{
    if (!ramWatchWindow_) {
        ramWatchWindow_ = new RamWatchWindow(context, hexViewWindow(), owner);
    }

    return ramWatchWindow_;
}

RamSearchWindow *WindowManager::ramSearchWindow()
{
    if (!ramSearchWindow_) {
        ramSearchWindow_ = new RamSearchWindow(context, hexViewWindow(), ramWatchWindow(), owner);
    }

    return ramSearchWindow_;
}

InputEditorWindow *WindowManager::inputEditorWindow()
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

AnnotationsWindow *WindowManager::annotationsWindow()
{
    if (!annotationsWindow_) {
        annotationsWindow_ = new AnnotationsWindow(context, &gameLoop->movie, owner);
        if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
            annotationsWindow_->update();
        }
    }

    return annotationsWindow_;
}

TimeTraceWindow *WindowManager::timeTraceWindow()
{
    if (!timeTraceWindow_) {
        timeTraceWindow_ = new TimeTraceWindow(context, owner);
    }

    return timeTraceWindow_;
}

LuaConsoleWindow *WindowManager::luaConsoleWindow()
{
    if (!luaConsoleWindow_) {
        luaConsoleWindow_ = new LuaConsoleWindow(context, owner);
    }

    return luaConsoleWindow_;
}

MovieSettingsWindow *WindowManager::movieSettingsWindow()
{
    if (!movieSettingsWindow_) {
        movieSettingsWindow_ = new MovieSettingsWindow(context, &gameLoop->movie, owner);
        movieSettingsWindow_->loadConfig();
    }

    return movieSettingsWindow_;
}

void WindowManager::showWindow(QWidget *window)
{
    window->show();
    window->raise();
    window->activateWindow();
}

void WindowManager::showExecutableWindow()
{
    executableWindow()->exec();
}

void WindowManager::openRuntimeSettingsWindow()
{
    settingsWindow()->openRuntimeTab();
}

void WindowManager::openMovieSettingsTab()
{
    settingsWindow()->openMovieTab();
}

void WindowManager::openInputSettingsTab()
{
    settingsWindow()->openInputTab();
}

void WindowManager::openAudioSettingsTab()
{
    settingsWindow()->openAudioTab();
}

void WindowManager::openVideoSettingsTab()
{
    settingsWindow()->openVideoTab();
}

void WindowManager::openDebugSettingsTab()
{
    settingsWindow()->openDebugTab();
}

void WindowManager::openGameSpecificSettingsTab()
{
    settingsWindow()->openGameSpecificTab();
}

void WindowManager::openPathSettingsTab()
{
    settingsWindow()->openPathTab();
}

void WindowManager::showMovieSettingsWindow()
{
    movieSettingsWindow()->exec();
}

void WindowManager::showAnnotationsWindow()
{
    showWindow(annotationsWindow());
}

void WindowManager::showInputEditorWindow()
{
    showWindow(inputEditorWindow());
}

void WindowManager::showEncodeWindow()
{
    encodeWindow()->exec();
}

void WindowManager::showGameInfoWindow()
{
    gameInfoWindow()->exec();
}

void WindowManager::showRamSearchWindow()
{
    showWindow(ramSearchWindow());
}

void WindowManager::showRamWatchWindow()
{
    showWindow(ramWatchWindow());
}

void WindowManager::showHexViewWindow()
{
    showWindow(hexViewWindow());
}

void WindowManager::showLuaConsoleWindow()
{
    showWindow(luaConsoleWindow());
}

void WindowManager::showTimeTraceWindow()
{
    showWindow(timeTraceWindow());
}

void WindowManager::showInputWindow()
{
    inputWindow()->exec();
}

void WindowManager::showControllerTabWindow()
{
    showWindow(controllerTabWindow());
}

bool WindowManager::isInputEditorVisible() const
{
    return inputEditorWindow_ && inputEditorWindow_->isVisible();
}

void WindowManager::startHexViewIfOpen()
{
    if (hexViewWindow_) {
        hexViewWindow_->start();
    }
}

void WindowManager::updateFrequentWindowViews()
{
    if (ramSearchWindow_ && ramSearchWindow_->isVisible()) {
        ramSearchWindow_->update();
    }
    if (ramWatchWindow_) {
        ramWatchWindow_->update();
    }
    if (inputEditorWindow_) {
        inputEditorWindow_->update();
    }
    if (hexViewWindow_ && hexViewWindow_->isVisible()) {
        hexViewWindow_->update();
    }
}

void WindowManager::updateFrameWindowViews()
{
    if (ramWatchWindow_) {
        ramWatchWindow_->update_frozen();
    }
}

void WindowManager::refreshMovieLoadedWindows()
{
    if (annotationsWindow_) {
        annotationsWindow_->update();
    }
}

void WindowManager::refreshMovieUnloadedWindows()
{
    if (annotationsWindow_) {
        annotationsWindow_->clear();
    }
}

void WindowManager::resetInputEditorWindow()
{
    if (inputEditorWindow_) {
        inputEditorWindow_->resetInputs();
    }
}

void WindowManager::reloadConfigWindows()
{
    if (encodeWindow_) {
        encodeWindow_->update_config();
    }
    if (executableWindow_) {
        executableWindow_->update_config();
    }
    if (inputWindow_) {
        inputWindow_->update();
    }
    if (inputEditorWindow_) {
        inputEditorWindow_->update_config();
    }
    if (settingsWindow_) {
        settingsWindow_->loadConfig();
    }
}

void WindowManager::getRamWatch(std::string &watch) const
{
    if (ramWatchWindow_) {
        ramWatchWindow_->ramWatchView->slotGet(watch);
    }
    else {
        watch.clear();
    }
}

void WindowManager::getMarkerText(std::string &text) const
{
    if (inputEditorWindow_) {
        inputEditorWindow_->inputEditorView->getCurrentMarkerText(text);
    }
    else {
        text.clear();
    }
}

void WindowManager::registerSavestate(int slot, unsigned long long frame)
{
    if (inputEditorWindow_) {
        inputEditorWindow_->inputEditorView->inputEditorModel->registerSavestate(slot, frame);
    }
}

void WindowManager::addTimeTrace(int type, unsigned long long hash, std::string stacktrace)
{
    if (timeTraceWindow_) {
        timeTraceWindow_->timeTraceModel->addCall(type, hash, stacktrace);
    }
}

void WindowManager::updateSettingsWindow(int status)
{
    if (settingsWindow_) {
        settingsWindow_->update(status);
    }
}