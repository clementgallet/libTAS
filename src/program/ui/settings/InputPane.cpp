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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>

#include "InputPane.h"
#include "../../Context.h"
#include "tooltip/ToolTipCheckBox.h"

InputPane::InputPane(Context* c) : context(c)
{
    initLayout();
    loadConfig();    
    initSignals();
    initToolTips();
}

void InputPane::initLayout()
{
    QGroupBox* mouseBox = new QGroupBox(tr("Mouse"));
    QVBoxLayout* mouseLayout = new QVBoxLayout;
    mouseBox->setLayout(mouseLayout);

    mouseSupportBox = new ToolTipCheckBox(tr("Mouse support"));
    mouseWarpBox = new ToolTipCheckBox(tr("Warp mouse to center"));
    mouseGameWarpBox = new ToolTipCheckBox(tr("Prevent mouse warping"));

    mouseLayout->addWidget(mouseSupportBox);
    mouseLayout->addWidget(mouseWarpBox);
    mouseLayout->addWidget(mouseGameWarpBox);

    QGroupBox* joyBox = new QGroupBox(tr("Joystick"));
    QFormLayout* joyLayout = new QFormLayout;
    joyLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    joyLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    joyBox->setLayout(joyLayout);

    joyChoice = new QComboBox();
    joyChoice->addItem(tr("None"), 0);
    joyChoice->addItem(tr("1"), 1);
    joyChoice->addItem(tr("2"), 2);
    joyChoice->addItem(tr("3"), 3);
    joyChoice->addItem(tr("4"), 4);

    joyLayout->addRow(new QLabel(tr("Joystick support:")), joyChoice);

    QVBoxLayout* const mainLayout = new QVBoxLayout;
    mainLayout->addWidget(mouseBox);
    mainLayout->addWidget(joyBox);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void InputPane::initSignals()
{
    connect(mouseSupportBox, &QCheckBox::toggled, this, &InputPane::saveConfig);
    connect(mouseWarpBox, &QCheckBox::toggled, this, &InputPane::saveConfig);
    connect(mouseGameWarpBox, &QCheckBox::toggled, this, &InputPane::saveConfig);
    connect(joyChoice, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &InputPane::saveConfig);
}

void InputPane::initToolTips()
{
    mouseSupportBox->setDescription("Enable mouse inputs. If unchecked, it will "
    "report a fixed mouse cursor at (0,0) coordinates.");
    mouseWarpBox->setDescription("Warp mouse to the center of the game window on each frame. "
    "This is useful in combination with relative mode, to ease mouse inputs.");
    mouseGameWarpBox->setDescription("Prevents games from warping the mouse cursor.");
}

void InputPane::showEvent(QShowEvent *event)
{
    loadConfig();
}

void InputPane::loadConfig()
{
    mouseSupportBox->setChecked(context->config.sc.mouse_support);
    mouseWarpBox->setChecked(context->config.mouse_warp);
    mouseGameWarpBox->setChecked(context->config.sc.mouse_prevent_warp);

    int index = joyChoice->findData(context->config.sc.nb_controllers);
    if (index != -1) joyChoice->setCurrentIndex(index);
}

void InputPane::saveConfig()
{
    context->config.sc.mouse_support = mouseSupportBox->isChecked();
    context->config.mouse_warp = mouseWarpBox->isChecked();
    context->config.sc.mouse_prevent_warp = mouseGameWarpBox->isChecked();

    context->config.sc.nb_controllers = joyChoice->itemData(joyChoice->currentIndex()).toInt();
    context->config.sc_modified = true;
}
