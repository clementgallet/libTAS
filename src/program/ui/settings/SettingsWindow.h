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

#ifndef LIBTAS_SETTINGSWINDOW_H_INCLUDED
#define LIBTAS_SETTINGSWINDOW_H_INCLUDED

#include <QtWidgets/QDialog>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QMainWindow>

class Context;
class RuntimePane;
class MoviePane;
class InputPane;
class AudioPane;
class VideoPane;
class DebugPane;
class GameSpecificPane;

class SettingsWindow : public QMainWindow {
    Q_OBJECT
public:
    SettingsWindow(Context *c, QWidget *parent = Q_NULLPTR);

    /* Update UI elements when the config has changed */
    // void update_config();

    Context *context;
    
    enum TabIndex
    {
        RuntimeTab = 0,
        MovieTab= 1,
        InputTab = 2,
        AudioTab = 3,
        VideoTab = 4,
        DebugTab = 5,
        GameSpecificTab = 6,
    };

    void openRuntimeTab();
    void openMovieTab();
    void openInputTab();
    void openAudioTab();
    void openVideoTab();
    void openDebugTab();
    void openGameSpecificTab();

    void loadConfig();

private:
    QTabWidget* tabWidget;
    RuntimePane* rp;
    MoviePane* mp;
    InputPane* ip;
    AudioPane* ap;
    VideoPane* vp;
    DebugPane* gp;
    GameSpecificPane* gsp;

private slots:
    void save();
    
public slots:
    void update(int status);
};

#endif
