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

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>

#include "GameSpecificPane.h"
#include "../../Context.h"
#include "tooltip/ToolTipCheckBox.h"

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

    QGroupBox *timingGroupBox = new QGroupBox(tr("Timing settings"));
    QVBoxLayout *timingLayout = new QVBoxLayout;
    timingLayout->addWidget(timingCeleste);
    timingGroupBox->setLayout(timingLayout);

    /* Sync settings */
    syncCeleste = new ToolTipCheckBox("Celeste");
    syncWitness = new ToolTipCheckBox("The Witness");

    QGroupBox *syncGroupBox = new QGroupBox(tr("Sync settings"));
    QVBoxLayout *syncLayout = new QVBoxLayout;
    syncLayout->addWidget(syncCeleste);
    syncLayout->addWidget(syncWitness);
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
    connect(timingCeleste, &QCheckBox::toggled, this, &GameSpecificPane::saveConfig);
    connect(syncCeleste, &QCheckBox::toggled, this, &GameSpecificPane::saveConfig);
    connect(syncWitness, &QCheckBox::toggled, this, &GameSpecificPane::saveConfig);
}

void GameSpecificPane::initToolTips()
{
    timingCeleste->setTitle("Celeste Timing");
    timingCeleste->setDescription("Fake advance the timer on <code>sched_yield()</code> "
    "calls so that the game does not softlock.");

    syncCeleste->setTitle("Celeste Sync");
    syncCeleste->setDescription("Pause the main thread, waiting for threads with "
    "specific names to finish processing. Thanks to that, loading times are "
    "deterministic.");

    syncWitness->setTitle("The Witness Sync");
    syncWitness->setDescription("Sync main thread with rendering thread, by hooking "
    "specific wined3d functions.");
}

void GameSpecificPane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void GameSpecificPane::loadConfig()
{
    timingCeleste->setChecked(context->config.sc.game_specific_timing & SharedConfig::GC_TIMING_CELESTE);
    syncCeleste->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_CELESTE);
    syncWitness->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_WITNESS);
}

void GameSpecificPane::saveConfig()
{
    context->config.sc.game_specific_timing = 0;
    context->config.sc.game_specific_timing |= timingCeleste->isChecked()? SharedConfig::GC_TIMING_CELESTE : 0;

    context->config.sc.game_specific_sync = 0;
    context->config.sc.game_specific_sync |= syncCeleste->isChecked()? SharedConfig::GC_SYNC_CELESTE : 0;
    context->config.sc.game_specific_sync |= syncWitness->isChecked()? SharedConfig::GC_SYNC_WITNESS : 0;

    context->config.sc_modified = true;
}
