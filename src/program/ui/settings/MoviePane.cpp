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
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>

#include "MoviePane.h"
#include "../../Context.h"
#include "tooltip/ToolTipComboBox.h"

MoviePane::MoviePane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void MoviePane::initLayout()
{
    autosaveBox = new QGroupBox(tr("Autosave"));
    autosaveBox->setCheckable(true);

    autosaveDelay = new QDoubleSpinBox();
    autosaveDelay->setMaximum(1000000000);
    
    autosaveFrames = new QSpinBox();
    autosaveFrames->setMaximum(1000000000);

    autosaveCount = new QSpinBox();
    autosaveCount->setMaximum(1000000000);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->addRow(new QLabel(tr("Minimum delay between autosaves (sec):")), autosaveDelay);
    formLayout->addRow(new QLabel(tr("Minimum advanced frames between autosaves:")), autosaveFrames);
    formLayout->addRow(new QLabel(tr("Maximum autosave count:")), autosaveCount);
    autosaveBox->setLayout(formLayout);

    QGroupBox* generalBox = new QGroupBox(tr("General"));
    QFormLayout* generalLayout = new QFormLayout;
    generalBox->setLayout(generalLayout);

    generalLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    generalLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    endChoice = new ToolTipComboBox();
    endChoice->addItem(tr("Keep Reading"), Config::MOVIEEND_READ);
    endChoice->addItem(tr("Switch to Writing"), Config::MOVIEEND_WRITE);

    generalLayout->addRow(new QLabel(tr("On Movie End:")), endChoice);

    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalBox);
    mainLayout->addWidget(autosaveBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);    
}

void MoviePane::initSignals()
{
    connect(autosaveBox, &QGroupBox::clicked, this, &MoviePane::saveConfig);
    connect(autosaveDelay, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MoviePane::saveConfig);
    connect(autosaveFrames, QOverload<int>::of(&QSpinBox::valueChanged), this, &MoviePane::saveConfig);
    connect(autosaveCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &MoviePane::saveConfig);
    connect(endChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MoviePane::saveConfig);    
}

void MoviePane::initToolTips()
{
    endChoice->setTitle("On Movie End");
    endChoice->setDescription("Indicate what to do after a reaching the end "
    "of a movie in playback mode:<br><br>"
    "<b>Keep Reading:</b> Stay in playback mode, and send blank inputs on each frame."
    "A blank input is defined as all bool inputs set to false, all value inputs set to 0.<br><br>"
    "<b>Switch to Writing:</b> Switch to writing mode.");
}


void MoviePane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void MoviePane::loadConfig()
{
    autosaveBox->setChecked(context->config.autosave);

    /* We don't want to trigger the signals */
    autosaveDelay->blockSignals(true);
    autosaveFrames->blockSignals(true);
    autosaveCount->blockSignals(true);
    autosaveDelay->setValue(context->config.autosave_delay_sec);
    autosaveFrames->setValue(context->config.autosave_frames);
    autosaveCount->setValue(context->config.autosave_count);
    autosaveDelay->blockSignals(false);
    autosaveFrames->blockSignals(false);
    autosaveCount->blockSignals(false);

    int index = endChoice->findData(context->config.on_movie_end);
    if (index != -1) endChoice->setCurrentIndex(index);
}

void MoviePane::saveConfig()
{
    context->config.autosave = autosaveBox->isChecked();
    context->config.autosave_delay_sec = autosaveDelay->value();
    context->config.autosave_frames = autosaveFrames->value();
    context->config.autosave_count = autosaveCount->value();

    context->config.on_movie_end = endChoice->itemData(endChoice->currentIndex()).toInt();
    context->config.sc_modified = true;
}

void MoviePane::update(int status)
{
    switch (status) {
    case Context::INACTIVE:
        break;
    case Context::STARTING:
        break;
    }
}
