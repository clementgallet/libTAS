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

#include "MainWindow.h"
#include "EncodeWindow.h"
#include "ExecutableWindow.h"
#include "InputWindow.h"
#include "ControllerTabWindow.h"
#include "GameInfoWindow.h"
#include "GameSpecificWindow.h"
#include "RamSearchWindow.h"
#include "RamWatchWindow.h"
#include "InputEditorWindow.h"
#include "InputEditorView.h"
#include "InputEditorModel.h"
#include "OsdWindow.h"
#include "AnnotationsWindow.h"
#include "AutoSaveWindow.h"
#include "TimeTraceWindow.h"
#include "TimeTraceModel.h"
#include "../movie/MovieFile.h"
#include "ErrorChecking.h"
#include "../../shared/version.h"
#include "../lua/Main.h"
#include "../utils.h"
#include "../GameEvents.h"

#include <iostream>
#include <future>
#include <sys/stat.h>
#include <csignal> // kill
#include <unistd.h> // access
#include <limits>

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
    encodeWindow = new EncodeWindow(c, this);
    inputWindow = new InputWindow(c, this);
    executableWindow = new ExecutableWindow(c, this);
    controllerTabWindow = new ControllerTabWindow(c, this);
    gameInfoWindow = new GameInfoWindow(this);
    gameSpecificWindow = new GameSpecificWindow(c, this);
    ramSearchWindow = new RamSearchWindow(c, this);
    ramWatchWindow = new RamWatchWindow(c, this);
    inputEditorWindow = new InputEditorWindow(c, this);
    osdWindow = new OsdWindow(c, this);
    annotationsWindow = new AnnotationsWindow(c, this);
    autoSaveWindow = new AutoSaveWindow(c, this);
    timeTraceWindow = new TimeTraceWindow(c, this);

    connect(gameLoop, &GameLoop::inputsToBeChanged, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::beginModifyInputs);
    connect(gameLoop->gameEvents, &GameEvents::inputsToBeChanged, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::beginModifyInputs);
    connect(gameLoop, &GameLoop::inputsChanged, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::endModifyInputs);
    connect(gameLoop->gameEvents, &GameEvents::inputsChanged, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::endModifyInputs);
    connect(gameLoop, &GameLoop::inputsToBeAdded, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::beginAddInputs);
    connect(gameLoop, &GameLoop::inputsAdded, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::endAddInputs);
    connect(gameLoop, &GameLoop::inputsToBeEdited, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::beginEditInputs);
    connect(gameLoop, &GameLoop::inputsEdited, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::endEditInputs);
    connect(gameLoop->gameEvents, &GameEvents::inputsEdited, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::endEditInputs);
    connect(gameLoop, &GameLoop::isInputEditorVisible, inputEditorWindow, &InputEditorWindow::isWindowVisible, Qt::DirectConnection);
    connect(gameLoop, &GameLoop::getRamWatch, ramWatchWindow, &RamWatchWindow::slotGet, Qt::DirectConnection);
    connect(gameLoop->gameEvents, &GameEvents::savestatePerformed, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::registerSavestate);
    connect(gameLoop, &GameLoop::invalidateSavestates, inputEditorWindow->inputEditorView->inputEditorModel, &InputEditorModel::invalidateSavestates);
    connect(gameLoop, &GameLoop::getTimeTrace, timeTraceWindow->timeTraceModel, &TimeTraceModel::addCall);

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
    connect(fpsNumField, QOverload<int>::of(&QSpinBox::valueChanged),[=](int i){context->config.sc.framerate_num = i;});

    fpsDenField = new QSpinBox();
    fpsDenField->setMaximum(std::numeric_limits<int>::max());
    fpsDenField->setMinimum(1);
    connect(fpsDenField, QOverload<int>::of(&QSpinBox::valueChanged),[=](int i){context->config.sc.framerate_den = i;});

    fpsValues = new QLabel("Current FPS: - / -");

    /* Re-record count */
    rerecordCount = new QSpinBox();
    rerecordCount->setReadOnly(true);
    rerecordCount->setMaximum(std::numeric_limits<int>::max());

    /* Initial time */
    initialTimeSec = new QSpinBox();
    initialTimeSec->setMaximum(std::numeric_limits<int>::max());
    initialTimeSec->setMinimumWidth(50);
    initialTimeNsec = new QSpinBox();
    initialTimeNsec->setMaximum(std::numeric_limits<int>::max());
    initialTimeNsec->setMinimumWidth(50);
    disabledWidgetsOnStart.append(initialTimeSec);
    disabledWidgetsOnStart.append(initialTimeNsec);

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

    connect(launchGdbAction, &QAction::triggered, this, &MainWindow::slotLaunchGdb);
    connect(launchLldbAction, &QAction::triggered, this, &MainWindow::slotLaunchLldb);

    /* launchGdbButton is a special case, it's explicitly disabled along with
     * all the other widgets on launch
     */
    //disabledWidgetsOnStart.append(launchGdbButton);

#ifdef __unix__
    launchGdbButton->setPopupMode(QToolButton::MenuButtonPopup);

    QMenu *launchGdbButtonMenu = new QMenu();
    launchGdbButton->setMenu(launchGdbButtonMenu);

    launchGdbButtonMenu->addAction(launchGdbAction);
    launchGdbButtonMenu->addAction(launchLldbAction);
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

    QGridLayout *movieCountLayout = new QGridLayout;
    movieCountLayout->addWidget(new QLabel(tr("Movie frame count:")), 0, 0);
    movieCountLayout->addWidget(movieFrameCount, 0, 1);
    movieCountLayout->addWidget(movieLength, 0, 3);
    movieCountLayout->addWidget(new QLabel(tr("Rerecord count:")), 1, 0);
    movieCountLayout->addWidget(rerecordCount, 1, 1);
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

    QHBoxLayout *generalFrameLayout = new QHBoxLayout;
    generalFrameLayout->addWidget(new QLabel(tr("Frame:")));
    generalFrameLayout->addStretch(1);
    generalFrameLayout->addWidget(frameCount);
    generalFrameLayout->addStretch(1);
    generalFrameLayout->addWidget(currentLength);
    generalFrameLayout->addStretch(1);

    QHBoxLayout *generalFpsLayout = new QHBoxLayout;
    generalFpsLayout->addWidget(new QLabel(tr("Frames per second:")));
    generalFpsLayout->addStretch(1);
    generalFpsLayout->addWidget(fpsNumField);
    generalFpsLayout->addWidget(new QLabel(tr("/")));
    generalFpsLayout->addWidget(fpsDenField);
    generalFpsLayout->addStretch(1);
    generalFpsLayout->addWidget(fpsValues);
    generalFpsLayout->addStretch(1);

    QHBoxLayout *generalTimeLayout = new QHBoxLayout;
    generalTimeLayout->addWidget(new QLabel(tr("System time:")));
    generalTimeLayout->addStretch(1);
    generalTimeLayout->addWidget(initialTimeSec);
    generalTimeLayout->addWidget(new QLabel(tr("sec")));
    generalTimeLayout->addStretch(1);
    generalTimeLayout->addWidget(initialTimeNsec);
    generalTimeLayout->addWidget(new QLabel(tr("nsec")));
    generalTimeLayout->addStretch(1);

    QHBoxLayout *generalControlLayout = new QHBoxLayout;
    generalControlLayout->addWidget(pauseCheck);
    generalControlLayout->addWidget(fastForwardCheck);
    generalControlLayout->addStretch(1);

    generalLayout->addLayout(generalFrameLayout);
    generalLayout->addLayout(generalFpsLayout);
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
}

MainWindow::~MainWindow()
{
    delete gameLoop;

    if (game_thread.joinable())
        game_thread.detach();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(!context->interactive) {
        event->accept();
        return;
    }

    if (context->status == Context::INACTIVE)
        event->accept();

    if(gameLoop->movie.inputs->modifiedSinceLastSave) {
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
    movieEndGroup = new QActionGroup(this);
    connect(movieEndGroup, &QActionGroup::triggered, this, &MainWindow::slotMovieEnd);

    addActionCheckable(movieEndGroup, tr("Keep Reading"), Config::MOVIEEND_READ);
    addActionCheckable(movieEndGroup, tr("Switch to Writing"), Config::MOVIEEND_WRITE);

    screenResGroup = new QActionGroup(this);
    addActionCheckable(screenResGroup, tr("Native"), 0);
    addActionCheckable(screenResGroup, tr("640x480 (4:3)"), (640 << 16) | 480);
    addActionCheckable(screenResGroup, tr("800x600 (4:3)"), (800 << 16) | 600);
    addActionCheckable(screenResGroup, tr("1024x768 (4:3)"), (1024 << 16) | 768);
    addActionCheckable(screenResGroup, tr("1280x720 (16:9)"), (1280 << 16) | 720);
    addActionCheckable(screenResGroup, tr("1280x800 (16:10)"), (1280 << 16) | 800);
    addActionCheckable(screenResGroup, tr("1400x1050 (4:3)"), (1400 << 16) | 1050);
    addActionCheckable(screenResGroup, tr("1440x900 (16:10)"), (1440 << 16) | 900);
    addActionCheckable(screenResGroup, tr("1600x900 (16:9)"), (1600 << 16) | 900);
    addActionCheckable(screenResGroup, tr("1680x1050 (16:10)"), (1680 << 16) | 1050);
    addActionCheckable(screenResGroup, tr("1920x1080 (16:9)"), (1920 << 16) | 1080);
    addActionCheckable(screenResGroup, tr("1920x1200 (16:10)"), (1920 << 16) | 1200);
    addActionCheckable(screenResGroup, tr("2560x1440 (16:9)"), (2560 << 16) | 1440);
    addActionCheckable(screenResGroup, tr("3840x2160 (16:9)"), (3840 << 16) | 2160);
    connect(screenResGroup, &QActionGroup::triggered, this, &MainWindow::slotScreenRes);

#ifdef LIBTAS_ENABLE_HUD
    osdGroup = new QActionGroup(this);
    osdGroup->setExclusive(false);
    connect(osdGroup, &QActionGroup::triggered, this, &MainWindow::slotOsd);

    addActionCheckable(osdGroup, tr("Frame Count"), SharedConfig::OSD_FRAMECOUNT);
    addActionCheckable(osdGroup, tr("Inputs"), SharedConfig::OSD_INPUTS);
    addActionCheckable(osdGroup, tr("Messages"), SharedConfig::OSD_MESSAGES);
    addActionCheckable(osdGroup, tr("Ram Watches"), SharedConfig::OSD_RAMWATCHES);
    addActionCheckable(osdGroup, tr("Lua"), SharedConfig::OSD_LUA);
#endif

    frequencyGroup = new QActionGroup(this);

    addActionCheckable(frequencyGroup, tr("8000 Hz"), 8000);
    addActionCheckable(frequencyGroup, tr("11025 Hz"), 11025);
    addActionCheckable(frequencyGroup, tr("12000 Hz"), 12000);
    addActionCheckable(frequencyGroup, tr("16000 Hz"), 16000);
    addActionCheckable(frequencyGroup, tr("22050 Hz"), 22050);
    addActionCheckable(frequencyGroup, tr("24000 Hz"), 24000);
    addActionCheckable(frequencyGroup, tr("32000 Hz"), 32000);
    addActionCheckable(frequencyGroup, tr("44100 Hz"), 44100);
    addActionCheckable(frequencyGroup, tr("48000 Hz"), 48000);

    bitDepthGroup = new QActionGroup(this);

    addActionCheckable(bitDepthGroup, tr("8 bit"), 8);
    addActionCheckable(bitDepthGroup, tr("16 bit"), 16);

    channelGroup = new QActionGroup(this);

    addActionCheckable(channelGroup, tr("Mono"), 1);
    addActionCheckable(channelGroup, tr("Stereo"), 2);

    localeGroup = new QActionGroup(this);

    addActionCheckable(localeGroup, tr("English"), SharedConfig::LOCALE_ENGLISH);
    addActionCheckable(localeGroup, tr("Japanese"), SharedConfig::LOCALE_JAPANESE);
    addActionCheckable(localeGroup, tr("Korean"), SharedConfig::LOCALE_KOREAN);
    addActionCheckable(localeGroup, tr("Chinese, Simplified"), SharedConfig::LOCALE_CHINESE_SIMPLIFIED);
    addActionCheckable(localeGroup, tr("Chinese, Traditional"), SharedConfig::LOCALE_CHINESE_TRADITIONAL);
    addActionCheckable(localeGroup, tr("Spanish"), SharedConfig::LOCALE_SPANISH);
    addActionCheckable(localeGroup, tr("German"), SharedConfig::LOCALE_GERMAN);
    addActionCheckable(localeGroup, tr("French"), SharedConfig::LOCALE_FRENCH);
    addActionCheckable(localeGroup, tr("Italian"), SharedConfig::LOCALE_ITALIAN);
    addActionCheckable(localeGroup, tr("Native"), SharedConfig::LOCALE_NATIVE);

    timeMainGroup = new QActionGroup(this);
    timeMainGroup->setExclusive(false);

    addActionCheckable(timeMainGroup, tr("time()"), SharedConfig::TIMETYPE_TIME);
    addActionCheckable(timeMainGroup, tr("gettimeofday()"), SharedConfig::TIMETYPE_GETTIMEOFDAY);
    addActionCheckable(timeMainGroup, tr("clock()"), SharedConfig::TIMETYPE_CLOCK);
    addActionCheckable(timeMainGroup, tr("clock_gettime()"), SharedConfig::TIMETYPE_CLOCKGETTIME);
    addActionCheckable(timeMainGroup, tr("SDL_GetTicks()"), SharedConfig::TIMETYPE_SDLGETTICKS);
    addActionCheckable(timeMainGroup, tr("SDL_GetPerformanceCounter()"), SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER);
    addActionCheckable(timeMainGroup, tr("GetTickCount()"), SharedConfig::TIMETYPE_GETTICKCOUNT);
    addActionCheckable(timeMainGroup, tr("GetTickCount64()"), SharedConfig::TIMETYPE_GETTICKCOUNT64);
    addActionCheckable(timeMainGroup, tr("QueryPerformanceCounter()"), SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER);

    timeSecGroup = new QActionGroup(this);
    timeSecGroup->setExclusive(false);

    addActionCheckable(timeSecGroup, tr("time()"), SharedConfig::TIMETYPE_TIME);
    addActionCheckable(timeSecGroup, tr("gettimeofday()"), SharedConfig::TIMETYPE_GETTIMEOFDAY);
    addActionCheckable(timeSecGroup, tr("clock()"), SharedConfig::TIMETYPE_CLOCK);
    addActionCheckable(timeSecGroup, tr("clock_gettime()"), SharedConfig::TIMETYPE_CLOCKGETTIME);
    addActionCheckable(timeSecGroup, tr("SDL_GetTicks()"), SharedConfig::TIMETYPE_SDLGETTICKS);
    addActionCheckable(timeSecGroup, tr("SDL_GetPerformanceCounter()"), SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER);
    addActionCheckable(timeSecGroup, tr("GetTickCount()"), SharedConfig::TIMETYPE_GETTICKCOUNT);
    addActionCheckable(timeSecGroup, tr("GetTickCount64()"), SharedConfig::TIMETYPE_GETTICKCOUNT64);
    addActionCheckable(timeSecGroup, tr("QueryPerformanceCounter()"), SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER);

    waitGroup = new QActionGroup(this);
    addActionCheckable(waitGroup, tr("Native waits"), SharedConfig::WAIT_NATIVE, "Don't modify wait calls");
    addActionCheckable(waitGroup, tr("Infinite waits"), SharedConfig::WAIT_INFINITE, "Waits have infinite timeout. Sync-proof, but may softlock");
    addActionCheckable(waitGroup, tr("Full infinite waits"), SharedConfig::WAIT_FULL_INFINITE, "Advance time for the full timeout and wait infinitely. Sync-proof, but may still softlock and may advance time too much resulting in incorrect frame boundaries");
    addActionCheckable(waitGroup, tr("Finite waits"), SharedConfig::WAIT_FINITE, "Try to wait, and advance time if we get a timeout. Prevent softlocks but not perfectly sync-proof");
    addActionCheckable(waitGroup, tr("Full waits"), SharedConfig::WAIT_FULL, "Advance time and try to wait.");
    addActionCheckable(waitGroup, tr("No waits"), SharedConfig::NO_WAIT, "Wait with zero timeout.");

    asyncGroup = new QActionGroup(this);
    asyncGroup->setExclusive(false);
    addActionCheckable(asyncGroup, tr("jsdev"), SharedConfig::ASYNC_JSDEV);
    addActionCheckable(asyncGroup, tr("evdev"), SharedConfig::ASYNC_EVDEV);
    addActionCheckable(asyncGroup, tr("XEvents at frame beginning"), SharedConfig::ASYNC_XEVENTS_BEG);
    addActionCheckable(asyncGroup, tr("XEvents at frame end"), SharedConfig::ASYNC_XEVENTS_END);
    addActionCheckable(asyncGroup, tr("SDL events at frame beginning"), SharedConfig::ASYNC_SDLEVENTS_BEG);
    addActionCheckable(asyncGroup, tr("SDL events at frame end"), SharedConfig::ASYNC_SDLEVENTS_END);

    savestateGroup = new QActionGroup(this);
    savestateGroup->setExclusive(false);
    connect(savestateGroup, &QActionGroup::triggered, this, &MainWindow::slotSavestate);

    QAction *action = addActionCheckable(savestateGroup, tr("Incremental savestates"), SharedConfig::SS_INCREMENTAL, tr("Optimize savestate size by only storing the memory pages that have been modified, at the cost of slightly more processing"));
    if (!context->is_soft_dirty) {
        action->setEnabled(false);
        context->config.sc.savestate_settings &= ~SharedConfig::SS_INCREMENTAL;
    }

    action = addActionCheckable(savestateGroup, tr("Store savestates in RAM"), SharedConfig::SS_RAM, tr("Storing savestates in RAM can provide a speed-up, but beware of your available memory"));
    disabledActionsOnStart.append(action);
    addActionCheckable(savestateGroup, tr("Backtrack savestate"), SharedConfig::SS_BACKTRACK, tr("Save a state whenether a thread is created/destroyed, so that you can rewind to the earliest time possible"));
    addActionCheckable(savestateGroup, tr("Compressed savestates"), SharedConfig::SS_COMPRESSED);
    addActionCheckable(savestateGroup, tr("Skip unmapped pages"), SharedConfig::SS_PRESENT, tr("Shorter savestates, but causes crashes in some games"));
    addActionCheckable(savestateGroup, tr("Fork to save states"), SharedConfig::SS_FORK, tr("Game can resume immediately without waiting for the state to be saved"));

    debugStateGroup = new QActionGroup(this);
    debugStateGroup->setExclusive(false);
    connect(debugStateGroup, &QActionGroup::triggered, this, &MainWindow::slotDebugState);

    addActionCheckable(debugStateGroup, tr("Uncontrolled time"), SharedConfig::DEBUG_UNCONTROLLED_TIME, "Let the game access to the real system time, only for debugging purpose");
    addActionCheckable(debugStateGroup, tr("Native events"), SharedConfig::DEBUG_NATIVE_EVENTS, "Let the game access to the real system events, only for debugging purpose");
    addActionCheckable(debugStateGroup, tr("Keep main first thread"), SharedConfig::DEBUG_MAIN_FIRST_THREAD, "Keep main thread as first thread, when rendering is done in another thread. Breaks determinism");

    loggingOutputGroup = new QActionGroup(this);

    addActionCheckable(loggingOutputGroup, tr("Disabled logging"), SharedConfig::NO_LOGGING);
    addActionCheckable(loggingOutputGroup, tr("Log to console"), SharedConfig::LOGGING_TO_CONSOLE);
    addActionCheckable(loggingOutputGroup, tr("Log to file"), SharedConfig::LOGGING_TO_FILE);

    loggingPrintGroup = new QActionGroup(this);
    loggingPrintGroup->setExclusive(false);
    connect(loggingPrintGroup, &QActionGroup::triggered, this, &MainWindow::slotLoggingPrint);

    addActionCheckable(loggingPrintGroup, tr("Main Thread"), LCF_MAINTHREAD);
    addActionCheckable(loggingPrintGroup, tr("Frequent"), LCF_FREQUENT);
    addActionCheckable(loggingPrintGroup, tr("Error"), LCF_ERROR);
    addActionCheckable(loggingPrintGroup, tr("Warning"), LCF_WARNING);
    addActionCheckable(loggingPrintGroup, tr("Info"), LCF_INFO);
    addActionCheckable(loggingPrintGroup, tr("TODO"), LCF_TODO);

    QAction *loggingPrintGroupSeparator = new QAction(loggingPrintGroup);
    loggingPrintGroupSeparator->setSeparator(true);

    addActionCheckable(loggingPrintGroup, tr("AV Dumping"), LCF_DUMP);
    addActionCheckable(loggingPrintGroup, tr("Checkpoint"), LCF_CHECKPOINT);
    addActionCheckable(loggingPrintGroup, tr("Events"), LCF_EVENTS);
    addActionCheckable(loggingPrintGroup, tr("File IO"), LCF_FILEIO);
    addActionCheckable(loggingPrintGroup, tr("Hook"), LCF_HOOK);
    addActionCheckable(loggingPrintGroup, tr("Joystick"), LCF_JOYSTICK);
    addActionCheckable(loggingPrintGroup, tr("Keyboard"), LCF_KEYBOARD);
    addActionCheckable(loggingPrintGroup, tr("Locale"), LCF_LOCALE);
    addActionCheckable(loggingPrintGroup, tr("Mouse"), LCF_MOUSE);
    addActionCheckable(loggingPrintGroup, tr("OpenGL/Vulkan"), LCF_OGL);
    addActionCheckable(loggingPrintGroup, tr("Random"), LCF_RANDOM);
    addActionCheckable(loggingPrintGroup, tr("SDL"), LCF_SDL);
    addActionCheckable(loggingPrintGroup, tr("Signals"), LCF_SIGNAL);
    addActionCheckable(loggingPrintGroup, tr("Sleep"), LCF_SLEEP);
    addActionCheckable(loggingPrintGroup, tr("Socket"), LCF_SOCKET);
    addActionCheckable(loggingPrintGroup, tr("Sound"), LCF_SOUND);
    addActionCheckable(loggingPrintGroup, tr("Steam"), LCF_STEAM);
    addActionCheckable(loggingPrintGroup, tr("System"), LCF_SYSTEM);
    addActionCheckable(loggingPrintGroup, tr("Time Get"), LCF_TIMEGET);
    addActionCheckable(loggingPrintGroup, tr("Time Set"), LCF_TIMESET);
    addActionCheckable(loggingPrintGroup, tr("Timers"), LCF_TIMERS);
    addActionCheckable(loggingPrintGroup, tr("Threads"), LCF_THREAD);
    addActionCheckable(loggingPrintGroup, tr("Wait"), LCF_WAIT);
    addActionCheckable(loggingPrintGroup, tr("Windows"), LCF_WINDOW);
    addActionCheckable(loggingPrintGroup, tr("Wine"), LCF_WINE);

    loggingExcludeGroup = new QActionGroup(this);
    loggingExcludeGroup->setExclusive(false);
    connect(loggingExcludeGroup, &QActionGroup::triggered, this, &MainWindow::slotLoggingExclude);

    // addActionCheckable(loggingExcludeGroup, tr("Main Thread"), LCF_MAINTHREAD);
    addActionCheckable(loggingExcludeGroup, tr("Frequent"), LCF_FREQUENT);
    addActionCheckable(loggingExcludeGroup, tr("Error"), LCF_ERROR);
    addActionCheckable(loggingExcludeGroup, tr("Warning"), LCF_WARNING);
    addActionCheckable(loggingExcludeGroup, tr("Info"), LCF_INFO);
    addActionCheckable(loggingExcludeGroup, tr("TODO"), LCF_TODO);

    QAction *loggingExcludeGroupSeparator = new QAction(loggingExcludeGroup);
    loggingExcludeGroupSeparator->setSeparator(true);

    addActionCheckable(loggingExcludeGroup, tr("AV Dumping"), LCF_DUMP);
    addActionCheckable(loggingExcludeGroup, tr("Checkpoint"), LCF_CHECKPOINT);
    addActionCheckable(loggingExcludeGroup, tr("Events"), LCF_EVENTS);
    addActionCheckable(loggingExcludeGroup, tr("File IO"), LCF_FILEIO);
    addActionCheckable(loggingExcludeGroup, tr("Hook"), LCF_HOOK);
    addActionCheckable(loggingExcludeGroup, tr("Joystick"), LCF_JOYSTICK);
    addActionCheckable(loggingExcludeGroup, tr("Keyboard"), LCF_KEYBOARD);
    addActionCheckable(loggingExcludeGroup, tr("Locale"), LCF_LOCALE);
    addActionCheckable(loggingExcludeGroup, tr("Mouse"), LCF_MOUSE);
    addActionCheckable(loggingExcludeGroup, tr("OpenGL/Vulkan"), LCF_OGL);
    addActionCheckable(loggingExcludeGroup, tr("Random"), LCF_RANDOM);
    addActionCheckable(loggingExcludeGroup, tr("SDL"), LCF_SDL);
    addActionCheckable(loggingExcludeGroup, tr("Signals"), LCF_SIGNAL);
    addActionCheckable(loggingExcludeGroup, tr("Sleep"), LCF_SLEEP);
    addActionCheckable(loggingExcludeGroup, tr("Socket"), LCF_SOCKET);
    addActionCheckable(loggingExcludeGroup, tr("Sound"), LCF_SOUND);
    addActionCheckable(loggingExcludeGroup, tr("Steam"), LCF_STEAM);
    addActionCheckable(loggingExcludeGroup, tr("System"), LCF_SYSTEM);
    addActionCheckable(loggingExcludeGroup, tr("Time Get"), LCF_TIMEGET);
    addActionCheckable(loggingExcludeGroup, tr("Time Set"), LCF_TIMESET);
    addActionCheckable(loggingExcludeGroup, tr("Timers"), LCF_TIMERS);
    addActionCheckable(loggingExcludeGroup, tr("Threads"), LCF_THREAD);
    addActionCheckable(loggingExcludeGroup, tr("Wait"), LCF_WAIT);
    addActionCheckable(loggingExcludeGroup, tr("Windows"), LCF_WINDOW);
    addActionCheckable(loggingExcludeGroup, tr("Wine"), LCF_WINE);

    slowdownGroup = new QActionGroup(this);
    connect(slowdownGroup, &QActionGroup::triggered, this, &MainWindow::slotSlowdown);

    addActionCheckable(slowdownGroup, tr("100% (normal speed)"), 1);
    addActionCheckable(slowdownGroup, tr("50%"), 2);
    addActionCheckable(slowdownGroup, tr("25%"), 4);
    addActionCheckable(slowdownGroup, tr("12%"), 8);

    fastforwardGroup = new QActionGroup(this);
    fastforwardGroup->setExclusive(false);
    connect(fastforwardGroup, &QActionGroup::triggered, this, &MainWindow::slotFastforwardMode);

    addActionCheckable(fastforwardGroup, tr("Skipping sleep"), SharedConfig::FF_SLEEP);
    addActionCheckable(fastforwardGroup, tr("Skipping audio mixing"), SharedConfig::FF_MIXING);
    addActionCheckable(fastforwardGroup, tr("Skipping all rendering"), SharedConfig::FF_RENDERING);

    joystickGroup = new QActionGroup(this);
    addActionCheckable(joystickGroup, tr("None"), 0);
    addActionCheckable(joystickGroup, tr("1"), 1);
    addActionCheckable(joystickGroup, tr("2"), 2);
    addActionCheckable(joystickGroup, tr("3"), 3);
    addActionCheckable(joystickGroup, tr("4"), 4);
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

    /* Movie Menu */
    QMenu *movieMenu = menuBar()->addMenu(tr("Movie"));
    movieMenu->setToolTipsVisible(true);

    action = movieMenu->addAction(tr("Open Movie..."), this, &MainWindow::slotBrowseMoviePath);
    disabledActionsOnStart.append(action);
    saveMovieAction = movieMenu->addAction(tr("Save Movie"), this, &MainWindow::slotSaveMovie);
    saveMovieAction->setEnabled(false);
    exportMovieAction = movieMenu->addAction(tr("Export Movie..."), this, &MainWindow::slotExportMovie);
    exportMovieAction->setEnabled(false);
    action = movieMenu->addAction(tr("Don't enforce movie settings"), this, &MainWindow::slotEnforceMovieSettings);
    action->setCheckable(true);
    action->setToolTip("When checked, settings stored inside the movie metadata won't be enforced (e.g. initial time, mouse/controller support, framerate...). You can then save your movie with the new settings.");

    movieMenu->addSeparator();

    annotateMovieAction = movieMenu->addAction(tr("Annotations..."), annotationsWindow, &AnnotationsWindow::show);
    annotateMovieAction->setEnabled(false);

    movieMenu->addSeparator();

    movieMenu->addAction(tr("Autosave..."), autoSaveWindow, &AutoSaveWindow::show);

    movieMenu->addSeparator();

    movieMenu->addAction(tr("Pause Movie at frame..."), this, &MainWindow::slotPauseMovie);
    autoRestartAction = movieMenu->addAction(tr("Auto-restart game"), this, &MainWindow::slotAutoRestart);
    autoRestartAction->setCheckable(true);
    autoRestartAction->setToolTip("When checked, the game will automatically restart if closed, except when using the Stop button");
    disabledActionsOnStart.append(autoRestartAction);

    QMenu *movieEndMenu = movieMenu->addMenu(tr("On Movie End"));
    movieEndMenu->addActions(movieEndGroup->actions());
    movieMenu->addAction(tr("Input Editor..."), inputEditorWindow, &InputEditorWindow::show);


    /* Video Menu */
    QMenu *videoMenu = menuBar()->addMenu(tr("Video"));
    videoMenu->setToolTipsVisible(true);

    QMenu *screenResMenu = videoMenu->addMenu(tr("Virtual screen resolution"));
    screenResMenu->addActions(screenResGroup->actions());
    disabledWidgetsOnStart.append(screenResMenu);

    renderSoftAction = videoMenu->addAction(tr("Force software rendering"), this, &MainWindow::slotRenderSoft);
    renderSoftAction->setCheckable(true);
    renderSoftAction->setToolTip("Enforce the use of Mesa's OpenGL software driver, which is necessary for savestates to work correctly");
    disabledActionsOnStart.append(renderSoftAction);

    renderPerfAction = videoMenu->addAction(tr("Toggle performance tweaks"), this, &MainWindow::slotRenderPerf);
    renderPerfAction->setCheckable(true);
    renderPerfAction->setToolTip("Change some OpenGL settings to get some performance boost. Should be set on startup to be effective");

#ifdef LIBTAS_ENABLE_HUD
    QMenu *osdMenu = videoMenu->addMenu(tr("OSD"));
    osdMenu->addActions(osdGroup->actions());
    osdMenu->addAction(tr("OSD Options..."), osdWindow, &OsdWindow::exec);
    osdMenu->addSeparator();
    osdEncodeAction = osdMenu->addAction(tr("OSD on video encode"), this, &MainWindow::slotOsdEncode);
    osdEncodeAction->setCheckable(true);
    osdMenu->installEventFilter(this);
#else
    QMenu *osdMenu = videoMenu->addMenu(tr("OSD (disabled)"));
    osdMenu->setEnabled(false);
#endif

    variableFramerateAction = videoMenu->addAction(tr("Variable framerate"), this, &MainWindow::slotVariableFramerate);
    variableFramerateAction->setCheckable(true);
    variableFramerateAction->setToolTip("When checked, you will be able to modify the framerate values during the game execution");
    disabledActionsOnStart.append(variableFramerateAction);

    /* Sound Menu */
    QMenu *soundMenu = menuBar()->addMenu(tr("Sound"));

    QMenu *formatMenu = soundMenu->addMenu(tr("Format"));
    formatMenu->addActions(frequencyGroup->actions());
    formatMenu->addSeparator();
    formatMenu->addActions(bitDepthGroup->actions());
    formatMenu->addSeparator();
    formatMenu->addActions(channelGroup->actions());
    disabledWidgetsOnStart.append(formatMenu);

    muteAction = soundMenu->addAction(tr("Mute"), this, &MainWindow::slotMuteSound);
    muteAction->setCheckable(true);
    disableAction = soundMenu->addAction(tr("Disable"), this, &MainWindow::slotDisableSound);
    disableAction->setCheckable(true);

    /* Runtime Menu */
    QMenu *runtimeMenu = menuBar()->addMenu(tr("Runtime"));
    runtimeMenu->setToolTipsVisible(true);

    QMenu *localeMenu = runtimeMenu->addMenu(tr("Force locale"));
    localeMenu->addActions(localeGroup->actions());

    QMenu *timeMenu = runtimeMenu->addMenu(tr("Time tracking"));
    disabledWidgetsOnStart.append(timeMenu);
    timeMenu->addActions(timeMainGroup->actions());
    timeMenu->setToolTip("Enable a hack to prevent softlocks when the game waits for time to advance. Only check the necessary one(s)");
    timeMenu->installEventFilter(this);

    busyloopAction = runtimeMenu->addAction(tr("Busy loop detection"), this, &MainWindow::slotBusyLoop);
    busyloopAction->setCheckable(true);
    disabledActionsOnStart.append(busyloopAction);

    QMenu *waitMenu = runtimeMenu->addMenu(tr("Wait timeout"));
    disabledWidgetsOnStart.append(waitMenu);
    waitMenu->addActions(waitGroup->actions());

    QMenu *savestateMenu = runtimeMenu->addMenu(tr("Savestates"));
    // savestateMenu->setToolTipsVisible(true);
    savestateMenu->addActions(savestateGroup->actions());

    preventSavefileAction = runtimeMenu->addAction(tr("Prevent writing to disk"), this, &MainWindow::slotPreventSavefile);
    preventSavefileAction->setCheckable(true);
    preventSavefileAction->setToolTip("Prevent the game from writing files on disk, but write in memory instead. May cause issues in some games");
    recycleThreadsAction = runtimeMenu->addAction(tr("Recycle threads"), this, &MainWindow::slotRecycleThreads);
    recycleThreadsAction->setToolTip("Recycle threads when they finish, to make savestates more useable. Can crash on some games");
    recycleThreadsAction->setCheckable(true);
    disabledActionsOnStart.append(recycleThreadsAction);
    steamAction = runtimeMenu->addAction(tr("Virtual Steam client"), this, &MainWindow::slotSteam);
    steamAction->setToolTip("Implement a dummy Steam client, to be able to launch some Steam games");
    steamAction->setCheckable(true);
    disabledActionsOnStart.append(steamAction);

    QMenu *asyncMenu = runtimeMenu->addMenu(tr("Asynchronous events"));
    asyncMenu->setToolTip("Only useful if the game pulls events asynchronously. We wait until all events are processed at the beginning of each frame");
    disabledWidgetsOnStart.append(asyncMenu);
    asyncMenu->addActions(asyncGroup->actions());

    runtimeMenu->addAction(tr("Game-specific settings..."), gameSpecificWindow, &GameSpecificWindow::exec);

    QMenu *debugMenu = runtimeMenu->addMenu(tr("Debug"));

    debugMenu->addActions(debugStateGroup->actions());

    QMenu *timeSecMenu = debugMenu->addMenu(tr("Time tracking all threads"));
    timeSecMenu->addActions(timeSecGroup->actions());
    timeSecMenu->installEventFilter(this);

    debugMenu->addSeparator();

    sigintAction = debugMenu->addAction(tr("Raise SIGINT upon game launch (if debugging)"));
    sigintAction->setCheckable(true);

    debugMenu->addSeparator();

    debugMenu->addActions(loggingOutputGroup->actions());
    disabledActionsOnStart.append(loggingOutputGroup->actions());

    debugMenu->addSeparator();

    QMenu *debugPrintMenu = debugMenu->addMenu(tr("Print Categories"));
    debugPrintMenu->addActions(loggingPrintGroup->actions());
    debugPrintMenu->installEventFilter(this);

    QMenu *debugExcludeMenu = debugMenu->addMenu(tr("Exclude Categories"));
    debugExcludeMenu->addActions(loggingExcludeGroup->actions());
    debugExcludeMenu->installEventFilter(this);

    debugMenu->addAction(tr("Time Trace..."), timeTraceWindow, &TimeTraceWindow::show);

    /* Tools Menu */
    QMenu *toolsMenu = menuBar()->addMenu(tr("Tools"));
    configEncodeAction = toolsMenu->addAction(tr("Configure encode..."), encodeWindow, &EncodeWindow::exec);
    toggleEncodeAction = toolsMenu->addAction(tr("Start encode"), this, &MainWindow::slotToggleEncode);

    toolsMenu->addSeparator();

    QMenu *slowdownMenu = toolsMenu->addMenu(tr("Slow Motion"));
    slowdownMenu->addActions(slowdownGroup->actions());

    toolsMenu->addSeparator();

    QMenu *fastforwardMenu = toolsMenu->addMenu(tr("Fast-forward mode"));
    fastforwardMenu->addActions(fastforwardGroup->actions());

    toolsMenu->addSeparator();

    toolsMenu->addAction(tr("Game information..."), gameInfoWindow, &GameInfoWindow::exec);

    toolsMenu->addSeparator();

    toolsMenu->addAction(tr("Ram Search..."), ramSearchWindow, &RamSearchWindow::show);
    toolsMenu->addAction(tr("Ram Watch..."), ramWatchWindow, &RamWatchWindow::show);

    toolsMenu->addSeparator();

    QMenu *luaMenu = toolsMenu->addMenu(tr("Lua"));

    luaMenu->addAction(tr("Execute Lua script..."), this, &MainWindow::slotLuaExecute);
    luaMenu->addAction(tr("Reset Lua VM"), this, &MainWindow::slotLuaReset);

    /* Input Menu */
    QMenu *inputMenu = menuBar()->addMenu(tr("Input"));
    inputMenu->setToolTipsVisible(true);

    inputMenu->addAction(tr("Configure mapping..."), inputWindow, &InputWindow::exec);

    mouseAction = inputMenu->addAction(tr("Mouse support"));
    mouseAction->setCheckable(true);
    disabledActionsOnStart.append(mouseAction);
    mouseModeAction = inputMenu->addAction(tr("Mouse relative mode"), this, &MainWindow::slotMouseMode);
    mouseModeAction->setCheckable(true);
    mouseWarpAction = inputMenu->addAction(tr("Warp mouse to center each frame"), this, &MainWindow::slotMouseWarp);
    mouseWarpAction->setCheckable(true);
    mouseGameWarpAction = inputMenu->addAction(tr("Prevent games from warping the mouse cursor"), this, &MainWindow::slotMouseGameWarp);
    mouseGameWarpAction->setCheckable(true);

    QMenu *joystickMenu = inputMenu->addMenu(tr("Joystick support"));
    joystickMenu->addActions(joystickGroup->actions());
    disabledWidgetsOnStart.append(joystickMenu);

    inputMenu->addAction(tr("Joystick inputs..."), controllerTabWindow, &ControllerTabWindow::show);
}

void MainWindow::updateStatus()
{
    /* Update game status (active/inactive) */

    switch (context->status) {

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

            fpsNumField->setEnabled(true);
            fpsDenField->setEnabled(true);
            initialTimeSec->setValue(context->config.sc.initial_time_sec);
            initialTimeNsec->setValue(context->config.sc.initial_time_nsec);

            launchGdbButton->setEnabled(true);

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

            if (!context->config.sc.variable_framerate) {
                fpsNumField->setEnabled(false);
                fpsDenField->setEnabled(false);
            }

            launchGdbButton->setEnabled(false);

            movieBox->setCheckable(false);
            if (context->config.sc.recording == SharedConfig::NO_RECORDING) {
                movieBox->setEnabled(false);
            }

            break;

        case Context::ACTIVE:
            stopButton->setEnabled(true);

            if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
                saveMovieAction->setEnabled(true);
                exportMovieAction->setEnabled(true);
            }

            break;
        case Context::QUITTING:
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
    initialTimeSec->setValue(context->current_time_sec);
    initialTimeNsec->setValue(context->current_time_nsec);

    /* Update movie time */
    double sec = context->current_time_sec - context->config.sc.initial_time_sec + ((double)(context->current_time_nsec - context->config.sc.initial_time_nsec))/1000000000;
    int imin = (int)(sec/60);
    double dsec = sec - 60*imin;
    currentLength->setText(QString("Current Time: %1m %2s").arg(imin).arg(dsec, 0, 'f', 2));

    /* Format movie length */
    if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
        if (context->movie_time_sec != -1) {
            double msec = context->movie_time_sec + ((double)context->movie_time_nsec)/1000000000.0;
            int immin = (int)(msec/60);
            double dmsec = msec - 60*immin;
            movieLength->setText(QString("Movie length: %1m %2s").arg(immin).arg(dmsec, 0, 'f', 2));
        }
        else {
            movieLength->setText("Movie length: -");
        }
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
    inputEditorWindow->inputEditorView->update();
}

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

void MainWindow::updateMovieParams()
{
    if ((context->config.sc.recording != SharedConfig::NO_RECORDING) &&
        (gameLoop->movie.loadMovie() == 0)) {
        authorField->setReadOnly(true);

        /* Format movie length */
        if (context->movie_time_sec != -1) {
            double msec = context->movie_time_sec + ((double)context->movie_time_nsec)/1000000000.0;
            int immin = (int)(msec/60);
            double dmsec = msec - 60*immin;
            movieLength->setText(QString("Movie length: %1m %2s").arg(immin).arg(dmsec, 0, 'f', 2));
        }
        else {
            movieLength->setText("Movie length: -");
        }

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
    }
    else {
        context->config.sc.movie_framecount = 0;
        context->rerecord_count = 0;
        context->authors = "";
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
    }
    inputEditorWindow->resetInputs();
    movieFrameCount->setValue(context->config.sc.movie_framecount);
    rerecordCount->setValue(context->rerecord_count);
    authorField->setText(context->authors.c_str());
    mouseAction->setChecked(context->config.sc.mouse_support);
    setRadioFromList(joystickGroup, context->config.sc.nb_controllers);
    fpsNumField->setValue(context->config.sc.framerate_num);
    fpsDenField->setValue(context->config.sc.framerate_den);
    initialTimeSec->setValue(context->config.sc.initial_time_sec);
    initialTimeNsec->setValue(context->config.sc.initial_time_nsec);
    autoRestartAction->setChecked(context->config.auto_restart);
    variableFramerateAction->setChecked(context->config.sc.variable_framerate);
    for (auto& action : timeMainGroup->actions()) {
        action->setChecked(context->config.sc.main_gettimes_threshold[action->data().toInt()] != -1);
    }
    for (auto& action : timeSecGroup->actions()) {
        action->setChecked(context->config.sc.sec_gettimes_threshold[action->data().toInt()] != -1);
    }
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

    setRadioFromList(frequencyGroup, context->config.sc.audio_frequency);
    setRadioFromList(bitDepthGroup, context->config.sc.audio_bitdepth);
    setRadioFromList(channelGroup, context->config.sc.audio_channels);

    muteAction->setChecked(context->config.sc.audio_mute);
    disableAction->setChecked(context->config.sc.audio_disabled);

    setRadioFromList(debugStateGroup, context->config.sc.debug_state);
    setRadioFromList(loggingOutputGroup, context->config.sc.logging_status);

    setCheckboxesFromMask(loggingPrintGroup, context->config.sc.includeFlags);
    setCheckboxesFromMask(loggingExcludeGroup, context->config.sc.excludeFlags);

    setRadioFromList(slowdownGroup, context->config.sc.speed_divisor);

    mouseModeAction->setChecked(context->config.sc.mouse_mode_relative);
    mouseWarpAction->setChecked(context->config.mouse_warp);
    mouseGameWarpAction->setChecked(context->config.sc.mouse_prevent_warp);

    int screenResValue = (context->config.sc.screen_width << 16) | context->config.sc.screen_height;
    setRadioFromList(screenResGroup, screenResValue);

#ifdef LIBTAS_ENABLE_HUD
    setCheckboxesFromMask(osdGroup, context->config.sc.osd);
    osdEncodeAction->setChecked(context->config.sc.osd_encode);
#endif

    setRadioFromList(localeGroup, context->config.sc.locale);

    busyloopAction->setChecked(context->config.sc.busyloop_detection);

    setRadioFromList(waitGroup, context->config.sc.wait_timeout);

    renderSoftAction->setChecked(context->config.sc.opengl_soft);
    renderPerfAction->setChecked(context->config.sc.opengl_performance);
    preventSavefileAction->setChecked(context->config.sc.prevent_savefiles);
    recycleThreadsAction->setChecked(context->config.sc.recycle_threads);
    steamAction->setChecked(context->config.sc.virtual_steam);
    setCheckboxesFromMask(asyncGroup, context->config.sc.async_events);

    setCheckboxesFromMask(savestateGroup, context->config.sc.savestate_settings);

    setCheckboxesFromMask(fastforwardGroup, context->config.sc.fastforward_mode);

    setRadioFromList(movieEndGroup, context->config.on_movie_end);

    switch (context->config.debugger) {
    case Config::DEBUGGER_GDB:
        launchGdbButton->setDefaultAction(launchGdbAction);
        break;
    case Config::DEBUGGER_LLDB:
        launchGdbButton->setDefaultAction(launchLldbAction);
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

void MainWindow::slotLaunch(bool attach_gdb)
{

    /* Do we attach gdb ? */
    context->attach_gdb = attach_gdb;

    if (context->status != Context::INACTIVE)
        return;

    /* Perform all checks */
    if (!ErrorChecking::allChecks(context))
        return;

    context->authors = authorField->text().toStdString();

    /* Set a few parameters */
    context->config.sc.framerate_num = fpsNumField->value();
    context->config.sc.framerate_den = fpsDenField->value();
    context->config.sc.initial_time_sec = initialTimeSec->value();
    context->config.sc.initial_time_nsec = initialTimeNsec->value();

    setListFromRadio(frequencyGroup, context->config.sc.audio_frequency);
    setListFromRadio(bitDepthGroup, context->config.sc.audio_bitdepth);
    setListFromRadio(channelGroup, context->config.sc.audio_channels);

    setListFromRadio(loggingOutputGroup, context->config.sc.logging_status);

    context->config.sc.sigint_upon_launch = context->attach_gdb && sigintAction->isChecked();

    context->config.sc.mouse_support = mouseAction->isChecked();
    setListFromRadio(joystickGroup, context->config.sc.nb_controllers);

    setListFromRadio(localeGroup, context->config.sc.locale);

    for (const auto& action : timeMainGroup->actions()) {
        int index = action->data().toInt();
        if (action->isChecked()) {
            context->config.sc.main_gettimes_threshold[index] = 100;
        }
        else {
            context->config.sc.main_gettimes_threshold[index] = -1;
        }
    }
    for (const auto& action : timeSecGroup->actions()) {
        int index = action->data().toInt();
        if (action->isChecked()) {
            context->config.sc.sec_gettimes_threshold[index] = 100;
        }
        else {
            context->config.sc.sec_gettimes_threshold[index] = -1;
        }
    }

    setListFromRadio(waitGroup, context->config.sc.wait_timeout);
    setMaskFromCheckboxes(asyncGroup, context->config.sc.async_events);
    setMaskFromCheckboxes(savestateGroup, context->config.sc.savestate_settings);

    context->config.gameargs = cmdOptions->currentText().toStdString();

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
    updateStatus();
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
            kill(context->fork_pid, SIGKILL);
        }
        return;
    }

    if (context->status == Context::ACTIVE) {
        context->status = Context::QUITTING;
        updateStatus();
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
    int gameArch = extractBinaryType(context->gamepath);
    if (gameArch == BT_PE32 || gameArch == BT_PE32P)
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
    gameSpecificWindow->update_config();
    inputWindow->update();
    osdWindow->update_config();
    autoSaveWindow->update_config();
    inputEditorWindow->update_config();
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

void MainWindow::slotEnforceMovieSettings(bool checked)
{
    gameLoop->movie.header->skipLoadSettings = checked;
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

#define BOOLSLOT(slot, parameter) void MainWindow::slot(bool checked)\
{\
    parameter = checked;\
    context->config.sc_modified = true;\
}\

#define CHECKBOXSLOT(slot, group, parameter) void MainWindow::slot()\
{\
    setMaskFromCheckboxes(group, parameter);\
    context->config.sc_modified = true;\
}\

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

void MainWindow::slotMuteSound(bool checked)
{
    context->config.sc.audio_mute = checked;
    context->config.sc_modified = true;
    updateStatusBar();
}

BOOLSLOT(slotDisableSound, context->config.sc.audio_disabled)

void MainWindow::slotRenderSoft(bool checked)
{
    context->config.sc.opengl_soft = checked;
    updateStatusBar();
}

BOOLSLOT(slotRenderPerf, context->config.sc.opengl_performance)

CHECKBOXSLOT(slotSavestate, savestateGroup, context->config.sc.savestate_settings)
CHECKBOXSLOT(slotDebugState, debugStateGroup, context->config.sc.debug_state)
CHECKBOXSLOT(slotLoggingPrint, loggingPrintGroup, context->config.sc.includeFlags)
CHECKBOXSLOT(slotLoggingExclude, loggingExcludeGroup, context->config.sc.excludeFlags)
CHECKBOXSLOT(slotFastforwardMode, fastforwardGroup, context->config.sc.fastforward_mode)

void MainWindow::slotSlowdown()
{
    setListFromRadio(slowdownGroup, context->config.sc.speed_divisor);
    context->config.sc_modified = true;
}

void MainWindow::slotScreenRes()
{
    int value = 0;
    setListFromRadio(screenResGroup, value);

    context->config.sc.screen_width = (value >> 16);
    context->config.sc.screen_height = (value & 0xffff);
}

#ifdef LIBTAS_ENABLE_HUD

CHECKBOXSLOT(slotOsd, osdGroup, context->config.sc.osd)
BOOLSLOT(slotOsdEncode, context->config.sc.osd_encode)

#endif

BOOLSLOT(slotBusyLoop, context->config.sc.busyloop_detection)
BOOLSLOT(slotPreventSavefile, context->config.sc.prevent_savefiles)
BOOLSLOT(slotRecycleThreads, context->config.sc.recycle_threads)
BOOLSLOT(slotSteam, context->config.sc.virtual_steam)
BOOLSLOT(slotAsyncEvents, context->config.sc.async_events)

void MainWindow::slotMovieEnd()
{
    setListFromRadio(movieEndGroup, context->config.on_movie_end);
}

void MainWindow::slotVariableFramerate(bool checked)
{
    context->config.sc.variable_framerate = checked;
    encodeWindow->update_config();
}

BOOLSLOT(slotAutoRestart, context->config.auto_restart)
BOOLSLOT(slotMouseMode, context->config.sc.mouse_mode_relative)
BOOLSLOT(slotMouseWarp, context->config.mouse_warp)
BOOLSLOT(slotMouseGameWarp, context->config.sc.mouse_prevent_warp)

void MainWindow::slotLuaExecute()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Lua script"), context->gamepath.c_str());
    if (filename.isNull())
        return;

    Lua::Main::run(context, filename.toStdString());
}

void MainWindow::slotLuaReset()
{
    Lua::Main::reset(context);
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

    /* Show alert window */
    QMessageBox::warning(this, "Warning", alert_msg);
}

void MainWindow::updateFramerate()
{
    fpsNumField->setValue(context->config.sc.framerate_num);
    fpsDenField->setValue(context->config.sc.framerate_den);
}
