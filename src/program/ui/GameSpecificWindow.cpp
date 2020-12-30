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

#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include "GameSpecificWindow.h"

#include <iostream>

GameSpecificWindow::GameSpecificWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Game-specific configuration");

    /* Timing settings */
    timingCeleste = new QCheckBox("Celeste");

    QGroupBox *timingGroupBox = new QGroupBox(tr("Timing settings"));
    QVBoxLayout *timingLayout = new QVBoxLayout;
    timingLayout->addWidget(timingCeleste);
    timingGroupBox->setLayout(timingLayout);

    /* Sync settings */
    syncCeleste = new QCheckBox("Celeste");
    syncWitness = new QCheckBox("The Witness");

    QGroupBox *syncGroupBox = new QGroupBox(tr("Sync settings"));
    QVBoxLayout *syncLayout = new QVBoxLayout;
    syncLayout->addWidget(syncCeleste);
    syncLayout->addWidget(syncWitness);
    syncGroupBox->setLayout(syncLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &GameSpecificWindow::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GameSpecificWindow::reject);

    /* Create the settings */
    QHBoxLayout *settingsLayout = new QHBoxLayout;

    settingsLayout->addWidget(timingGroupBox);
    settingsLayout->addWidget(syncGroupBox);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addLayout(settingsLayout);
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    update_config();
}

void GameSpecificWindow::update_config()
{
    timingCeleste->setChecked(context->config.sc.game_specific_timing & SharedConfig::GC_TIMING_CELESTE);
    syncCeleste->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_CELESTE);
    syncWitness->setChecked(context->config.sc.game_specific_sync & SharedConfig::GC_SYNC_WITNESS);
}

void GameSpecificWindow::slotOk()
{
    context->config.sc.game_specific_timing = 0;
    if (timingCeleste->isChecked())
        context->config.sc.game_specific_timing |= SharedConfig::GC_TIMING_CELESTE;

    context->config.sc.game_specific_sync = 0;
    if (syncCeleste->isChecked())
        context->config.sc.game_specific_sync |= SharedConfig::GC_SYNC_CELESTE;
    if (syncWitness->isChecked())
        context->config.sc.game_specific_sync |= SharedConfig::GC_SYNC_WITNESS;

    context->config.sc_modified = true;

    /* Close window */
    accept();
}
