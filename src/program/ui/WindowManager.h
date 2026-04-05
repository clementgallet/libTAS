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

#include <string>

class QWidget;
class MainWindow;
class GameLoop;
struct Context;
class SettingsWindow;
class EncodeWindow;
class InputWindow;
class ExecutableWindow;
class ControllerTabWindow;
class GameInfoWindow;
class RamSearchWindow;
class RamWatchWindow;
class InputEditorWindow;
class AnnotationsWindow;
class TimeTraceWindow;
class LuaConsoleWindow;
class MovieSettingsWindow;
class HexViewWindow;

class WindowManager {
public:
    WindowManager(MainWindow *owner, Context *context, GameLoop *gameLoop);

    void showExecutableWindow();
    void openRuntimeSettingsWindow();
    void openMovieSettingsTab();
    void openInputSettingsTab();
    void openAudioSettingsTab();
    void openVideoSettingsTab();
    void openDebugSettingsTab();
    void openGameSpecificSettingsTab();
    void openPathSettingsTab();
    void showMovieSettingsWindow();
    void showAnnotationsWindow();
    void showInputEditorWindow();
    void showEncodeWindow();
    void showGameInfoWindow();
    void showRamSearchWindow();
    void showRamWatchWindow();
    void showHexViewWindow();
    void showLuaConsoleWindow();
    void showTimeTraceWindow();
    void showInputWindow();
    void showControllerTabWindow();

    bool isInputEditorVisible() const;
    void startHexViewIfOpen();
    void updateFrequentWindowViews();
    void updateFrameWindowViews();
    void refreshMovieLoadedWindows();
    void refreshMovieUnloadedWindows();
    void resetInputEditorWindow();
    void reloadConfigWindows();
    void getRamWatch(std::string &watch) const;
    void getMarkerText(std::string &text) const;
    void registerSavestate(int slot, unsigned long long frame);
    void addTimeTrace(int type, unsigned long long hash, std::string stacktrace);
    void updateSettingsWindow(int status);

private:
    static void showWindow(QWidget *window);

    SettingsWindow *settingsWindow();
    EncodeWindow *encodeWindow();
    InputWindow *inputWindow();
    ExecutableWindow *executableWindow();
    ControllerTabWindow *controllerTabWindow();
    GameInfoWindow *gameInfoWindow();
    HexViewWindow *hexViewWindow();
    RamWatchWindow *ramWatchWindow();
    RamSearchWindow *ramSearchWindow();
    InputEditorWindow *inputEditorWindow();
    AnnotationsWindow *annotationsWindow();
    TimeTraceWindow *timeTraceWindow();
    LuaConsoleWindow *luaConsoleWindow();
    MovieSettingsWindow *movieSettingsWindow();

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