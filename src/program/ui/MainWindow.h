/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_MAINWINDOW_H_INCLUDED
#define LIBTAS_MAINWINDOW_H_INCLUDED

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QAction>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QToolButton>
#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <forward_list>
#include <string>

#ifdef __unix__
#include "config.h"
#endif

#include "../GameLoop.h"
#include "../Context.h"

#include <thread>

class SettingsWindow;
class EncodeWindow;
class InputWindow;
class ExecutableWindow;
class ControllerTabWindow;
class GameInfoWindow;
class GameSpecificWindow;
class RamSearchWindow;
class RamWatchWindow;
class InputEditorWindow;
class OsdWindow;
class AnnotationsWindow;
class AutoSaveWindow;
class TimeTraceWindow;
class CustomResolutionDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Context* c);
    ~MainWindow();

    /* Capture the user closing the game and show a "save your work?" dialog */
    void closeEvent(QCloseEvent *event);

    std::thread game_thread;
    GameLoop *gameLoop;
    Context *context;

    SettingsWindow* settingsWindow;
    EncodeWindow* encodeWindow;
    InputWindow* inputWindow;
    ExecutableWindow* executableWindow;
    ControllerTabWindow* controllerTabWindow;
    GameInfoWindow* gameInfoWindow;
    GameSpecificWindow* gameSpecificWindow;
    RamSearchWindow* ramSearchWindow;
    RamWatchWindow* ramWatchWindow;
    InputEditorWindow* inputEditorWindow;
    OsdWindow* osdWindow;
    AnnotationsWindow* annotationsWindow;
    AutoSaveWindow* autoSaveWindow;
    TimeTraceWindow* timeTraceWindow;

    QList<QWidget*> disabledWidgetsOnStart;
    QList<QAction*> disabledActionsOnStart;

    QAction *saveMovieAction;
    QAction *exportMovieAction;
    QAction *annotateMovieAction;

    QAction *autoRestartAction;
    QAction *variableFramerateAction;

    QAction *renderSoftAction;
    QAction *renderPerfAction;

    QAction *busyloopAction;

    QAction *configEncodeAction;
    QAction *toggleEncodeAction;

    QActionGroup *slowdownGroup;
    QActionGroup *fastforwardGroup;
    QActionGroup *fastforwardRenderGroup;

    QAction *mouseModeAction;

    QComboBox *gamePath;
    QPushButton *browseGamePath;
    QComboBox *cmdOptions;

    QLineEdit *moviePath;
    QPushButton *browseMoviePath;

    QLineEdit *authorField;

    QRadioButton *movieRecording;
    QRadioButton *moviePlayback;

    QSpinBox *fpsNumField;
    QSpinBox *fpsDenField;
    QLabel *fpsValues;

    QCheckBox *pauseCheck;
    QCheckBox *fastForwardCheck;

    QSpinBox *frameCount;
    QSpinBox *movieFrameCount;
    QSpinBox *rerecordCount;
    QLabel *currentLength;
    QLabel *movieLength;

    QSpinBox *elapsedTimeSec;
    QSpinBox *elapsedTimeNsec;
    QSpinBox *realTimeSec;
    QSpinBox *realTimeNsec;
    QLabel *realTimeFormat;

    QPushButton *launchButton;
    QToolButton *launchGdbButton;
    QAction *launchGdbAction;
    QAction *launchLldbAction;
    QPushButton *stopButton;

    QGroupBox *movieBox;

    QLabel *statusIcon;
    QLabel *statusSoft;
    QLabel *statusMute;


    /* Event filter function to prevent menu close when a checkable option is clicked */
    bool eventFilter(QObject *obj, QEvent *event);

private:
    /* Update the status bar */
    void updateStatusBar();

    /* Update movie parameters from movie file */
    void updateMovieParams();

    /* Update the list of recent gamepaths */
    void updateRecentGamepaths();

    /* Timer to limit the number of update calls */
    QElapsedTimer* updateTimer;

    /* Timer to trigger the update call */
    QTimer* callTimer;
    
    /* Helper function to create a checkable action inside an action group */
    QAction *addActionCheckable(QActionGroup*& group, const QString& text, const QVariant &data, const QString& toolTip);
    QAction *addActionCheckable(QActionGroup*& group, const QString& text, const QVariant &data);

    /* Create the main window actions that will go in the menus */
    void createActions();

    /* Create the main window menus */
    void createMenus();

private slots:

    /* Update UI elements (mainly enable/disable) depending on
     * the game status (running/stopped), to prevent modifying values that
     * are not supposed to be modified when the game is running.
     */
    void updateStatus(int status);

    /* Update UI elements that are often modified, triggered by a timer */
    void updateUIFrequent();

    /* Update UI elements when the shared config has changed (pause, fastforward,
     * encode, etc.
     */
    void updateSharedConfigChanged();

    /* Update UI elements when a config file is loaded */
    void updateUIFromConfig();

    /* Show an alert dialog containing alert_msg as text */
    void alertDialog(QString alert_msg);

    /* Show an alert dialog asking the user a question, and get the answer */
    void alertOffer(QString alert_msg, void* promise);

    /* Update framerate values */
    void updateFramerate();

    void slotLaunchGdb();
    void slotLaunchLldb();
    void slotLaunch(bool attach_gdb);
    void slotStop();
    void slotBrowseGamePath();
    void slotGamePathChanged();
    void slotBrowseMoviePath();
    void slotMoviePathChanged();
    void slotSaveMovie();
    void slotExportMovie();
    void slotPause(bool checked);
    void slotFastForward(bool checked);
    void slotMovieEnable(bool checked);
    void slotMovieRecording();
    void slotToggleEncode();
    void slotPauseMovie();
    void slotVariableFramerate(bool checked);
    void slotLuaExecute();
    void slotRealTimeFormat();
};

#endif
