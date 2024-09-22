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

#include "MovieSettingsWindow.h"

#include "Context.h"
#include "movie/MovieFile.h"
#include "../shared/SharedConfig.h"
#include "settings/tooltip/ToolTipGroupBox.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>

MovieSettingsWindow::MovieSettingsWindow(Context *c, MovieFile *m, QWidget *parent) : QDialog(parent), context(c), movie(m)
{
    setWindowTitle("Movie Settings");

    QGroupBox* inputBox = new QGroupBox(tr("Inputs"));

    mouseSupportChoice = new QComboBox();
    mouseSupportChoice->addItem(tr("No"), 0);
    mouseSupportChoice->addItem(tr("Yes"), 1);

    joystickSupportChoice = new QComboBox();
    joystickSupportChoice->addItem(tr("No"), 0);
    joystickSupportChoice->addItem(tr("1"), 1);
    joystickSupportChoice->addItem(tr("2"), 2);
    joystickSupportChoice->addItem(tr("3"), 3);
    joystickSupportChoice->addItem(tr("4"), 4);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    inputLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    inputLayout->addRow(new QLabel(tr("Mouse support:")), mouseSupportChoice);
    inputLayout->addRow(new QLabel(tr("Joystick support:")), joystickSupportChoice);
    inputBox->setLayout(inputLayout);

    QGroupBox* timeBox = new QGroupBox(tr("Time"));

    framerateNum = new QSpinBox();
    framerateNum->setMaximum(1000000000);

    framerateDen = new QSpinBox();
    framerateDen->setMaximum(1000000000);
    
    variableFramerate = new QComboBox();
    variableFramerate->addItem(tr("No"), 0);
    variableFramerate->addItem(tr("Yes"), 1);

    autoRestart = new QComboBox();
    autoRestart->addItem(tr("No"), 0);
    autoRestart->addItem(tr("Yes"), 1);

    initialSec = new QSpinBox();
    initialSec->setMaximum(1000000000);

    initialNSec = new QSpinBox();
    initialNSec->setMaximum(1000000000);

    initialMonotonicSec = new QSpinBox();
    initialMonotonicSec->setMaximum(1000000000);

    initialMonotonicNSec = new QSpinBox();
    initialMonotonicNSec->setMaximum(1000000000);

    QFormLayout *timeLayout = new QFormLayout;
    timeLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    timeLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    timeLayout->addRow(new QLabel(tr("Framerate numerator:")), framerateNum);
    timeLayout->addRow(new QLabel(tr("Framerate denominator:")), framerateDen);
    timeLayout->addRow(new QLabel(tr("Variable framerate:")), variableFramerate);
    timeLayout->addRow(new QLabel(tr("Auto-restart:")), autoRestart);
    timeLayout->addRow(new QLabel(tr("Initial elapsed time seconds:")), initialMonotonicSec);
    timeLayout->addRow(new QLabel(tr("Initial elapsed time nanoseconds:")), initialMonotonicNSec);
    timeLayout->addRow(new QLabel(tr("Initial system time seconds:")), initialSec);
    timeLayout->addRow(new QLabel(tr("Initial system time nanoseconds:")), initialNSec);
    timeBox->setLayout(timeLayout);

    timeTrackingBox = new ToolTipGroupBox(tr("Time tracking"));

    timeTrackingTime = new QSpinBox();
    timeTrackingGettimeofday = new QSpinBox();
    timeTrackingClock = new QSpinBox();
    timeTrackingClockgettimereal = new QSpinBox();
    timeTrackingClockgettimemonotonic = new QSpinBox();
    timeTrackingSdlgetticks = new QSpinBox();
    timeTrackingSdlgetperformancecounter = new QSpinBox();
    timeTrackingGetTickCount = new QSpinBox();
    timeTrackingGetTickCount64 = new QSpinBox();
    timeTrackingQueryPerformanceCounter = new QSpinBox();

    timeTrackingTime->setMaximum(1000000000);
    timeTrackingGettimeofday->setMaximum(1000000000);
    timeTrackingClock->setMaximum(1000000000);
    timeTrackingClockgettimereal->setMaximum(1000000000);
    timeTrackingClockgettimemonotonic->setMaximum(1000000000);
    timeTrackingSdlgetticks->setMaximum(1000000000);
    timeTrackingSdlgetperformancecounter->setMaximum(1000000000);
    timeTrackingGetTickCount->setMaximum(1000000000);
    timeTrackingGetTickCount64->setMaximum(1000000000);
    timeTrackingQueryPerformanceCounter->setMaximum(1000000000);

    timeTrackingTime->setMinimum(-1);
    timeTrackingGettimeofday->setMinimum(-1);
    timeTrackingClock->setMinimum(-1);
    timeTrackingClockgettimereal->setMinimum(-1);
    timeTrackingClockgettimemonotonic->setMinimum(-1);
    timeTrackingSdlgetticks->setMinimum(-1);
    timeTrackingSdlgetperformancecounter->setMinimum(-1);
    timeTrackingGetTickCount->setMinimum(-1);
    timeTrackingGetTickCount64->setMinimum(-1);
    timeTrackingQueryPerformanceCounter->setMinimum(-1);

    QFormLayout *timeTrackingLayout = new QFormLayout;
    timeTrackingLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    timeTrackingLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    timeTrackingLayout->addRow(new QLabel(tr("time:")), timeTrackingTime);
    timeTrackingLayout->addRow(new QLabel(tr("gettimeofday:")), timeTrackingGettimeofday);
    timeTrackingLayout->addRow(new QLabel(tr("clock:")), timeTrackingClock);
    timeTrackingLayout->addRow(new QLabel(tr("clock_gettime_real:")), timeTrackingClockgettimereal);
    timeTrackingLayout->addRow(new QLabel(tr("clock_gettime_monotonic:")), timeTrackingClockgettimemonotonic);
    timeTrackingLayout->addRow(new QLabel(tr("sdl_getticks:")), timeTrackingSdlgetticks);
    timeTrackingLayout->addRow(new QLabel(tr("sdl_getperformancecounter:")), timeTrackingSdlgetperformancecounter);
    timeTrackingLayout->addRow(new QLabel(tr("GetTickCount:")), timeTrackingGetTickCount);
    timeTrackingLayout->addRow(new QLabel(tr("GetTickCount64:")), timeTrackingGetTickCount64);
    timeTrackingLayout->addRow(new QLabel(tr("QueryPerformanceCounter:")), timeTrackingQueryPerformanceCounter);
    timeTrackingBox->setLayout(timeTrackingLayout);

    timeTrackingBox->setTitle("Time-tracking value");
    timeTrackingBox->setDescription("The value determines after how many calls "
    "of the specific function the deterministic timer is advanced.<br><br>"
    "By default, when enabled, the value is set to 100. If you are still having occasionnal non-draw frames, "
    "you can try increasing the value. It will make the game run slower but more "
    "deterministic. Set the value to -1 to disable time-tracking of the specific function.");

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &MovieSettingsWindow::saveConfig);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MovieSettingsWindow::reject);

    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(inputBox);
    mainLayout->addWidget(timeBox);
    mainLayout->addWidget(timeTrackingBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void MovieSettingsWindow::showEvent(QShowEvent *event)
{
    loadConfig();
}

void MovieSettingsWindow::loadConfig()
{
    /* Extract and load movie */
    int ret = movie->loadMovie();
    if (ret < 0)
        return;

    /* Load settings */
    mouseSupportChoice->setCurrentIndex(context->config.sc.mouse_support);
    int index = joystickSupportChoice->findData(context->config.sc.nb_controllers);
    if (index != -1) joystickSupportChoice->setCurrentIndex(index);

    framerateNum->setValue(context->config.sc.initial_framerate_num);
    framerateDen->setValue(context->config.sc.initial_framerate_den);
    variableFramerate->setCurrentIndex(context->config.sc.variable_framerate);
    autoRestart->setCurrentIndex(context->config.auto_restart);

    initialSec->setValue(context->config.sc.initial_time_sec);
    initialNSec->setValue(context->config.sc.initial_time_nsec);
    initialMonotonicSec->setValue(context->config.sc.initial_monotonic_time_sec);
    initialMonotonicNSec->setValue(context->config.sc.initial_monotonic_time_nsec);

    timeTrackingTime->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
    timeTrackingGettimeofday->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
    timeTrackingClock->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
    timeTrackingClockgettimereal->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME]);
    timeTrackingClockgettimemonotonic->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC]);
    timeTrackingSdlgetticks->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
    timeTrackingSdlgetperformancecounter->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);
    timeTrackingGetTickCount->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT]);
    timeTrackingGetTickCount64->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT64]);
    timeTrackingQueryPerformanceCounter->setValue(context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER]);
}

void MovieSettingsWindow::saveConfig()
{
    context->config.sc.mouse_support = mouseSupportChoice->currentData().toBool();
    context->config.sc.nb_controllers = joystickSupportChoice->currentData().toInt();
    context->config.sc.initial_framerate_num = framerateNum->value();
    context->config.sc.initial_framerate_den = framerateDen->value();
    context->config.sc.variable_framerate = variableFramerate->currentData().toBool();
    context->config.auto_restart = autoRestart->currentData().toBool();

    context->config.sc.initial_time_sec = initialSec->value();
    context->config.sc.initial_time_nsec = initialNSec->value();
    context->config.sc.initial_monotonic_time_sec = initialMonotonicSec->value();
    context->config.sc.initial_monotonic_time_nsec = initialMonotonicNSec->value();

    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME] = timeTrackingTime->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY] = timeTrackingGettimeofday->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK] = timeTrackingClock->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_REALTIME] = timeTrackingClockgettimereal->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME_MONOTONIC] = timeTrackingClockgettimemonotonic->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS] = timeTrackingSdlgetticks->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER] = timeTrackingSdlgetperformancecounter->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT] = timeTrackingGetTickCount->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTICKCOUNT64] = timeTrackingGetTickCount64->value();
    context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER] = timeTrackingQueryPerformanceCounter->value();
    
    movie->saveMovie();

    accept();
}
