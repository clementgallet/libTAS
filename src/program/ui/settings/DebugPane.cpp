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

#include "DebugPane.h"
#include "tooltip/ToolTipCheckBox.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLineEdit>

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
    QGridLayout* generalLayout = new QGridLayout;
    generalBox->setLayout(generalLayout);

    debugUncontrolledBox = new ToolTipCheckBox(tr("Uncontrolled time"));
    debugEventsBox = new ToolTipCheckBox(tr("Native events"));
    debugMainBox = new ToolTipCheckBox(tr("Keep main first thread"));
    debugIOBox = new ToolTipCheckBox(tr("Native file IO"));
    debugInetBox = new ToolTipCheckBox(tr("Native internet"));

    generalLayout->addWidget(debugUncontrolledBox, 0, 0);
    generalLayout->addWidget(debugEventsBox, 1, 0);
    generalLayout->addWidget(debugMainBox, 2, 0);
    generalLayout->addWidget(debugIOBox, 0, 1);
    generalLayout->addWidget(debugInetBox, 1, 1);

    QGroupBox* debuggerBox = new QGroupBox(tr("Debugger"));
    QFormLayout* debuggerLayout = new QFormLayout;
    debuggerLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    debuggerLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    debuggerBox->setLayout(debuggerLayout);

    debugSigIntBox = new QCheckBox();
    debugStraceEvents = new QLineEdit();

    debuggerLayout->addRow(new QLabel(tr("Raise SIGINT upon game launch:")), debugSigIntBox);
    debuggerLayout->addRow(new QLabel(tr("Strace traced events (-e):")), debugStraceEvents);

    QGroupBox* logBox = new QGroupBox(tr("Logging"));
    QVBoxLayout* logLayout = new QVBoxLayout;
    logBox->setLayout(logLayout);

    logToChoice = new QComboBox();

    logToChoice->addItem(tr("Disabled logging"), SharedConfig::NO_LOGGING);
    logToChoice->addItem(tr("Log to console"), SharedConfig::LOGGING_TO_CONSOLE);
    logToChoice->addItem(tr("Log to file"), SharedConfig::LOGGING_TO_FILE);

    QGroupBox* logLevelBox = new QGroupBox(tr("Level"));
    logLevelSlider = new QSlider(Qt::Horizontal);
    logLevelSlider->setRange(0, 6);
    logLevelSlider->setSingleStep(1);
    logLevelSlider->setPageStep(1);
    logLevelSlider->setTickPosition(QSlider::TicksBelow);
    logLevelSlider->setTracking(false);

    QGridLayout *logLevelLayout = new QGridLayout;
    logLevelLayout->addWidget(logLevelSlider, 0, 1, 1, 12);

    QLabel* label = new QLabel("Fatal");
    label->setAlignment(Qt::AlignHCenter);
    logLevelLayout->addWidget(label, 1, 0, 1, 2);

    label = new QLabel("Error");
    label->setAlignment(Qt::AlignHCenter);
    logLevelLayout->addWidget(label, 1, 2, 1, 2);

    label = new QLabel("Warn");
    label->setAlignment(Qt::AlignHCenter);
    logLevelLayout->addWidget(label, 1, 4, 1, 2);

    label = new QLabel("Info");
    label->setAlignment(Qt::AlignHCenter);
    logLevelLayout->addWidget(label, 1, 6, 1, 2);

    label = new QLabel("Debug");
    label->setAlignment(Qt::AlignHCenter);
    logLevelLayout->addWidget(label, 1, 8, 1, 2);

    label = new QLabel("Trace");
    label->setAlignment(Qt::AlignHCenter);
    logLevelLayout->addWidget(label, 1, 10, 1, 2);

    label = new QLabel("Stack");
    label->setAlignment(Qt::AlignHCenter);
    logLevelLayout->addWidget(label, 1, 12, 1, 2);
    
    logLevelBox->setLayout(logLevelLayout);

    QGroupBox* logPrintBox = new QGroupBox(tr("Print"));
    QGridLayout* logPrintLayout = new QGridLayout;
    logPrintBox->setLayout(logPrintLayout);

    QGroupBox* logMainPrintBox = new QGroupBox();
    QHBoxLayout* logMainPrintLayout = new QHBoxLayout;
    logMainPrintBox->setLayout(logMainPrintLayout);

    logPrintAllBox = new QCheckBox(tr("All"));
    logPrintNoneBox = new QCheckBox(tr("Default"));
    logPrintMainBox = new QCheckBox(tr("Main Thread"));
    logPrintTODOBox = new QCheckBox(tr("TODO"));
    
    logPrintBoxes.emplace_back(new QCheckBox(tr("AV Dumping")), LCF_DUMP);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Checkpoint")), LCF_CHECKPOINT);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Events")), LCF_EVENTS);
    logPrintBoxes.emplace_back(new QCheckBox(tr("File IO")), LCF_FILEIO);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Hacks")), LCF_HACKS);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Hook")), LCF_HOOK);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Joystick")), LCF_JOYSTICK);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Keyboard")), LCF_KEYBOARD);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Locale")), LCF_LOCALE);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Mouse")), LCF_MOUSE);
    logPrintBoxes.emplace_back(new QCheckBox(tr("OpenGL/Vulkan")), LCF_OGL);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Random")), LCF_RANDOM);
    logPrintBoxes.emplace_back(new QCheckBox(tr("SDL")), LCF_SDL);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Signals")), LCF_SIGNAL);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Sleep")), LCF_SLEEP);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Socket")), LCF_SOCKET);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Sound")), LCF_SOUND);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Steam")), LCF_STEAM);
    logPrintBoxes.emplace_back(new QCheckBox(tr("System")), LCF_SYSTEM);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Time Get")), LCF_TIMEGET);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Time Set")), LCF_TIMESET);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Timers")), LCF_TIMERS);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Threads")), LCF_THREAD);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Wait")), LCF_WAIT);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Windows")), LCF_WINDOW);
    logPrintBoxes.emplace_back(new QCheckBox(tr("Wine")), LCF_WINE);
    
    logMainPrintLayout->addWidget(logPrintAllBox);
    logMainPrintLayout->addWidget(logPrintNoneBox);
    logMainPrintLayout->addWidget(logPrintMainBox);
    logMainPrintLayout->addWidget(logPrintTODOBox);

    logPrintLayout->addWidget(logMainPrintBox, 0, 0, 1, 5);

    int row = 1;
    int col = 0;
    for (auto& checkbox : logPrintBoxes) {
        checkbox.first->setTristate(true);
        logPrintLayout->addWidget(checkbox.first, row, col);
        row++;
        if (row > 5) {
            row = 1;
            col++;
        }        
    }
    
    logLayout->addWidget(logToChoice);
    logLayout->addWidget(logLevelBox);
    logLayout->addWidget(logPrintBox);

    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalBox);
    mainLayout->addWidget(debuggerBox);
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
    connect(debugStraceEvents, &QLineEdit::textEdited, this, &DebugPane::saveConfig);
    
    connect(logToChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &DebugPane::saveConfig);
    connect(logLevelSlider, &QAbstractSlider::valueChanged, this, &DebugPane::saveConfig);

    connect(logPrintAllBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintNoneBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintMainBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    connect(logPrintTODOBox, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    for (const auto& checkbox : logPrintBoxes) {
        connect(checkbox.first, &QAbstractButton::clicked, this, &DebugPane::saveConfig);
    }
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
    debugStraceEvents->setText(context->config.strace_events.c_str());

    int index = logToChoice->findData(context->config.sc.logging_status);
    if (index >= 0)
        logToChoice->setCurrentIndex(index);

    /* Disconnect to not trigger valueChanged() signal */
    disconnect(logLevelSlider, &QAbstractSlider::valueChanged, this, &DebugPane::saveConfig);
    logLevelSlider->setValue(context->config.sc.logging_level);
    connect(logLevelSlider, &QAbstractSlider::valueChanged, this, &DebugPane::saveConfig);

    logPrintMainBox->setChecked(context->config.sc.logging_include_flags & LCF_MAINTHREAD);
    logPrintTODOBox->setChecked(context->config.sc.logging_include_flags & LCF_TODO);

    for (auto& checkbox : logPrintBoxes) {
        if (context->config.sc.logging_exclude_flags & checkbox.second)
            checkbox.first->setCheckState(Qt::Unchecked);
        else if (context->config.sc.logging_include_flags & checkbox.second)
            checkbox.first->setCheckState(Qt::Checked);
        else
            checkbox.first->setCheckState(Qt::PartiallyChecked);
    }
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
    context->config.strace_events = debugStraceEvents->text().toStdString();

    context->config.sc.logging_status = logToChoice->currentData().toInt();
    
    context->config.sc.logging_level = logLevelSlider->value();
    
    context->config.sc.logging_include_flags = 0;
    context->config.sc.logging_include_flags |= logPrintMainBox->isChecked() ? LCF_MAINTHREAD : 0;
    context->config.sc.logging_include_flags |= logPrintTODOBox->isChecked() ? LCF_TODO : 0;
    context->config.sc.logging_exclude_flags = 0;

    if (logPrintAllBox->isChecked()) {
        logPrintAllBox->setChecked(false);
        context->config.sc.logging_include_flags |= LCF_ALL;
        for (auto& checkbox : logPrintBoxes) {
            checkbox.first->setCheckState(Qt::Checked);
        }
    }
    else if (logPrintNoneBox->isChecked()) {
        logPrintNoneBox->setChecked(false);
        for (auto& checkbox : logPrintBoxes) {
            checkbox.first->setCheckState(Qt::PartiallyChecked);
        }
    }
    else {
        for (auto& checkbox : logPrintBoxes) {
            Qt::CheckState state = checkbox.first->checkState();
            switch (state) {
                case Qt::Unchecked:
                    context->config.sc.logging_exclude_flags |= checkbox.second;
                    break;
                case Qt::Checked:
                    context->config.sc.logging_include_flags |= checkbox.second;
                    break;
                case Qt::PartiallyChecked:
                    break;
            }
        }
    }
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
