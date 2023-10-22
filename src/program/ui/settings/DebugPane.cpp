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

// #include <QtWidgets/QLabel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>

#include "DebugPane.h"
#include "../../Context.h"
#include "tooltip/ToolTipCheckBox.h"

DebugPane::DebugPane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void DebugPane::initLayout()
{
    generalBox = new QGroupBox(tr("General"));
    QVBoxLayout* generalLayout = new QVBoxLayout;
    generalBox->setLayout(generalLayout);

    debugUncontrolledBox = new ToolTipCheckBox(tr("Uncontrolled time"));
    debugEventsBox = new ToolTipCheckBox(tr("Native events"));
    debugMainBox = new ToolTipCheckBox(tr("Keep main first thread"));
    debugIOBox = new ToolTipCheckBox(tr("Native file IO"));
    debugInetBox = new ToolTipCheckBox(tr("Native internet"));
    debugSigIntBox = new QCheckBox(tr("Raise SIGINT upon game launch (if debugging)"));

    generalLayout->addWidget(debugUncontrolledBox);
    generalLayout->addWidget(debugEventsBox);
    generalLayout->addWidget(debugMainBox);
    generalLayout->addWidget(debugIOBox);
    generalLayout->addWidget(debugInetBox);
    generalLayout->addWidget(debugSigIntBox);

    QGroupBox* logBox = new QGroupBox(tr("Logging"));
    QVBoxLayout* logLayout = new QVBoxLayout;
    logBox->setLayout(logLayout);

    logToChoice = new QComboBox();

    logToChoice->addItem(tr("Disabled logging"), SharedConfig::NO_LOGGING);
    logToChoice->addItem(tr("Log to console"), SharedConfig::LOGGING_TO_CONSOLE);
    logToChoice->addItem(tr("Log to file"), SharedConfig::LOGGING_TO_FILE);

    QGroupBox* logPrintBox = new QGroupBox(tr("Print"));
    QGridLayout* logPrintLayout = new QGridLayout;
    logPrintBox->setLayout(logPrintLayout);

    QGroupBox* logMainPrintBox = new QGroupBox();
    QHBoxLayout* logMainPrintLayout = new QHBoxLayout;
    logMainPrintBox->setLayout(logMainPrintLayout);

    logPrintMainBox = new QCheckBox(tr("Main Thread"));
    logPrintFrequentBox = new QCheckBox(tr("Frequent"));
    logPrintErrorBox = new QCheckBox(tr("Error"));
    logPrintWarningBox = new QCheckBox(tr("Warning"));
    logPrintInfoBox = new QCheckBox(tr("Info"));
    logPrintTODOBox = new QCheckBox(tr("TODO"));
    logPrintAVBox = new QCheckBox(tr("AV Dumping"));
    logPrintCheckpointBox = new QCheckBox(tr("Checkpoint"));
    logPrintEventsBox = new QCheckBox(tr("Events"));
    logPrintIOBox = new QCheckBox(tr("File IO"));
    logPrintHookBox = new QCheckBox(tr("Hook"));
    logPrintJoystickBox = new QCheckBox(tr("Joystick"));
    logPrintKeyboardBox = new QCheckBox(tr("Keyboard"));
    logPrintLocaleBox = new QCheckBox(tr("Locale"));
    logPrintMouseBox = new QCheckBox(tr("Mouse"));
    logPrintGLBox = new QCheckBox(tr("OpenGL/Vulkan"));
    logPrintRandomBox = new QCheckBox(tr("Random"));
    logPrintSDLBox = new QCheckBox(tr("SDL"));
    logPrintSignalsBox = new QCheckBox(tr("Signals"));
    logPrintSleepBox = new QCheckBox(tr("Sleep"));
    logPrintSocketBox = new QCheckBox(tr("Socket"));
    logPrintSoundBox = new QCheckBox(tr("Sound"));
    logPrintSteamBox = new QCheckBox(tr("Steam"));
    logPrintSystemBox = new QCheckBox(tr("System"));
    logPrintTimeGetBox = new QCheckBox(tr("Time Get"));
    logPrintTimeSetBox = new QCheckBox(tr("Time Set"));
    logPrintTimersBox = new QCheckBox(tr("Timers"));
    logPrintThreadsBox = new QCheckBox(tr("Threads"));
    logPrintWaitBox = new QCheckBox(tr("Wait"));
    logPrintWindowsBox = new QCheckBox(tr("Windows"));
    logPrintWineBox = new QCheckBox(tr("Wine"));

    logMainPrintLayout->addWidget(logPrintMainBox);
    logMainPrintLayout->addWidget(logPrintFrequentBox);
    logMainPrintLayout->addWidget(logPrintErrorBox);
    logMainPrintLayout->addWidget(logPrintWarningBox);
    logMainPrintLayout->addWidget(logPrintInfoBox);
    logMainPrintLayout->addWidget(logPrintTODOBox);

    logPrintLayout->addWidget(logMainPrintBox, 0, 0, 1, 5);
    logPrintLayout->addWidget(logPrintAVBox, 1, 0);
    logPrintLayout->addWidget(logPrintCheckpointBox, 2, 0);
    logPrintLayout->addWidget(logPrintEventsBox, 3, 0);
    logPrintLayout->addWidget(logPrintIOBox, 4, 0);
    logPrintLayout->addWidget(logPrintHookBox, 5, 0);
    logPrintLayout->addWidget(logPrintJoystickBox, 1, 1);
    logPrintLayout->addWidget(logPrintKeyboardBox, 2, 1);
    logPrintLayout->addWidget(logPrintLocaleBox, 3, 1);
    logPrintLayout->addWidget(logPrintMouseBox, 4, 1);
    logPrintLayout->addWidget(logPrintGLBox, 5, 1);
    logPrintLayout->addWidget(logPrintRandomBox, 1, 2);
    logPrintLayout->addWidget(logPrintSDLBox, 2, 2);
    logPrintLayout->addWidget(logPrintSignalsBox, 3, 2);
    logPrintLayout->addWidget(logPrintSleepBox, 4, 2);
    logPrintLayout->addWidget(logPrintSocketBox, 5, 2);
    logPrintLayout->addWidget(logPrintSoundBox, 1, 3);
    logPrintLayout->addWidget(logPrintSteamBox, 2, 3);
    logPrintLayout->addWidget(logPrintSystemBox, 3, 3);
    logPrintLayout->addWidget(logPrintTimeGetBox, 4, 3);
    logPrintLayout->addWidget(logPrintTimeSetBox, 5, 3);
    logPrintLayout->addWidget(logPrintTimersBox, 1, 4);
    logPrintLayout->addWidget(logPrintThreadsBox, 2, 4);
    logPrintLayout->addWidget(logPrintWaitBox, 3, 4);
    logPrintLayout->addWidget(logPrintWindowsBox, 4, 4);
    logPrintLayout->addWidget(logPrintWineBox, 5, 4);

    QGroupBox* logExcludeBox = new QGroupBox(tr("Exclude"));
    QGridLayout* logExcludeLayout = new QGridLayout;
    logExcludeBox->setLayout(logExcludeLayout);

    QGroupBox* logMainExcludeBox = new QGroupBox();
    QHBoxLayout* logMainExcludeLayout = new QHBoxLayout;
    logMainExcludeBox->setLayout(logMainExcludeLayout);

    logExcludeMainBox = new QCheckBox(tr("Main Thread"));
    logExcludeFrequentBox = new QCheckBox(tr("Frequent"));
    logExcludeErrorBox = new QCheckBox(tr("Error"));
    logExcludeWarningBox = new QCheckBox(tr("Warning"));
    logExcludeInfoBox = new QCheckBox(tr("Info"));
    logExcludeTODOBox = new QCheckBox(tr("TODO"));
    logExcludeAVBox = new QCheckBox(tr("AV Dumping"));
    logExcludeCheckpointBox = new QCheckBox(tr("Checkpoint"));
    logExcludeEventsBox = new QCheckBox(tr("Events"));
    logExcludeIOBox = new QCheckBox(tr("File IO"));
    logExcludeHookBox = new QCheckBox(tr("Hook"));
    logExcludeJoystickBox = new QCheckBox(tr("Joystick"));
    logExcludeKeyboardBox = new QCheckBox(tr("Keyboard"));
    logExcludeLocaleBox = new QCheckBox(tr("Locale"));
    logExcludeMouseBox = new QCheckBox(tr("Mouse"));
    logExcludeGLBox = new QCheckBox(tr("OpenGL/Vulkan"));
    logExcludeRandomBox = new QCheckBox(tr("Random"));
    logExcludeSDLBox = new QCheckBox(tr("SDL"));
    logExcludeSignalsBox = new QCheckBox(tr("Signals"));
    logExcludeSleepBox = new QCheckBox(tr("Sleep"));
    logExcludeSocketBox = new QCheckBox(tr("Socket"));
    logExcludeSoundBox = new QCheckBox(tr("Sound"));
    logExcludeSteamBox = new QCheckBox(tr("Steam"));
    logExcludeSystemBox = new QCheckBox(tr("System"));
    logExcludeTimeGetBox = new QCheckBox(tr("Time Get"));
    logExcludeTimeSetBox = new QCheckBox(tr("Time Set"));
    logExcludeTimersBox = new QCheckBox(tr("Timers"));
    logExcludeThreadsBox = new QCheckBox(tr("Threads"));
    logExcludeWaitBox = new QCheckBox(tr("Wait"));
    logExcludeWindowsBox = new QCheckBox(tr("Windows"));
    logExcludeWineBox = new QCheckBox(tr("Wine"));
    
    logMainExcludeLayout->addWidget(logExcludeMainBox);
    logMainExcludeLayout->addWidget(logExcludeFrequentBox);
    logMainExcludeLayout->addWidget(logExcludeErrorBox);
    logMainExcludeLayout->addWidget(logExcludeWarningBox);
    logMainExcludeLayout->addWidget(logExcludeInfoBox);
    logMainExcludeLayout->addWidget(logExcludeTODOBox);

    logExcludeLayout->addWidget(logMainExcludeBox, 0, 0, 1, 5);
    logExcludeLayout->addWidget(logExcludeAVBox, 1, 0);
    logExcludeLayout->addWidget(logExcludeCheckpointBox, 2, 0);
    logExcludeLayout->addWidget(logExcludeEventsBox, 3, 0);
    logExcludeLayout->addWidget(logExcludeIOBox, 4, 0);
    logExcludeLayout->addWidget(logExcludeHookBox, 5, 0);
    logExcludeLayout->addWidget(logExcludeJoystickBox, 1, 1);
    logExcludeLayout->addWidget(logExcludeKeyboardBox, 2, 1);
    logExcludeLayout->addWidget(logExcludeLocaleBox, 3, 1);
    logExcludeLayout->addWidget(logExcludeMouseBox, 4, 1);
    logExcludeLayout->addWidget(logExcludeGLBox, 5, 1);
    logExcludeLayout->addWidget(logExcludeRandomBox, 1, 2);
    logExcludeLayout->addWidget(logExcludeSDLBox, 2, 2);
    logExcludeLayout->addWidget(logExcludeSignalsBox, 3, 2);
    logExcludeLayout->addWidget(logExcludeSleepBox, 4, 2);
    logExcludeLayout->addWidget(logExcludeSocketBox, 5, 2);
    logExcludeLayout->addWidget(logExcludeSoundBox, 1, 3);
    logExcludeLayout->addWidget(logExcludeSteamBox, 2, 3);
    logExcludeLayout->addWidget(logExcludeSystemBox, 3, 3);
    logExcludeLayout->addWidget(logExcludeTimeGetBox, 4, 3);
    logExcludeLayout->addWidget(logExcludeTimeSetBox, 5, 3);
    logExcludeLayout->addWidget(logExcludeTimersBox, 1, 4);
    logExcludeLayout->addWidget(logExcludeThreadsBox, 2, 4);
    logExcludeLayout->addWidget(logExcludeWaitBox, 3, 4);
    logExcludeLayout->addWidget(logExcludeWindowsBox, 4, 4);
    logExcludeLayout->addWidget(logExcludeWineBox, 5, 4);
    
    logLayout->addWidget(logToChoice);
    logLayout->addWidget(logPrintBox);
    logLayout->addWidget(logExcludeBox);

    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalBox);
    mainLayout->addWidget(logBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);    
}

void DebugPane::initSignals()
{
    connect(debugUncontrolledBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(debugEventsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(debugMainBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(debugIOBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(debugInetBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(debugSigIntBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logToChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &DebugPane::saveConfig);

    connect(logPrintMainBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintFrequentBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintErrorBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintWarningBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintInfoBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintTODOBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintAVBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintCheckpointBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintEventsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintIOBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintHookBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintJoystickBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintKeyboardBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintLocaleBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintMouseBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintGLBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintRandomBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintSDLBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintSignalsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintSleepBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintSocketBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintSoundBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintSteamBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintSystemBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintTimeGetBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintTimeSetBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintTimersBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintThreadsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintWaitBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintWindowsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintWineBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);

    connect(logExcludeMainBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeFrequentBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeErrorBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeWarningBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeInfoBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeTODOBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeAVBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeCheckpointBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeEventsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeIOBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeHookBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeJoystickBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeKeyboardBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeLocaleBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeMouseBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeGLBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeRandomBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeSDLBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeSignalsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeSleepBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeSocketBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeSoundBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeSteamBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeSystemBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeTimeGetBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeTimeSetBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeTimersBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeThreadsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeWaitBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeWindowsBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logExcludeWineBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
}

void DebugPane::initToolTips()
{
    debugUncontrolledBox->setDescription("Let the game access to the real system time, only for debugging purpose");

    debugEventsBox->setDescription("Let the game access to the real system events, only for debugging purpose");

    debugMainBox->setDescription("Keep the first thread as the main thread. "
    "The main thread has a special role in libTAS: only this thread can advance time "
    "when it sleeps, or when it queries time with time-tracking enabled. "
    "Main thread is always defined as the thread that draws on the game window.<br><br>"
    "Some games handle time in the first thread, and creates another thread for rendering, "
    "so in this specific case, enabling this option may fix game softlocks.");

    debugIOBox->setDescription("Let the game access to the real filesystem, only for debugging purpose. "
    "This is different from unchecking 'Prevent writing to disk', as it will allow "
    "games to access to device files, such as reading joystick events, or the hardware random generator.");

    debugInetBox->setDescription("Let the game access the internet, only for debugging purpose.");
}

void DebugPane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void DebugPane::loadConfig()
{
    debugUncontrolledBox->setChecked(context->config.sc.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME);
    debugEventsBox->setChecked(context->config.sc.debug_state & SharedConfig::DEBUG_NATIVE_EVENTS);
    debugMainBox->setChecked(context->config.sc.debug_state & SharedConfig::DEBUG_MAIN_FIRST_THREAD);
    debugIOBox->setChecked(context->config.sc.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO);
    debugInetBox->setChecked(context->config.sc.debug_state & SharedConfig::DEBUG_NATIVE_INET);
    debugSigIntBox->setChecked(context->config.sc.sigint_upon_launch);
    
    int index = logToChoice->findData(context->config.sc.logging_status);
    if (index >= 0)
        logToChoice->setCurrentIndex(index);
        
    logPrintMainBox->setChecked(context->config.sc.includeFlags & LCF_MAINTHREAD);
    logPrintFrequentBox->setChecked(context->config.sc.includeFlags & LCF_FREQUENT);
    logPrintErrorBox->setChecked(context->config.sc.includeFlags & LCF_ERROR);
    logPrintWarningBox->setChecked(context->config.sc.includeFlags & LCF_WARNING);
    logPrintInfoBox->setChecked(context->config.sc.includeFlags & LCF_INFO);
    logPrintTODOBox->setChecked(context->config.sc.includeFlags & LCF_TODO);
    logPrintAVBox->setChecked(context->config.sc.includeFlags & LCF_DUMP);
    logPrintCheckpointBox->setChecked(context->config.sc.includeFlags & LCF_CHECKPOINT);
    logPrintEventsBox->setChecked(context->config.sc.includeFlags & LCF_EVENTS);
    logPrintIOBox->setChecked(context->config.sc.includeFlags & LCF_FILEIO);
    logPrintHookBox->setChecked(context->config.sc.includeFlags & LCF_HOOK);
    logPrintJoystickBox->setChecked(context->config.sc.includeFlags & LCF_JOYSTICK);
    logPrintKeyboardBox->setChecked(context->config.sc.includeFlags & LCF_KEYBOARD);
    logPrintLocaleBox->setChecked(context->config.sc.includeFlags & LCF_LOCALE);
    logPrintMouseBox->setChecked(context->config.sc.includeFlags & LCF_MOUSE);
    logPrintGLBox->setChecked(context->config.sc.includeFlags & LCF_OGL);
    logPrintRandomBox->setChecked(context->config.sc.includeFlags & LCF_RANDOM);
    logPrintSDLBox->setChecked(context->config.sc.includeFlags & LCF_SDL);
    logPrintSignalsBox->setChecked(context->config.sc.includeFlags & LCF_SIGNAL);
    logPrintSleepBox->setChecked(context->config.sc.includeFlags & LCF_SLEEP);
    logPrintSocketBox->setChecked(context->config.sc.includeFlags & LCF_SOCKET);
    logPrintSoundBox->setChecked(context->config.sc.includeFlags & LCF_SOUND);
    logPrintSteamBox->setChecked(context->config.sc.includeFlags & LCF_STEAM);
    logPrintSystemBox->setChecked(context->config.sc.includeFlags & LCF_SYSTEM);
    logPrintTimeGetBox->setChecked(context->config.sc.includeFlags & LCF_TIMEGET);
    logPrintTimeSetBox->setChecked(context->config.sc.includeFlags & LCF_TIMESET);
    logPrintTimersBox->setChecked(context->config.sc.includeFlags & LCF_TIMERS);
    logPrintThreadsBox->setChecked(context->config.sc.includeFlags & LCF_THREAD);
    logPrintWaitBox->setChecked(context->config.sc.includeFlags & LCF_WAIT);
    logPrintWindowsBox->setChecked(context->config.sc.includeFlags & LCF_WINDOW);
    logPrintWineBox->setChecked(context->config.sc.includeFlags & LCF_WINE);
    
    logExcludeMainBox->setChecked(context->config.sc.excludeFlags & LCF_MAINTHREAD);
    logExcludeFrequentBox->setChecked(context->config.sc.excludeFlags & LCF_FREQUENT);
    logExcludeErrorBox->setChecked(context->config.sc.excludeFlags & LCF_ERROR);
    logExcludeWarningBox->setChecked(context->config.sc.excludeFlags & LCF_WARNING);
    logExcludeInfoBox->setChecked(context->config.sc.excludeFlags & LCF_INFO);
    logExcludeTODOBox->setChecked(context->config.sc.excludeFlags & LCF_TODO);
    logExcludeAVBox->setChecked(context->config.sc.excludeFlags & LCF_DUMP);
    logExcludeCheckpointBox->setChecked(context->config.sc.excludeFlags & LCF_CHECKPOINT);
    logExcludeEventsBox->setChecked(context->config.sc.excludeFlags & LCF_EVENTS);
    logExcludeIOBox->setChecked(context->config.sc.excludeFlags & LCF_FILEIO);
    logExcludeHookBox->setChecked(context->config.sc.excludeFlags & LCF_HOOK);
    logExcludeJoystickBox->setChecked(context->config.sc.excludeFlags & LCF_JOYSTICK);
    logExcludeKeyboardBox->setChecked(context->config.sc.excludeFlags & LCF_KEYBOARD);
    logExcludeLocaleBox->setChecked(context->config.sc.excludeFlags & LCF_LOCALE);
    logExcludeMouseBox->setChecked(context->config.sc.excludeFlags & LCF_MOUSE);
    logExcludeGLBox->setChecked(context->config.sc.excludeFlags & LCF_OGL);
    logExcludeRandomBox->setChecked(context->config.sc.excludeFlags & LCF_RANDOM);
    logExcludeSDLBox->setChecked(context->config.sc.excludeFlags & LCF_SDL);
    logExcludeSignalsBox->setChecked(context->config.sc.excludeFlags & LCF_SIGNAL);
    logExcludeSleepBox->setChecked(context->config.sc.excludeFlags & LCF_SLEEP);
    logExcludeSocketBox->setChecked(context->config.sc.excludeFlags & LCF_SOCKET);
    logExcludeSoundBox->setChecked(context->config.sc.excludeFlags & LCF_SOUND);
    logExcludeSteamBox->setChecked(context->config.sc.excludeFlags & LCF_STEAM);
    logExcludeSystemBox->setChecked(context->config.sc.excludeFlags & LCF_SYSTEM);
    logExcludeTimeGetBox->setChecked(context->config.sc.excludeFlags & LCF_TIMEGET);
    logExcludeTimeSetBox->setChecked(context->config.sc.excludeFlags & LCF_TIMESET);
    logExcludeTimersBox->setChecked(context->config.sc.excludeFlags & LCF_TIMERS);
    logExcludeThreadsBox->setChecked(context->config.sc.excludeFlags & LCF_THREAD);
    logExcludeWaitBox->setChecked(context->config.sc.excludeFlags & LCF_WAIT);
    logExcludeWindowsBox->setChecked(context->config.sc.excludeFlags & LCF_WINDOW);
    logExcludeWineBox->setChecked(context->config.sc.excludeFlags & LCF_WINE);
}

void DebugPane::saveConfig()
{
    context->config.sc.debug_state = 0;
    if (debugUncontrolledBox->isChecked())
        context->config.sc.debug_state |= SharedConfig::DEBUG_UNCONTROLLED_TIME;
    if (debugEventsBox->isChecked())
        context->config.sc.debug_state |= SharedConfig::DEBUG_NATIVE_EVENTS;
    if (debugMainBox->isChecked())
        context->config.sc.debug_state |= SharedConfig::DEBUG_MAIN_FIRST_THREAD;
    if (debugIOBox->isChecked())
        context->config.sc.debug_state |= SharedConfig::DEBUG_NATIVE_FILEIO;
    if (debugInetBox->isChecked())
        context->config.sc.debug_state |= SharedConfig::DEBUG_NATIVE_INET;
    context->config.sc.sigint_upon_launch = debugSigIntBox->isChecked();

    context->config.sc.logging_status = logToChoice->currentData().toInt();
    
    context->config.sc.includeFlags = 0;
    context->config.sc.includeFlags |= logPrintMainBox->isChecked() ? LCF_MAINTHREAD : 0;
    context->config.sc.includeFlags |= logPrintFrequentBox->isChecked() ? LCF_FREQUENT : 0;
    context->config.sc.includeFlags |= logPrintErrorBox->isChecked() ? LCF_ERROR : 0;
    context->config.sc.includeFlags |= logPrintWarningBox->isChecked() ? LCF_WARNING : 0;
    context->config.sc.includeFlags |= logPrintInfoBox->isChecked() ? LCF_INFO : 0;
    context->config.sc.includeFlags |= logPrintTODOBox->isChecked() ? LCF_TODO : 0;
    context->config.sc.includeFlags |= logPrintAVBox->isChecked() ? LCF_DUMP : 0;
    context->config.sc.includeFlags |= logPrintCheckpointBox->isChecked() ? LCF_CHECKPOINT : 0;
    context->config.sc.includeFlags |= logPrintEventsBox->isChecked() ? LCF_EVENTS : 0;
    context->config.sc.includeFlags |= logPrintIOBox->isChecked() ? LCF_FILEIO : 0;
    context->config.sc.includeFlags |= logPrintHookBox->isChecked() ? LCF_HOOK : 0;
    context->config.sc.includeFlags |= logPrintJoystickBox->isChecked() ? LCF_JOYSTICK : 0;
    context->config.sc.includeFlags |= logPrintKeyboardBox->isChecked() ? LCF_KEYBOARD : 0;
    context->config.sc.includeFlags |= logPrintLocaleBox->isChecked() ? LCF_LOCALE : 0;
    context->config.sc.includeFlags |= logPrintMouseBox->isChecked() ? LCF_MOUSE : 0;
    context->config.sc.includeFlags |= logPrintGLBox->isChecked() ? LCF_OGL : 0;
    context->config.sc.includeFlags |= logPrintRandomBox->isChecked() ? LCF_RANDOM : 0;
    context->config.sc.includeFlags |= logPrintSDLBox->isChecked() ? LCF_SDL : 0;
    context->config.sc.includeFlags |= logPrintSignalsBox->isChecked() ? LCF_SIGNAL : 0;
    context->config.sc.includeFlags |= logPrintSleepBox->isChecked() ? LCF_SLEEP : 0;
    context->config.sc.includeFlags |= logPrintSocketBox->isChecked() ? LCF_SOCKET : 0;
    context->config.sc.includeFlags |= logPrintSoundBox->isChecked() ? LCF_SOUND : 0;
    context->config.sc.includeFlags |= logPrintSteamBox->isChecked() ? LCF_STEAM : 0;
    context->config.sc.includeFlags |= logPrintSystemBox->isChecked() ? LCF_SYSTEM : 0;
    context->config.sc.includeFlags |= logPrintTimeGetBox->isChecked() ? LCF_TIMEGET : 0;
    context->config.sc.includeFlags |= logPrintTimeSetBox->isChecked() ? LCF_TIMESET : 0;
    context->config.sc.includeFlags |= logPrintTimersBox->isChecked() ? LCF_TIMERS : 0;
    context->config.sc.includeFlags |= logPrintThreadsBox->isChecked() ? LCF_THREAD : 0;
    context->config.sc.includeFlags |= logPrintWaitBox->isChecked() ? LCF_WAIT : 0;
    context->config.sc.includeFlags |= logPrintWindowsBox->isChecked() ? LCF_WINDOW : 0;
    context->config.sc.includeFlags |= logPrintWineBox->isChecked() ? LCF_WINE : 0;
    
    context->config.sc.excludeFlags = 0;
    context->config.sc.excludeFlags |= logExcludeMainBox->isChecked() ? LCF_MAINTHREAD : 0;
    context->config.sc.excludeFlags |= logExcludeFrequentBox->isChecked() ? LCF_FREQUENT : 0;
    context->config.sc.excludeFlags |= logExcludeErrorBox->isChecked() ? LCF_ERROR : 0;
    context->config.sc.excludeFlags |= logExcludeWarningBox->isChecked() ? LCF_WARNING : 0;
    context->config.sc.excludeFlags |= logExcludeInfoBox->isChecked() ? LCF_INFO : 0;
    context->config.sc.excludeFlags |= logExcludeTODOBox->isChecked() ? LCF_TODO : 0;
    context->config.sc.excludeFlags |= logExcludeAVBox->isChecked() ? LCF_DUMP : 0;
    context->config.sc.excludeFlags |= logExcludeCheckpointBox->isChecked() ? LCF_CHECKPOINT : 0;
    context->config.sc.excludeFlags |= logExcludeEventsBox->isChecked() ? LCF_EVENTS : 0;
    context->config.sc.excludeFlags |= logExcludeIOBox->isChecked() ? LCF_FILEIO : 0;
    context->config.sc.excludeFlags |= logExcludeHookBox->isChecked() ? LCF_HOOK : 0;
    context->config.sc.excludeFlags |= logExcludeJoystickBox->isChecked() ? LCF_JOYSTICK : 0;
    context->config.sc.excludeFlags |= logExcludeKeyboardBox->isChecked() ? LCF_KEYBOARD : 0;
    context->config.sc.excludeFlags |= logExcludeLocaleBox->isChecked() ? LCF_LOCALE : 0;
    context->config.sc.excludeFlags |= logExcludeMouseBox->isChecked() ? LCF_MOUSE : 0;
    context->config.sc.excludeFlags |= logExcludeGLBox->isChecked() ? LCF_OGL : 0;
    context->config.sc.excludeFlags |= logExcludeRandomBox->isChecked() ? LCF_RANDOM : 0;
    context->config.sc.excludeFlags |= logExcludeSDLBox->isChecked() ? LCF_SDL : 0;
    context->config.sc.excludeFlags |= logExcludeSignalsBox->isChecked() ? LCF_SIGNAL : 0;
    context->config.sc.excludeFlags |= logExcludeSleepBox->isChecked() ? LCF_SLEEP : 0;
    context->config.sc.excludeFlags |= logExcludeSocketBox->isChecked() ? LCF_SOCKET : 0;
    context->config.sc.excludeFlags |= logExcludeSoundBox->isChecked() ? LCF_SOUND : 0;
    context->config.sc.excludeFlags |= logExcludeSteamBox->isChecked() ? LCF_STEAM : 0;
    context->config.sc.excludeFlags |= logExcludeSystemBox->isChecked() ? LCF_SYSTEM : 0;
    context->config.sc.excludeFlags |= logExcludeTimeGetBox->isChecked() ? LCF_TIMEGET : 0;
    context->config.sc.excludeFlags |= logExcludeTimeSetBox->isChecked() ? LCF_TIMESET : 0;
    context->config.sc.excludeFlags |= logExcludeTimersBox->isChecked() ? LCF_TIMERS : 0;
    context->config.sc.excludeFlags |= logExcludeThreadsBox->isChecked() ? LCF_THREAD : 0;
    context->config.sc.excludeFlags |= logExcludeWaitBox->isChecked() ? LCF_WAIT : 0;
    context->config.sc.excludeFlags |= logExcludeWindowsBox->isChecked() ? LCF_WINDOW : 0;
    context->config.sc.excludeFlags |= logExcludeWineBox->isChecked() ? LCF_WINE : 0;

    context->config.sc_modified = true;
}

void DebugPane::update(int status)
{
    switch (status) {
    case Context::INACTIVE:
        generalBox->setEnabled(true);
        break;
    case Context::STARTING:
        generalBox->setEnabled(false);
        break;
    }
}
