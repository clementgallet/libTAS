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
#include "ErrorChecking.h"
#include "settings/SettingsWindow.h"

#include "movie/MovieFile.h"
#include "utils.h"
#include "GameEvents.h"
#include "GameThread.h"
#include "../shared/version.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QApplication>
#include <QtCore/QTimer>

#include <iostream>
#include <future>
#include <sys/stat.h>
#include <csignal> // kill
#include <unistd.h> // access, isatty
#include <limits>
#include <features.h> // __GLIBC_PREREQ
#include <sys/wait.h> // waitpid

/* Check all checkboxes from a list of actions whose associated flag data
 * is present in the value
 */
#define setCheckboxesFromMask(actionGroup, value)\
do {\
    for (auto& action : actionGroup->actions()) {\
        action->setChecked(value & action->data().toInt());\
    }\
} while(false)

/* For each checkbox of the action group that is checked, set the
 * corresponding flag in the value.
 */
#define setMaskFromCheckboxes(actionGroup, value)\
do {\
    value = 0;\
    for (const auto& action : actionGroup->actions()) {\
        if (action->isChecked()) {\
            value |= action->data().toInt();\
        }\
    }\
} while(false)

/* Check the radio from a list of actions whose associated data is equal
 * to the value.
 */
#define setRadioFromList(actionGroup, value)\
do {\
    /* Check the last item by default */ \
    actionGroup->actions().last()->setChecked(true); \
    for (auto& action : actionGroup->actions()) {\
        if (value == action->data().toInt()) {\
            action->setChecked(true);\
            break;\
        }\
    }\
} while(false)

/* Set the value to the data of the checked radio from the action group. */
#define setListFromRadio(actionGroup, value)\
do {\
    for (const auto& action : actionGroup->actions()) {\
        if (action->isChecked()) {\
            value = action->data().toInt();\
            break;\
        }\
    }\
} while(false)

#define LAMBDABOOLSLOT(parameter) [=, this](bool checked) {\
    parameter = checked;\
    context->config.sc_modified = true;\
}\

#define LAMBDACHECKBOXSLOT(group, parameter) [=, this](bool) {\
    setMaskFromCheckboxes(group, parameter);\
    context->config.sc_modified = true;\
}\

#define LAMBDARADIOSLOT(group, parameter) [=, this]() {\
    setListFromRadio(group, parameter);\
    context->config.sc_modified = true;\
}\

MainWindow::MainWindow(Context* c) : QMainWindow(), context(c)
{
    qRegisterMetaType<std::string>("std::string");

#ifdef LIBTAS_INTERIM_COMMIT
#ifdef LIBTAS_INTERIM_DATE
    QString title = QString("libTAS v%1.%2.%3 - interim %4 (%5)").arg(MAJORVERSION).arg(MINORVERSION).arg(PATCHVERSION).arg(LIBTAS_INTERIM_COMMIT).arg(LIBTAS_INTERIM_DATE);
#else
    QString title = QString("libTAS v%1.%2.%3 - interim %4").arg(MAJORVERSION).arg(MINORVERSION).arg(PATCHVERSION).arg(LIBTAS_INTERIM_COMMIT);
#endif
#else
    QString title = QString("libTAS v%1.%2.%3").arg(MAJORVERSION).arg(MINORVERSION).arg(PATCHVERSION);
#endif

    setWindowTitle(title);

    /* Create the object that will launch and communicate with the game,
     * and connect all the signals.
     */
    gameLoop = new GameLoop(context);
    connect(gameLoop, &GameLoop::uiChanged, this, &MainWindow::updateUIFrequent);
    connect(gameLoop, &GameLoop::newFrame, this, &MainWindow::updateUIFrame, Qt::DirectConnection);
    connect(gameLoop, &GameLoop::statusChanged, this, &MainWindow::updateStatus);
    connect(gameLoop, &GameLoop::configChanged, this, &MainWindow::updateUIFromConfig);
    connect(gameLoop, &GameLoop::alertToShow, this, &MainWindow::alertDialog);
    connect(gameLoop->gameEvents, &GameEvents::alertToShow, this, &MainWindow::alertDialog);
    connect(gameLoop, &GameLoop::sharedConfigChanged, this, &MainWindow::updateSharedConfigChanged);
    connect(gameLoop->gameEvents, &GameEvents::sharedConfigChanged, this, &MainWindow::updateSharedConfigChanged);
    connect(gameLoop, &GameLoop::askToShow, this, &MainWindow::alertOffer);
    connect(gameLoop->gameEvents, &GameEvents::askToShow, this, &MainWindow::alertOffer);
    connect(gameLoop, &GameLoop::updateFramerate, this, &MainWindow::updateFramerate);

    /* Create other windows */
    settingsWindow = new SettingsWindow(c, this);
    encodeWindow = new EncodeWindow(c, this);
    inputWindow = new InputWindow(c, this);
    executableWindow = new ExecutableWindow(c, this);
    controllerTabWindow = new ControllerTabWindow(c, this);
    gameInfoWindow = new GameInfoWindow(this);
    hexViewWindow = new HexViewWindow(this);
    ramWatchWindow = new RamWatchWindow(c, hexViewWindow, this);
    ramSearchWindow = new RamSearchWindow(c, hexViewWindow, ramWatchWindow, this);
    inputEditorWindow = new InputEditorWindow(c, &gameLoop->movie, this);
    annotationsWindow = new AnnotationsWindow(c, this);
    timeTraceWindow = new TimeTraceWindow(c, this);
    luaConsoleWindow = new LuaConsoleWindow(c, this);
    movieSettingsWindow = new MovieSettingsWindow(c, &gameLoop->movie, this);

    connect(gameLoop, &GameLoop::isInputEditorVisible, inputEditorWindow, &InputEditorWindow::isWindowVisible, Qt::DirectConnection);
    connect(gameLoop->gameEvents, &GameEvents::isInputEditorVisible, inputEditorWindow, &InputEditorWindow::isWindowVisible, Qt::DirectConnection);
    connect(gameLoop, &GameLoop::getRamWatch, ramWatchWindow->ramWatchView, &RamWatchView::slotGet, Qt::DirectConnection);
    connect(gameLoop, &GameLoop::getMarkerText, inputEditorWindow->inputEditorView, &InputEditorView::getCurrentMarkerText, Qt::DirectConnection);
    connect(gameLoop->gameEvents, &GameEvents::savestatePerformed, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::registerSavestate);
    connect(gameLoop, &GameLoop::getTimeTrace, timeTraceWindow->timeTraceModel, &TimeTraceModel::addCall);
    connect(gameLoop, &GameLoop::statusChanged, settingsWindow, &SettingsWindow::update);

    /* Menu */
    createActions();
    createMenus();
    menuBar()->setNativeMenuBar(false);

    /* Game Executable */
    gamePath = new QComboBox();
    gamePath->setMinimumWidth(400);
    gamePath->setEditable(true);
    connect(gamePath, &QComboBox::editTextChanged, this, &MainWindow::slotGamePathChanged);
    gamePath->setInsertPolicy(QComboBox::NoInsert);
    disabledWidgetsOnStart.append(gamePath);
    updateRecentGamepaths();

    browseGamePath = new QPushButton("Browse...");
    connect(browseGamePath, &QAbstractButton::clicked, this, &MainWindow::slotBrowseGamePath);
    disabledWidgetsOnStart.append(browseGamePath);

    /* Command-line options */
    cmdOptions = new QComboBox();
    cmdOptions->setMinimumWidth(400);
    cmdOptions->setEditable(true);
    cmdOptions->setInsertPolicy(QComboBox::NoInsert);
    disabledWidgetsOnStart.append(cmdOptions);

    /* Movie File */
    moviePath = new QLineEdit();
    connect(moviePath, &QLineEdit::textEdited, this, &MainWindow::slotMoviePathChanged);
    disabledWidgetsOnStart.append(moviePath);

    browseMoviePath = new QPushButton("Browse...");
    connect(browseMoviePath, &QAbstractButton::clicked, this, &MainWindow::slotBrowseMoviePath);
    disabledWidgetsOnStart.append(browseMoviePath);

    authorField = new QLineEdit();
    disabledWidgetsOnStart.append(authorField);

    movieRecording = new QRadioButton("Recording");
    connect(movieRecording, &QAbstractButton::clicked, this, &MainWindow::slotMovieRecording);
    moviePlayback = new QRadioButton("Playback");
    connect(moviePlayback, &QAbstractButton::clicked, this, &MainWindow::slotMovieRecording);

    /* Frame count */
    frameCount = new QSpinBox();
    frameCount->setReadOnly(true);
    frameCount->setMaximum(std::numeric_limits<int>::max());

    movieFrameCount = new QSpinBox();
    movieFrameCount->setReadOnly(true);
    movieFrameCount->setMaximum(std::numeric_limits<int>::max());

    /* Current/movie length */
    currentLength = new QLabel("Current time: -");
    movieLength = new QLabel("Movie length: -");

    /* Frames per second */
    fpsNumField = new QSpinBox();
    fpsNumField->setMaximum(std::numeric_limits<int>::max());
    fpsNumField->setMinimum(1);
    connect(fpsNumField, QOverload<int>::of(&QSpinBox::valueChanged),[=, this](int i){
        context->current_framerate_num = i;
        gameLoop->movie.inputs->variable_framerate = true;
    });

    fpsDenField = new QSpinBox();
    fpsDenField->setMaximum(std::numeric_limits<int>::max());
    fpsDenField->setMinimum(1);
    connect(fpsDenField, QOverload<int>::of(&QSpinBox::valueChanged),[=, this](int i){
        context->current_framerate_den = i;
        gameLoop->movie.inputs->variable_framerate = true;
    });

    fpsValues = new QLabel("Current FPS: - / -");

    /* Re-record count */
    rerecordCount = new QSpinBox();
    rerecordCount->setReadOnly(true);
    rerecordCount->setMaximum(std::numeric_limits<int>::max());

    /* Elapsed and Real time */
    elapsedTimeSec = new QSpinBox();
    elapsedTimeSec->setMaximum(std::numeric_limits<int>::max());
    elapsedTimeSec->setMinimumWidth(50);
    elapsedTimeNsec = new QSpinBox();
    elapsedTimeNsec->setMaximum(std::numeric_limits<int>::max());
    elapsedTimeNsec->setMinimumWidth(50);

    realTimeSec = new QSpinBox();
    realTimeSec->setMinimum(1);
    realTimeSec->setMaximum(std::numeric_limits<int>::max());
    realTimeSec->setMinimumWidth(50);
    realTimeNsec = new QSpinBox();
    realTimeNsec->setMaximum(std::numeric_limits<int>::max());
    realTimeNsec->setMinimumWidth(50);
    realTimeFormat = new QLabel();
    connect(realTimeSec, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::slotRealTimeFormat);
    connect(realTimeNsec, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::slotRealTimeFormat);

    QPushButton* realTimeChange = new QPushButton("Set");
    connect(realTimeChange, &QAbstractButton::clicked, [=, this](){
        context->new_realtime_sec = realTimeSec->value();
        context->new_realtime_nsec = realTimeNsec->value();
    });

    /* Pause/FF */
    pauseCheck = new QCheckBox("Pause");
    connect(pauseCheck, &QAbstractButton::clicked, this, &MainWindow::slotPause);
    fastForwardCheck = new QCheckBox("Fast-forward");
    connect(fastForwardCheck, &QAbstractButton::clicked, this, &MainWindow::slotFastForward);

    /* Buttons */
    launchButton = new QPushButton(tr("Start"));
    connect(launchButton, &QAbstractButton::clicked, this, [this] { MainWindow::slotLaunch(false); });
    disabledWidgetsOnStart.append(launchButton);

    launchGdbButton = new QToolButton();
    launchGdbButton->setToolButtonStyle(Qt::ToolButtonTextOnly);

    launchGdbAction = new QAction(tr("Launch with GDB"), this);
    launchLldbAction = new QAction(tr("Launch with LLDB"), this);
    launchStraceAction = new QAction(tr("Launch with strace"), this);
    if (!isatty(STDIN_FILENO)) {
        launchGdbAction->setEnabled(false);
        launchLldbAction->setEnabled(false);
        launchStraceAction->setEnabled(false);
    }

    connect(launchGdbAction, &QAction::triggered, this, &MainWindow::slotLaunchGdb);
    connect(launchLldbAction, &QAction::triggered, this, &MainWindow::slotLaunchLldb);
    connect(launchStraceAction, &QAction::triggered, this, &MainWindow::slotLaunchStrace);

#ifdef __unix__
    launchGdbButton->setPopupMode(QToolButton::MenuButtonPopup);

    QMenu *launchGdbButtonMenu = new QMenu();
    launchGdbButton->setMenu(launchGdbButtonMenu);

    launchGdbButtonMenu->addAction(launchGdbAction);
    launchGdbButtonMenu->addAction(launchLldbAction);
    launchGdbButtonMenu->addAction(launchStraceAction);
#endif

    stopButton = new QPushButton(tr("Stop"));
    connect(stopButton, &QAbstractButton::clicked, this, &MainWindow::slotStop);
    stopButton->setEnabled(false);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(launchButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(launchGdbButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(stopButton, QDialogButtonBox::ActionRole);

    /* Status bar */
    QStyle *currentStyle = QApplication::style();
    QIcon icon = currentStyle->standardIcon(QStyle::SP_MessageBoxWarning);
    QPixmap pixmap = icon.pixmap(statusBar()->height()*0.6,statusBar()->height()*0.6);

    statusIcon = new QLabel();
    statusIcon->setPixmap(pixmap);
    statusSoft = new QLabel(tr("Savestates will likely not work unless you check [Video > Force software rendering]"));
    statusMute = new QLabel(tr("Savestates will likely not work unless you check [Sound > Mute]"));

    /* Layouts */


    /* Game parameters layout */
    QGroupBox *gameBox = new QGroupBox(tr("Game execution"));
    QGridLayout *gameLayout = new QGridLayout;
    gameLayout->addWidget(new QLabel(tr("Game executable")), 0, 0);
    gameLayout->addWidget(gamePath, 0, 1);
    gameLayout->addWidget(browseGamePath, 0, 2);
    gameLayout->addWidget(new QLabel(tr("Command-line options")), 1, 0);
    gameLayout->addWidget(cmdOptions, 1, 1);
    gameLayout->setColumnStretch(1, 1);
    gameBox->setLayout(gameLayout);

    /* Movie layout */
    movieBox = new QGroupBox(tr("Movie recording"));
    movieBox->setCheckable(true);
    connect(movieBox, &QGroupBox::clicked, this, &MainWindow::slotMovieEnable);

    QVBoxLayout *movieLayout = new QVBoxLayout;

    QGridLayout *movieFileLayout = new QGridLayout;
    movieFileLayout->addWidget(new QLabel(tr("Movie file:")), 0, 0);
    movieFileLayout->addWidget(moviePath, 0, 1);
    movieFileLayout->addWidget(browseMoviePath, 0, 2);
    movieFileLayout->addWidget(new QLabel(tr("Authors:")), 1, 0);
    movieFileLayout->addWidget(authorField, 1, 1);
    movieFileLayout->setColumnStretch(1, 1);

    QGridLayout *movieCountLayout = new QGridLayout;
    movieCountLayout->addWidget(new QLabel(tr("Movie frame count:")), 0, 0);
    movieCountLayout->addWidget(movieFrameCount, 0, 1);
    movieCountLayout->addWidget(movieLength, 0, 3);
    movieCountLayout->addWidget(new QLabel(tr("Rerecord count:")), 1, 0);
    movieCountLayout->addWidget(rerecordCount, 1, 1);
    movieCountLayout->setColumnStretch(1, 1);
    movieCountLayout->setColumnMinimumWidth(2, 50);

    QGroupBox *movieStatusBox = new QGroupBox(tr("Movie status"));
    QHBoxLayout *movieStatusLayout = new QHBoxLayout;
    movieStatusLayout->addWidget(movieRecording);
    movieStatusLayout->addWidget(moviePlayback);
    movieStatusLayout->addStretch(1);
    movieStatusBox->setLayout(movieStatusLayout);

    movieLayout->addLayout(movieFileLayout);
    movieLayout->addLayout(movieCountLayout);
    movieLayout->addWidget(movieStatusBox);
    movieBox->setLayout(movieLayout);

    /* General layout */
    QGroupBox *generalBox = new QGroupBox(tr("General options"));
    QVBoxLayout *generalLayout = new QVBoxLayout;

    QGridLayout *generalFrameLayout = new QGridLayout;
    generalFrameLayout->addWidget(new QLabel(tr("Frame:")), 0, 0);
//    generalFrameLayout->addStretch(1);
    generalFrameLayout->addWidget(frameCount, 0, 1, 1, 3);
//    generalFrameLayout->addStretch(1);
    generalFrameLayout->addWidget(currentLength, 0, 5);
//    generalFrameLayout->addStretch(1);

    generalFrameLayout->addWidget(new QLabel(tr("Frames per second:")), 1, 0);
//    generalFpsLayout->addStretch(1);
    generalFrameLayout->addWidget(fpsNumField, 1, 1);
    generalFrameLayout->addWidget(new QLabel(tr("/")), 1, 2);
    generalFrameLayout->addWidget(fpsDenField, 1, 3);
//    generalFpsLayout->addStretch(1);
    generalFrameLayout->addWidget(fpsValues, 1, 5);
//    generalFpsLayout->addStretch(1);
    generalFrameLayout->setColumnStretch(1, 1);
    generalFrameLayout->setColumnStretch(3, 1);
    generalFrameLayout->setColumnStretch(4, 1);
    generalFrameLayout->setColumnStretch(6, 1);


    QGridLayout *generalTimeLayout = new QGridLayout;
    generalTimeLayout->addWidget(new QLabel(tr("Elapsed time:")), 0, 0);
    generalTimeLayout->addWidget(elapsedTimeSec, 0, 2);
    generalTimeLayout->addWidget(new QLabel(tr("sec")), 0, 3);
    generalTimeLayout->addWidget(elapsedTimeNsec, 0, 5);
    generalTimeLayout->addWidget(new QLabel(tr("nsec")), 0, 6);
    generalTimeLayout->addWidget(new QLabel(tr("System time:")), 1, 0);
    generalTimeLayout->addWidget(realTimeSec, 1, 2);
    generalTimeLayout->addWidget(new QLabel(tr("sec")), 1, 3);
    generalTimeLayout->addWidget(realTimeNsec, 1, 5);
    generalTimeLayout->addWidget(new QLabel(tr("nsec")), 1, 6);
    generalTimeLayout->addWidget(realTimeFormat, 1, 7);
    generalTimeLayout->addWidget(realTimeChange, 1, 8);

    QHBoxLayout *generalControlLayout = new QHBoxLayout;
    generalControlLayout->addWidget(pauseCheck);
    generalControlLayout->addWidget(fastForwardCheck);
    generalControlLayout->addStretch(1);

    generalLayout->addLayout(generalFrameLayout);
//    generalLayout->addLayout(generalFpsLayout);
    generalLayout->addLayout(generalTimeLayout);
    generalLayout->addLayout(generalControlLayout);
    generalBox->setLayout(generalLayout);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(gameBox);
    mainLayout->addStretch(1);
    mainLayout->addWidget(movieBox);
    mainLayout->addStretch(1);
    mainLayout->addWidget(generalBox);
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonBox);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    updateUIFromConfig();

    /* Start the update timer */
    updateTimer = new QElapsedTimer();
    updateTimer->start();

    /* Configure the call timer */
    callTimer = new QTimer(this);
    callTimer->setSingleShot(true);
    connect(callTimer, &QTimer::timeout, this, &MainWindow::updateUIFrequent);

    /* We may have already started dumping from command-line */
    if (context->config.dumping) {
        slotToggleEncode();
    }

    /* We must start the game in non-interactive mode */
    if (!context->interactive) {
        slotPause(false);
        slotFastForward(true);
        slotLaunch(false);
    }
    
    debugger_pid = 0;
}

MainWindow::~MainWindow()
{
    delete gameLoop;

    if (game_thread.joinable())
        game_thread.detach();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!context->interactive) {
        event->accept();
        return;
    }

    if (context->status == Context::INACTIVE) {
        event->accept();
        return;
    }

    if (gameLoop->movie.inputs->modifiedSinceLastSave) {
        QMessageBox::StandardButton result = QMessageBox::question(
            this, tr("Unsaved Work"), 
            tr("You have unsaved work. Would you like to save it?"),
            QMessageBox::Cancel|QMessageBox::Discard|QMessageBox::Save,
            QMessageBox::Save
        );

        switch (result) {
            case QMessageBox::Save:
                this->gameLoop->movie.saveMovie();
                event->accept();
                break;
            case QMessageBox::Discard:
                event->accept();
                break;
            case QMessageBox::Cancel:
            default:
                event->ignore();
                break;
        }
    } else {
        event->accept();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMenu *menu = qobject_cast<QMenu*>(obj);
        if (menu) {
            if (menu->activeAction() && menu->activeAction()->isCheckable()) {
                /* If we click on a checkable action, trigger the action but
                 * do not close the menu
                 */
                menu->activeAction()->trigger();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

/* We are going to do this a lot, so this is a helper function to insert
 * checkable actions into an action group with data.
 */
QAction *MainWindow::addActionCheckable(QActionGroup*& group, const QString& text, const QVariant &qdata, const QString& toolTip)
{
    QAction *action = group->addAction(text);
    action->setCheckable(true);
    action->setData(qdata);
    if (!toolTip.isEmpty())
        action->setToolTip(toolTip);
    return action;
}

QAction *MainWindow::addActionCheckable(QActionGroup*& group, const QString& text, const QVariant &qdata) {
    return addActionCheckable(group, text, qdata, "");
}

void MainWindow::createActions()
{
    slowdownGroup = new QActionGroup(this);
    connect(slowdownGroup, &QActionGroup::triggered, this, LAMBDARADIOSLOT(slowdownGroup, context->config.sc.speed_divisor));

    addActionCheckable(slowdownGroup, tr("100% (normal speed)"), 1);
    addActionCheckable(slowdownGroup, tr("50%"), 2);
    addActionCheckable(slowdownGroup, tr("25%"), 4);
    addActionCheckable(slowdownGroup, tr("12%"), 8);

    fastforwardGroup = new QActionGroup(this);
    fastforwardGroup->setExclusive(false);
    connect(fastforwardGroup, &QActionGroup::triggered, this, LAMBDACHECKBOXSLOT(fastforwardGroup, context->config.sc.fastforward_mode));

    addActionCheckable(fastforwardGroup, tr("Skipping sleep"), SharedConfig::FF_SLEEP);
    addActionCheckable(fastforwardGroup, tr("Skipping audio mixing"), SharedConfig::FF_MIXING);

    fastforwardRenderGroup = new QActionGroup(this);
    connect(fastforwardRenderGroup, &QActionGroup::triggered, this, LAMBDARADIOSLOT(fastforwardRenderGroup, context->config.sc.fastforward_render));

    addActionCheckable(fastforwardRenderGroup, tr("Skipping no rendering"), SharedConfig::FF_RENDER_ALL);
    addActionCheckable(fastforwardRenderGroup, tr("Skipping most rendering"), SharedConfig::FF_RENDER_SOME);
    addActionCheckable(fastforwardRenderGroup, tr("Skipping all rendering"), SharedConfig::FF_RENDER_NO);
    
    saveStateGroup = new QActionGroup(this);
    loadStateGroup = new QActionGroup(this);
    loadBranchGroup = new QActionGroup(this);
    connect(saveStateGroup, &QActionGroup::triggered, this, &MainWindow::slotSaveState);
    connect(loadStateGroup, &QActionGroup::triggered, this, &MainWindow::slotSaveState);
    connect(loadBranchGroup, &QActionGroup::triggered, this, &MainWindow::slotSaveState);

    QAction *action;
    for (int s = 0; s < 10; s++) {
        action = saveStateGroup->addAction(QString("Slot %1").arg(s+1));
        action->setData(HOTKEY_SAVESTATE1 + s);
        action = loadStateGroup->addAction(QString("Slot %1").arg(s+1));
        action->setData(HOTKEY_LOADSTATE1 + s);
        action = loadBranchGroup->addAction(QString("Slot %1").arg(s+1));
        action->setData(HOTKEY_LOADBRANCH1 + s);
    }
}

void MainWindow::createMenus()
{
    QAction *action;

    /* File Menu */
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));

    action = fileMenu->addAction(tr("Open Executable..."), this, &MainWindow::slotBrowseGamePath);
    disabledActionsOnStart.append(action);
    action = fileMenu->addAction(tr("Executable Options..."), executableWindow, &ExecutableWindow::exec);
    disabledActionsOnStart.append(action);

    /* Settings Menu */
    QMenu *settingsMenu = menuBar()->addMenu(tr("Settings"));
    settingsMenu->addAction(tr("Runtime..."), settingsWindow, &SettingsWindow::openRuntimeTab);
    settingsMenu->addAction(tr("Movie..."), settingsWindow, &SettingsWindow::openMovieTab);
    settingsMenu->addAction(tr("Inputs..."), settingsWindow, &SettingsWindow::openInputTab);
    settingsMenu->addAction(tr("Audio..."), settingsWindow, &SettingsWindow::openAudioTab);
    settingsMenu->addAction(tr("Video..."), settingsWindow, &SettingsWindow::openVideoTab);
    settingsMenu->addAction(tr("Debug..."), settingsWindow, &SettingsWindow::openDebugTab);
    settingsMenu->addAction(tr("Game Specific..."), settingsWindow, &SettingsWindow::openGameSpecificTab);
    settingsMenu->addAction(tr("Paths..."), settingsWindow, &SettingsWindow::openPathTab);

    /* Movie Menu */
    QMenu *movieMenu = menuBar()->addMenu(tr("Movie"));
    movieMenu->setToolTipsVisible(true);

    action = movieMenu->addAction(tr("Open Movie..."), this, &MainWindow::slotBrowseMoviePath);
    disabledActionsOnStart.append(action);
    saveMovieAction = movieMenu->addAction(tr("Save Movie"), this, &MainWindow::slotSaveMovie);
    saveMovieAction->setEnabled(false);
    exportMovieAction = movieMenu->addAction(tr("Export Movie..."), this, &MainWindow::slotExportMovie);
    exportMovieAction->setEnabled(false);
    settingsMovieAction = movieMenu->addAction(tr("Movie Settings..."), movieSettingsWindow, &MovieSettingsWindow::exec);
    settingsMovieAction->setEnabled(false);
    action = movieMenu->addAction(tr("Don't enforce movie settings"), this, [=, this](bool checked){gameLoop->movie.header->skipLoadSettings = checked;});
    action->setCheckable(true);
    action->setToolTip("When checked, settings stored inside the movie metadata won't be enforced (e.g. initial time, mouse/controller support, framerate...). You can then save your movie with the new settings.");

    movieMenu->addSeparator();

    annotateMovieAction = movieMenu->addAction(tr("Annotations..."), annotationsWindow, &AnnotationsWindow::show);
    bool recording_inactive = ((context->config.sc.recording != SharedConfig::NO_RECORDING) && (gameLoop->movie.loadMovie() == 0));
    annotateMovieAction->setEnabled(recording_inactive);

    movieMenu->addSeparator();

    movieMenu->addAction(tr("Pause Movie at frame..."), this, &MainWindow::slotPauseMovie);
    autoRestartAction = movieMenu->addAction(tr("Auto-restart game"), this, LAMBDABOOLSLOT(context->config.auto_restart));
    autoRestartAction->setCheckable(true);
    autoRestartAction->setToolTip("When checked, the game will automatically restart if closed, except when using the Stop button");
    disabledActionsOnStart.append(autoRestartAction);

    movieMenu->addAction(tr("Input Editor..."), inputEditorWindow, &InputEditorWindow::show);

    /* Tools Menu */
    QMenu *toolsMenu = menuBar()->addMenu(tr("Tools"));
    configEncodeAction = toolsMenu->addAction(tr("Configure encode..."), encodeWindow, &EncodeWindow::exec);
    toggleEncodeAction = toolsMenu->addAction(tr("Start encode"), this, &MainWindow::slotToggleEncode);
    screenshotAction = toolsMenu->addAction(tr("Screenshot..."), this, &MainWindow::slotScreenshot);

    toolsMenu->addSeparator();

    QMenu *slowdownMenu = toolsMenu->addMenu(tr("Slow Motion"));
    slowdownMenu->addActions(slowdownGroup->actions());

    toolsMenu->addSeparator();

    QMenu *fastforwardMenu = toolsMenu->addMenu(tr("Fast-forward mode"));
    fastforwardMenu->addActions(fastforwardGroup->actions());
    fastforwardMenu->addSeparator();
    fastforwardMenu->addActions(fastforwardRenderGroup->actions());

    toolsMenu->addSeparator();

    QMenu *saveStateMenu = toolsMenu->addMenu(tr("Save State"));
    saveStateMenu->addActions(saveStateGroup->actions());
    QMenu *loadStateMenu = toolsMenu->addMenu(tr("Load State"));
    loadStateMenu->addActions(loadStateGroup->actions());
    QMenu *loadBranchMenu = toolsMenu->addMenu(tr("Load Branch"));
    loadBranchMenu->addActions(loadBranchGroup->actions());

    toolsMenu->addSeparator();

    toolsMenu->addAction(tr("Game information..."), gameInfoWindow, &GameInfoWindow::exec);

    toolsMenu->addSeparator();

    toolsMenu->addAction(tr("Ram Search..."), ramSearchWindow, &RamSearchWindow::show);
    toolsMenu->addAction(tr("Ram Watch..."), ramWatchWindow, &RamWatchWindow::show);
    toolsMenu->addAction(tr("Hex Edit..."), hexViewWindow, &HexViewWindow::show);

    toolsMenu->addSeparator();

    toolsMenu->addAction(tr("Lua Console..."), luaConsoleWindow, &LuaConsoleWindow::show);

    toolsMenu->addSeparator();

    busyloopAction = toolsMenu->addAction(tr("Busy loop detection"), this, LAMBDABOOLSLOT(context->config.sc.busyloop_detection));
    busyloopAction->setCheckable(true);
    disabledActionsOnStart.append(busyloopAction);

    toolsMenu->addAction(tr("Time Trace..."), timeTraceWindow, &TimeTraceWindow::show);


    /* Input Menu */
    QMenu *inputMenu = menuBar()->addMenu(tr("Input"));
    inputMenu->setToolTipsVisible(true);

    inputMenu->addAction(tr("Configure mapping..."), inputWindow, &InputWindow::exec);

    mouseModeAction = inputMenu->addAction(tr("Mouse relative mode"), this, LAMBDABOOLSLOT(context->config.sc.mouse_mode_relative));
    mouseModeAction->setCheckable(true);

    inputMenu->addAction(tr("Joystick inputs..."), controllerTabWindow, &ControllerTabWindow::show);
}

void MainWindow::updateStatus(int status)
{
    /* Update game status (active/inactive) */

    switch (status) {

        case Context::INACTIVE:
            for (QWidget* w : disabledWidgetsOnStart) {
                if (!w->actions().empty())
                    for (QAction* a : w->actions())
                        a->setEnabled(true);
                else
                    w->setEnabled(true);
            }
            for (QAction* a : disabledActionsOnStart)
                a->setEnabled(true);

            if (context->config.sc.recording == SharedConfig::NO_RECORDING) {
                movieBox->setEnabled(true);
            }
            movieBox->setCheckable(true);
            movieBox->setChecked(context->config.sc.recording != SharedConfig::NO_RECORDING);

            fpsNumField->setReadOnly(false);
            fpsDenField->setReadOnly(false);
            elapsedTimeSec->setReadOnly(false);
            elapsedTimeNsec->setReadOnly(false);
            elapsedTimeSec->setValue(context->config.sc.initial_monotonic_time_sec);
            elapsedTimeNsec->setValue(context->config.sc.initial_monotonic_time_nsec);
            realTimeSec->setValue(context->config.sc.initial_time_sec);
            realTimeNsec->setValue(context->config.sc.initial_time_nsec);

            launchGdbButton->setEnabled(true);
            launchGdbAction->setText(tr("Launch with GDB"));
            launchLldbAction->setText(tr("Launch with LLDB"));
            launchStraceAction->setText(tr("Launch with strace"));

            if (context->config.sc.av_dumping) {
                context->config.sc.av_dumping = false;
                configEncodeAction->setEnabled(true);
                toggleEncodeAction->setText("Start encode");
            }

            frameCount->setValue(0);
            currentLength->setText("Current Time: -");
            fpsValues->setText("Current FPS: - / -");

            stopButton->setText("Stop");
            stopButton->setEnabled(false);

            updateMovieParams();
            break;

        case Context::STARTING:
            for (QWidget* w : disabledWidgetsOnStart) {
                if (!w->actions().empty())
                    for (QAction* a : w->actions())
                        a->setEnabled(false);
                else
                    w->setEnabled(false);
            }
            for (QAction* a : disabledActionsOnStart)
                a->setEnabled(false);

            elapsedTimeSec->setReadOnly(true);
            elapsedTimeNsec->setReadOnly(true);

            launchGdbButton->setEnabled(false);

            movieBox->setCheckable(false);
            if (context->config.sc.recording == SharedConfig::NO_RECORDING) {
                movieBox->setEnabled(false);
            }

            break;

        case Context::ACTIVE:
            if (!context->attach_gdb) {
                launchGdbButton->setEnabled(true);
                launchGdbAction->setText(tr("Attach with GDB"));
                launchLldbAction->setText(tr("Attach with LLDB"));
                launchStraceAction->setText(tr("Attach with strace"));
            }
            stopButton->setEnabled(true);

            if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
                saveMovieAction->setEnabled(true);
                exportMovieAction->setEnabled(true);
            }
            hexViewWindow->start();
            break;
        case Context::QUITTING:
            launchGdbButton->setEnabled(false);
            stopButton->setText("Kill");
            break;

        case Context::RESTARTING:

            /* Check that there might be a thread from a previous game execution */
            if (game_thread.joinable())
                game_thread.join();

            /* Restart game */
            game_thread = std::thread{&GameLoop::start, gameLoop};
            break;

        default:
            break;
    }
}

void MainWindow::updateSharedConfigChanged()
{
    /* Update pause status */
    pauseCheck->setChecked(!context->config.sc.running);

    /* Update fastforward status */
    fastForwardCheck->setChecked(context->config.sc.fastforward);

    /* Update recording state */
    std::string movieframestr;
    switch (context->config.sc.recording) {
        case SharedConfig::RECORDING_WRITE:
            movieRecording->setChecked(true);
            movieFrameCount->setValue(context->config.sc.movie_framecount);
            break;
        case SharedConfig::RECORDING_READ:
            moviePlayback->setChecked(true);
            movieFrameCount->setValue(context->config.sc.movie_framecount);
            break;
        default:
            break;
    }

    /* Update encode menus */
    if (context->config.sc.av_dumping) {
        configEncodeAction->setEnabled(false);
        toggleEncodeAction->setText("Stop encode");
    }
    else {
        configEncodeAction->setEnabled(true);
        toggleEncodeAction->setText("Start encode");
    }
}

void MainWindow::updateRecentGamepaths()
{
    /* We don't want to fire a signal by changing the combobox content */
    disconnect(gamePath, &QComboBox::editTextChanged, this, &MainWindow::slotGamePathChanged);

    gamePath->clear();
    for (const auto& path : context->config.recent_gamepaths) {
        gamePath->addItem(QString(path.c_str()));
    }

    connect(gamePath, &QComboBox::editTextChanged, this, &MainWindow::slotGamePathChanged);
}

void MainWindow::updateUIFrequent()
{
    /* Only update every 50 ms */
    int64_t elapsed = updateTimer->elapsed();
    if (elapsed < 50) {
        /* Call this function on timeout, if not already done */
        if (!callTimer->isActive()) {
            callTimer->start(50 - elapsed);
        }
        return;
    }

    updateTimer->start();

    /* Update frame count */
    frameCount->setValue(context->framecount);
    movieFrameCount->setValue(context->config.sc.movie_framecount);

    /* Update time */
    elapsedTimeSec->setValue(context->current_time_sec);
    elapsedTimeNsec->setValue(context->current_time_nsec);
    realTimeSec->setValue(context->new_realtime_sec);
    realTimeNsec->setValue(context->new_realtime_nsec);

    /* Update movie time */
    double sec = (context->current_time_sec - context->config.sc.initial_monotonic_time_sec) +
                ((double) (context->current_time_nsec - context->config.sc.initial_monotonic_time_nsec))/1000000000;
    int imin = (int)(sec/60);
    double dsec = sec - 60*imin;
    currentLength->setText(QString("Current Time: %1m %2s").arg(imin).arg(dsec, 0, 'f', 2));

    /* Format movie length */
    if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
        double msec = gameLoop->movie.inputs->length_sec + ((double)gameLoop->movie.inputs->length_nsec)/1000000000.0;
        int immin = (int)(msec/60);
        double dmsec = msec - 60*immin;
        movieLength->setText(QString("Movie length: %1m %2s").arg(immin).arg(dmsec, 0, 'f', 2));
    }

    /* Update rerecord count */
    rerecordCount->setValue(context->rerecord_count);

    /* Update fps values */
    if ((context->fps > 0) || (context->lfps > 0)) {
        fpsValues->setText(QString("Current FPS: %1 / %2").arg(context->fps, 0, 'f', 1).arg(context->lfps, 0, 'f', 1));
    }
    else {
        fpsValues->setText("Current FPS: - / -");
    }

    /* Update RAM watch/search */
    if (ramSearchWindow->isVisible()) {
        ramSearchWindow->update();
    }
    ramWatchWindow->update();

    /* Update input editor */
    inputEditorWindow->update();

    /* Update hex viewer */
    if (hexViewWindow->isVisible()) {
        hexViewWindow->update();
    }
    
    /* Detect if the debugger was detached from the game and allow user to attach again */
    if (context->attach_gdb && debugger_pid) {
        int ret = waitpid(debugger_pid, nullptr, WNOHANG);
        if (ret == debugger_pid) {
            context->attach_gdb = false;
            debugger_pid = 0;
            launchGdbButton->setEnabled(true);
        }
    }
}

void MainWindow::updateUIFrame()
{
    /* Process everything that needs to be performed exactly once per frame */
    
    /* Autohold and autofire need to be applied at the beginning of each
     * frame, only if the input editor is visible. It should not apply when
     * rewinding to a previous frame. */
    if (inputEditorWindow->isVisible() && !context->seek_frame)
        gameLoop->movie.applyAutoHoldFire();

    /* Keep all ram watches frozen */
    ramWatchWindow->update_frozen();
}

void MainWindow::updateMovieParams()
{
    if ((context->config.sc.recording != SharedConfig::NO_RECORDING) &&
        (gameLoop->movie.loadMovie() == 0)) {
        authorField->setReadOnly(true);

        /* Format movie length */
        double msec = gameLoop->movie.header->length_sec + ((double)gameLoop->movie.header->length_nsec)/1000000000.0;
        int immin = (int)(msec/60);
        double dmsec = msec - 60*immin;
        movieLength->setText(QString("Movie length: %1m %2s").arg(immin).arg(dmsec, 0, 'f', 2));

        /* If move exists, default to read mode except in non-interactive mode */
        if (context->interactive) {
            moviePlayback->setChecked(true);
            context->config.sc.recording = SharedConfig::RECORDING_READ;
            context->config.sc_modified = true;
        }
        else {
            if (context->config.sc.recording == SharedConfig::RECORDING_READ)
                moviePlayback->setChecked(true);
            else
                movieRecording->setChecked(true);
        }

        annotationsWindow->update();
        saveMovieAction->setEnabled(true);
        exportMovieAction->setEnabled(true);
        settingsMovieAction->setEnabled(true);
    }
    else {
        context->config.sc.movie_framecount = 0;
        context->rerecord_count = 0;
        gameLoop->movie.header->authors = "";
        authorField->setReadOnly(false);
        movieLength->setText("Movie length: -");
        movieRecording->setChecked(true);
        if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
            context->config.sc.recording = SharedConfig::RECORDING_WRITE;
            context->config.sc_modified = true;
        }
        annotationsWindow->clear();
        
        saveMovieAction->setEnabled(false);
        exportMovieAction->setEnabled(false);
        settingsMovieAction->setEnabled(false);
    }
    inputEditorWindow->resetInputs();
    movieFrameCount->setValue(context->config.sc.movie_framecount);
    rerecordCount->setValue(context->rerecord_count);
    authorField->setText(gameLoop->movie.header->authors.c_str());
    fpsNumField->setValue(context->config.sc.initial_framerate_num);
    fpsDenField->setValue(context->config.sc.initial_framerate_den);
    elapsedTimeSec->setValue(context->config.sc.initial_monotonic_time_sec);
    elapsedTimeNsec->setValue(context->config.sc.initial_monotonic_time_nsec);
    realTimeSec->setValue(context->config.sc.initial_time_sec);
    realTimeNsec->setValue(context->config.sc.initial_time_nsec);
    autoRestartAction->setChecked(context->config.auto_restart);
}

void MainWindow::updateUIFromConfig()
{
    /* We don't want to trigger the signal here */
    disconnect(gamePath, &QComboBox::editTextChanged, this, &MainWindow::slotGamePathChanged);
    gamePath->setEditText(context->gamepath.c_str());
    connect(gamePath, &QComboBox::editTextChanged, this, &MainWindow::slotGamePathChanged);

    cmdOptions->clear();
    for (const auto& args : context->config.recent_args) {
        cmdOptions->addItem(QString(args.c_str()));
    }
    cmdOptions->setEditText(context->config.gameargs.c_str());
    
    moviePath->setText(context->config.moviefile.c_str());

    movieBox->setChecked(!(context->config.sc.recording == SharedConfig::NO_RECORDING));

    updateMovieParams();

    pauseCheck->setChecked(!context->config.sc.running);
    fastForwardCheck->setChecked(context->config.sc.fastforward);

    setRadioFromList(slowdownGroup, context->config.sc.speed_divisor);

    mouseModeAction->setChecked(context->config.sc.mouse_mode_relative);

    busyloopAction->setChecked(context->config.sc.busyloop_detection);

    setCheckboxesFromMask(fastforwardGroup, context->config.sc.fastforward_mode);
    setRadioFromList(fastforwardRenderGroup, context->config.sc.fastforward_render);

    switch (context->config.debugger) {
    case Config::DEBUGGER_GDB:
        launchGdbButton->setDefaultAction(launchGdbAction);
        break;
    case Config::DEBUGGER_LLDB:
        launchGdbButton->setDefaultAction(launchLldbAction);
        break;
    case Config::DEBUGGER_STRACE:
        launchGdbButton->setDefaultAction(launchStraceAction);
        break;
    }

    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    statusBar()->removeWidget(statusIcon);
    statusBar()->removeWidget(statusSoft);
    statusBar()->removeWidget(statusMute);

    if (!context->config.sc.opengl_soft) {
        statusBar()->addWidget(statusIcon);
        statusIcon->show();
        statusBar()->addWidget(statusSoft);
        statusSoft->show();
        return;
    }
    if (!context->config.sc.audio_mute) {
        statusBar()->addWidget(statusIcon);
        statusIcon->show();
        statusBar()->addWidget(statusMute);
        statusMute->show();
        return;
    }
}

void MainWindow::slotLaunchGdb() {
    context->config.debugger = Config::DEBUGGER_GDB;
    launchGdbButton->setDefaultAction(launchGdbAction);

    slotLaunch(true);
}

void MainWindow::slotLaunchLldb() {
    context->config.debugger = Config::DEBUGGER_LLDB;
    launchGdbButton->setDefaultAction(launchLldbAction);

    slotLaunch(true);
}

void MainWindow::slotLaunchStrace() {
    context->config.debugger = Config::DEBUGGER_STRACE;
    launchGdbButton->setDefaultAction(launchStraceAction);

    slotLaunch(true);
}

void MainWindow::slotLaunch(bool attach_gdb)
{
    /* Special case of attaching gdb to an already running process */
    if (attach_gdb && !context->attach_gdb && (context->status == Context::ACTIVE)) {
        context->attach_gdb = true;
        launchGdbButton->setEnabled(false);

        debugger_pid = fork();
        if (debugger_pid == 0) {
            GameThread::attach(context);
        }
        
        return;
    }

    if (context->status != Context::INACTIVE)
        return;

    /* Do we attach gdb ? */
    context->attach_gdb = attach_gdb;
    
    /* Perform all checks */
    if (!ErrorChecking::allChecks(context))
        return;

    gameLoop->movie.header->authors = authorField->text().toStdString();

    /* Set a few parameters */
    context->config.sc.initial_framerate_num = fpsNumField->value();
    context->config.sc.initial_framerate_den = fpsDenField->value();
    context->config.sc.initial_monotonic_time_sec = elapsedTimeSec->value();
    context->config.sc.initial_monotonic_time_nsec = elapsedTimeNsec->value();
    context->config.sc.initial_time_sec = realTimeSec->value();
    context->config.sc.initial_time_nsec = realTimeNsec->value();

    context->config.sc.sigint_upon_launch &= context->attach_gdb;

    context->config.gameargs = cmdOptions->currentText().toStdString();

    /* Ask if the user allow library downloads */
    if (context->config.allow_downloads == -1) {
        QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Libraries download"), 
        tr("Some games require old libraries unavailable in recent systems, and libTAS can download them for you, if you allow it (this can be changed at any time in the settings)."), QMessageBox::Yes | QMessageBox::No);
        context->config.allow_downloads = (btn == QMessageBox::Yes);
    }

    /* Save the config */
    context->config.save(context->gamepath);

    /* Update the game args list */
    cmdOptions->clear();
    for (const auto& args : context->config.recent_args) {
        cmdOptions->addItem(QString(args.c_str()));
    }
    cmdOptions->setEditText(context->config.gameargs.c_str());

    /* Check that there might be a thread from a previous game execution */
    if (game_thread.joinable())
        game_thread.join();

    /* Start game */
    context->status = Context::STARTING;
    updateStatus(Context::STARTING);
    settingsWindow->update(Context::STARTING);
    game_thread = std::thread{&GameLoop::start, gameLoop};
}

void MainWindow::slotStop()
{
    if (context->status == Context::QUITTING) {
        if (context->game_pid != 0) {
            /* Terminate the game process */
            kill(context->game_pid, SIGKILL);
        } else {
            /* In this case, the game has closed (because game_pid has been set
             * to 0 by loopExit) yet status is still QUITTING, because status
             * depends on whether fork_pid is alive, not the game itself.
             *
             * So in this case, the game process and the forked process have
             * different pids (which can happen with gdb, for example) and the
             * game process has quit, but the forked process has not.
             */
            gameLoop->killForkProcess();
        }
        return;
    }

    if (context->status == Context::ACTIVE) {
        context->status = Context::QUITTING;
        updateStatus(Context::QUITTING);
        settingsWindow->update(Context::QUITTING);
        game_thread.detach();
    }
}

void MainWindow::slotBrowseGamePath()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Game path"), context->gamepath.c_str());
    if (filename.isNull())
        return;

    gamePath->setEditText(filename);
    slotGamePathChanged();
}

void MainWindow::slotGamePathChanged()
{
    /* Save the previous config */
    context->config.save(context->gamepath);

    context->gamepath = gamePath->currentText().toStdString();

    /* Check if gamepath exists, otherwise disable Start buttons and return */
    if (access(context->gamepath.c_str(), F_OK) != 0) {
        launchButton->setEnabled(false);
        launchGdbButton->setEnabled(false);
        return;
    }
    
    launchButton->setEnabled(true);
    
    /* Disable `Start and attach gdb` if Windows game */
    int gameArch = extractBinaryType(context->gamepath) & BT_TYPEMASK;
    if (gameArch == BT_PE32 || gameArch == BT_PE32P || gameArch == BT_NE)
        launchGdbButton->setEnabled(false);
    else
        launchGdbButton->setEnabled(true);
    
    /* Try to load the game-specific pref file */
    context->config.load(context->gamepath);

    updateRecentGamepaths();

    if (!context->is_soft_dirty) {
        context->config.sc.savestate_settings &= ~SharedConfig::SS_INCREMENTAL;
    }

    /* Update the UI accordingly */
    updateUIFromConfig();
    encodeWindow->update_config();
    executableWindow->update_config();
    inputWindow->update();
    inputEditorWindow->update_config();
    settingsWindow->loadConfig();
}

void MainWindow::slotBrowseMoviePath()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Choose a movie file"), context->config.moviefile.c_str(), tr("libTAS movie files (*.ltm)"), Q_NULLPTR, QFileDialog::DontConfirmOverwrite);
    if (filename.isNull())
        return;

    moviePath->setText(filename);
    context->config.moviefile = filename.toStdString();

    updateMovieParams();
}

void MainWindow::slotMoviePathChanged()
{
    context->config.moviefile = moviePath->text().toStdString();
    updateMovieParams();
}

void MainWindow::slotSaveMovie()
{
    if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
        int ret = gameLoop->movie.saveMovie();
        if (ret < 0) {
            QMessageBox::warning(this, "Warning", gameLoop->movie.errorString(ret));
        }
    }
}

void MainWindow::slotExportMovie()
{
    if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
        QString filename = QFileDialog::getSaveFileName(this, tr("Choose a movie file"), context->config.moviefile.c_str(), tr("libTAS movie files (*.ltm)"));
        if (!filename.isNull()) {
            int ret = gameLoop->movie.saveMovie(filename.toStdString());
            if (ret < 0) {
                QMessageBox::warning(this, "Warning", gameLoop->movie.errorString(ret));
            }
        }
    }
}

void MainWindow::slotPauseMovie()
{
    context->pause_frame = QInputDialog::getInt(this, tr("Pause Movie"),
        tr("Pause movie at the indicated frame. Fill zero to disable. Fill a negative value to pause at a number of frames before the end of the movie."),
        context->pause_frame);
}

void MainWindow::slotPause(bool checked)
{
    if (context->status == Context::INACTIVE) {
        /* If the game is inactive, set the value directly */
        context->config.sc.running = !checked;
    }
    else {
        /* Else, let the game thread set the value */
        context->hotkey_pressed_queue.push(HOTKEY_PLAYPAUSE);
    }
}

void MainWindow::slotFastForward(bool checked)
{
    if (context->status == Context::INACTIVE) {
        /* If the game is inactive, set the value directly */
        context->config.sc.fastforward = checked;
    }
    else {
        /* Else, let the game thread set the value */
        context->hotkey_pressed_queue.push(HOTKEY_TOGGLE_FASTFORWARD);
    }
}

void MainWindow::slotSaveState(QAction *action)
{
    int hotkey_type = action->data().toInt();
    context->hotkey_pressed_queue.push(hotkey_type);
}

void MainWindow::slotMovieEnable(bool checked)
{
    if (checked) {
        if (movieRecording->isChecked()) {
            context->config.sc.recording = SharedConfig::RECORDING_WRITE;
        }
        else {
            context->config.sc.recording = SharedConfig::RECORDING_READ;
        }
    }
    else {
        context->config.sc.recording = SharedConfig::NO_RECORDING;
    }
    updateMovieParams();

    annotateMovieAction->setEnabled(checked);
    context->config.sc_modified = true;
}

void MainWindow::slotMovieRecording()
{
    /* If the game is running, we let the main thread deal with movie toggling.
     * Else, we set the recording mode.
     */
    if (context->status == Context::INACTIVE) {
        if (movieRecording->isChecked()) {
            context->config.sc.recording = SharedConfig::RECORDING_WRITE;
            authorField->setReadOnly(false);
        }
        else {
            context->config.sc.recording = SharedConfig::RECORDING_READ;
            authorField->setReadOnly(true);
        }
    }
    else {
        context->hotkey_pressed_queue.push(HOTKEY_READWRITE);
    }
    context->config.sc_modified = true;
}

void MainWindow::slotToggleEncode()
{
    /* Prompt a confirmation message for overwriting an encode file, except
     * when in non-interactive mode */
    if (!context->config.sc.av_dumping && context->interactive) {
        struct stat sb;
        if (stat(context->config.dumpfile.c_str(), &sb) == 0 && sb.st_size != 0) {
            /* Pause the game during the choice */
            context->config.sc.running = false;
            context->config.sc_modified = true;

            QMessageBox::StandardButton btn = QMessageBox::question(this, "File overwrite", QString("The encode file %1 does exist. Do you want to overwrite it?").arg(context->config.dumpfile.c_str()), QMessageBox::Ok | QMessageBox::Cancel);
            if (btn != QMessageBox::Ok)
                return;
        }
    }

    /* If the game is running, we let the main thread deal with dumping.
     * Else, we set the dumping mode ourselved.
     */
    if (context->status == Context::INACTIVE) {
        context->config.sc.av_dumping = !context->config.sc.av_dumping;
        context->config.sc_modified = true;
        updateSharedConfigChanged();
    }
    else {
        /* TODO: Using directly the hotkey does not check for existing file */
        context->hotkey_pressed_queue.push(HOTKEY_TOGGLE_ENCODE);
    }
}

void MainWindow::slotScreenshot()
{
    /* Prompt for screenshot filename and path */
    if (context->interactive) {
        QString defaultPath = QString(context->config.screenshotfile.c_str());
        
        QString screenshotPath = QFileDialog::getSaveFileName(this,
            tr("Choose a screenshot file"),
            defaultPath);
            
        if (!screenshotPath.isNull())
            context->config.screenshotfile = screenshotPath.toStdString();
        else
            return;
    }
    
    context->hotkey_pressed_queue.push(HOTKEY_SCREENSHOT);
}

void MainWindow::slotRealTimeFormat()
{
    char buf[22];
    struct timespec realtime = {realTimeSec->value(), realTimeNsec->value()};
    struct tm tm;
    gmtime_r(&realtime.tv_sec, &tm);
    strftime(buf, 21, "%FT%TZ", gmtime(&realtime.tv_sec));
    realTimeFormat->setText(QString(buf));
}

void MainWindow::alertOffer(QString alert_msg, void* promise)
{
    std::promise<bool>* saveAnswer = static_cast<std::promise<bool>*>(promise);

    if (context->interactive) {
        QMessageBox::StandardButton btn = QMessageBox::question(this, "", alert_msg, QMessageBox::Yes | QMessageBox::No);
        saveAnswer->set_value(btn == QMessageBox::Yes);
    }
    else {
        /* Always answer 'yes' in non-interactive mode */
        saveAnswer->set_value(true);
    }
}

void MainWindow::alertDialog(QString alert_msg)
{
    /* Don't show any dialog box in non-interactive mode */
    if (!context->interactive)
        return;

    /* Pause the game */
    context->config.sc.running = false;
    context->config.sc_modified = true;
    updateSharedConfigChanged();

    /* Show alert window */
    QMessageBox::warning(this, "Warning", alert_msg);
}

void MainWindow::updateFramerate()
{
    fpsNumField->setValue(context->current_framerate_num);
    fpsDenField->setValue(context->current_framerate_den);
}
