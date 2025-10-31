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

#include "GameSpecificPane.h"
#include "tooltip/ToolTipCheckBox.h"

#include "Context.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>

GameSpecificPane::GameSpecificPane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void GameSpecificPane::initLayout()
{
    /* Timing settings */
    timingCeleste = new ToolTipCheckBox("Celeste");
    timingArmaCwa = new ToolTipCheckBox("Arma Cold War Assault");

    timingGroupBox = new QGroupBox(tr("Timing settings"));
    QVBoxLayout *timingLayout = new QVBoxLayout;
    timingLayout->addWidget(timingCeleste);
    timingLayout->addWidget(timingArmaCwa);
    timingGroupBox->setLayout(timingLayout);

    /* Sync settings */
    syncCeleste = new ToolTipCheckBox("Celeste");
    syncWitness = new ToolTipCheckBox("The Witness");
    syncUnityJobs = new ToolTipCheckBox("Unity Jobs");
    syncUnityLoads = new ToolTipCheckBox("Unity Loads");
    syncUnityReads = new ToolTipCheckBox("Unity Reads");

    syncGroupBox = new QGroupBox(tr("Sync settings"));
    QVBoxLayout *syncLayout = new QVBoxLayout;
    syncLayout->addWidget(syncCeleste);
    syncLayout->addWidget(syncWitness);
    syncLayout->addWidget(syncUnityJobs);
    syncLayout->addWidget(syncUnityLoads);
    syncLayout->addWidget(syncUnityReads);
    syncGroupBox->setLayout(syncLayout);

    /* Create the main layout */
    QHBoxLayout *mainLayout = new QHBoxLayout;

    mainLayout->addWidget(timingGroupBox);
    mainLayout->addWidget(syncGroupBox);
    
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void GameSpecificPane::initSignals()
{
    connect(timingCeleste, &QAbstractButton::clicked, this, &GameSpecificPane::saveConfig);
    connect(timingArmaCwa, &QAbstractButton::clicked, this, &GameSpecificPane::saveConfig);
    connect(syncCeleste, &QAbstractButton::clicked, this, &GameSpecificPane::saveConfig);
    connect(syncWitness, &QAbstractButton::clicked, this, &GameSpecificPane::saveConfig);
    connect(syncUnityJobs, &QAbstractButton::clicked, this, &GameSpecificPane::saveConfig);
    connect(syncUnityLoads, &QAbstractButton::clicked, this, &GameSpecificPane::saveConfig);
    connect(syncUnityReads, &QAbstractButton::clicked, this, &GameSpecificPane::saveConfig);
}

void GameSpecificPane::initToolTips()
{
    timingCeleste->setTitle("Celeste Timing");
    timingCeleste->setDescription("Fake advance the timer on <code>sched_yield()</code> "
    "calls so that the game does not softlock.");

    timingArmaCwa->setTitle("Arma Cold War Assault Timing");
    timingArmaCwa->setDescription("Set the thread named \"G.Main\" as the main thread. "
    "This prevents a softlock as long as clock_gettime() monotonic time tracking is enabled.");

    syncCeleste->setTitle("Celeste Sync");
    syncCeleste->setDescription("Pause the main thread, waiting for threads with "
    "specific names to finish processing. Thanks to that, loading times are "
    "deterministic.");

    syncWitness->setTitle("The Witness Sync");
    syncWitness->setDescription("Sync main thread with rendering thread, by hooking "
    "specific wined3d functions.");

    syncUnityJobs->setTitle("Unity Jobs Sync");
    syncUnityJobs->setDescription("When a job is scheduled, it waits for the job to "
    "be finished. Also, when multiple jobs are scheduled at the same time (e.g. with "
    "a parallel for), then it schedules and waits for each job one at a time.");

    syncUnityLoads->setTitle("Unity Loads Sync");
    syncUnityLoads->setDescription("When a preload operation is added to the queue "
    "it waits for the operation to be processed by the Preload thread.");

    syncUnityReads->setTitle("Unity Reads Sync");
    syncUnityReads->setDescription("When an async read is performed, it tries to "
    "perform the read synchronously. Either by doing the read itself, or by "
    "scheduling and waiting for the read to be finished.");
}

void GameSpecificPane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void GameSpecificPane::loadConfig()
{
    timingCeleste->setChecked(context->config.sc.game_specific_timing & SharedConfig::GC_TIMING_CELESTE);
    timingArmaCwa->setChecked(context->config.sc.game_specific_timing & SharedConfig::GC_TIMING_ARMA_CWA);
    syncCeleste->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_CELESTE);
    syncWitness->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_WITNESS);
    syncUnityJobs->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_UNITY_JOBS);
    syncUnityLoads->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_UNITY_LOADS);
    syncUnityReads->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_UNITY_READS);
}

void GameSpecificPane::saveConfig()
{
    context->config.sc.game_specific_timing = 0;
    context->config.sc.game_specific_timing |= timingCeleste->isChecked()? SharedConfig::GC_TIMING_CELESTE : 0;
    context->config.sc.game_specific_timing |= timingArmaCwa->isChecked()? SharedConfig::GC_TIMING_ARMA_CWA : 0;

    context->config.sc.game_specific_sync = 0;
    context->config.sc.game_specific_sync |= syncCeleste->isChecked()? SharedConfig::GC_SYNC_CELESTE : 0;
    context->config.sc.game_specific_sync |= syncWitness->isChecked()? SharedConfig::GC_SYNC_WITNESS : 0;
    context->config.sc.game_specific_sync |= syncUnityJobs->isChecked()? SharedConfig::GC_SYNC_UNITY_JOBS : 0;
    context->config.sc.game_specific_sync |= syncUnityLoads->isChecked()? SharedConfig::GC_SYNC_UNITY_LOADS : 0;
    context->config.sc.game_specific_sync |= syncUnityReads->isChecked()? SharedConfig::GC_SYNC_UNITY_READS : 0;

    context->config.sc_modified = true;
}

void GameSpecificPane::update(int status)
{
    switch (status) {
    case Context::INACTIVE:
        timingGroupBox->setEnabled(true);
        syncGroupBox->setEnabled(true);
        break;
    case Context::STARTING:
        timingGroupBox->setEnabled(false);
        syncGroupBox->setEnabled(false);
        break;
    }
}
