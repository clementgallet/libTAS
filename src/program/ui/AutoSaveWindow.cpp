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

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>

#include "AutoSaveWindow.h"
#include "../Context.h"

AutoSaveWindow::AutoSaveWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Autosave configuration");

    autosaveBox = new QGroupBox(tr("Autosave"));
    autosaveBox->setCheckable(true);
    // connect(movieBox, &QGroupBox::clicked, this, &MainWindow::slotMovieEnable);

    autosaveDelay = new QDoubleSpinBox();
    autosaveDelay->setMaximum(1000000000);

    autosaveFrames = new QSpinBox();
    autosaveFrames->setMaximum(1000000000);

    autosaveCount = new QSpinBox();
    autosaveCount->setMaximum(1000000000);

    /* Create the form layout */
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel(tr("Minimum delay between autosaves:")), autosaveDelay);
    formLayout->addRow(new QLabel(tr("Minimum advanced frames between autosaves:")), autosaveFrames);
    formLayout->addRow(new QLabel(tr("Maximum autosave count:")), autosaveCount);

    autosaveBox->setLayout(formLayout);

    /* Buttons */
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &AutoSaveWindow::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AutoSaveWindow::reject);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(autosaveBox);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    update_config();
}

void AutoSaveWindow::update_config()
{
    autosaveBox->setChecked(context->config.autosave);
    autosaveDelay->setValue(context->config.autosave_delay_sec);
    autosaveFrames->setValue(context->config.autosave_frames);
    autosaveCount->setValue(context->config.autosave_count);
}

void AutoSaveWindow::slotOk()
{
    context->config.autosave = autosaveBox->isChecked();
    context->config.autosave_delay_sec = autosaveDelay->value();
    context->config.autosave_frames = autosaveFrames->value();
    context->config.autosave_count = autosaveCount->value();

    /* Close window */
    accept();
}
