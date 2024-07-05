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

#include "ControllerWidget.h"
#include "ControllerAxisWidget.h"

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>

ControllerWidget::ControllerWidget(QWidget *parent) : QWidget(parent)
{
    /* Create axis controls */

    axis_left = new ControllerAxisWidget();
    axis_right = new ControllerAxisWidget();

    axis_left_x = new QSpinBox();
    axis_left_x->setRange(INT16_MIN, INT16_MAX);
    // connect(axis_left_x, QOverload<int>::of(&QSpinBox::valueChanged), axis_left, &ControllerAxisWidget::slotSetXAxis);
    connect(axis_left_x, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), axis_left, &ControllerAxisWidget::slotSetXAxis);
    connect(axis_left, &ControllerAxisWidget::XAxisChanged, axis_left_x, &QSpinBox::setValue);

    axis_left_y = new QSpinBox();
    axis_left_y->setRange(INT16_MIN, INT16_MAX);
    connect(axis_left_y, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), axis_left, &ControllerAxisWidget::slotSetYAxis);
    connect(axis_left, &ControllerAxisWidget::YAxisChanged, axis_left_y, &QSpinBox::setValue);

    axis_right_x = new QSpinBox();
    axis_right_x->setRange(INT16_MIN, INT16_MAX);
    connect(axis_right_x, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), axis_right, &ControllerAxisWidget::slotSetXAxis);
    connect(axis_right, &ControllerAxisWidget::XAxisChanged, axis_right_x, &QSpinBox::setValue);

    axis_right_y = new QSpinBox();
    axis_right_y->setRange(INT16_MIN, INT16_MAX);
    connect(axis_right_y, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), axis_right, &ControllerAxisWidget::slotSetYAxis);
    connect(axis_right, &ControllerAxisWidget::YAxisChanged, axis_right_y, &QSpinBox::setValue);

    trigger_left = new QSlider(Qt::Horizontal);
    trigger_left->setMinimum(INT16_MIN);
    trigger_left->setMaximum(INT16_MAX);

    trigger_left_value = new QSpinBox();
    trigger_left_value->setRange(INT16_MIN, INT16_MAX);
    connect(trigger_left_value, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), trigger_left, &QSlider::setValue);
    connect(trigger_left, &QSlider::valueChanged, trigger_left_value, &QSpinBox::setValue);

    trigger_right = new QSlider(Qt::Horizontal);
    trigger_right->setMinimum(INT16_MIN);
    trigger_right->setMaximum(INT16_MAX);

    trigger_right_value = new QSpinBox();
    trigger_right_value->setRange(INT16_MIN, INT16_MAX);
    connect(trigger_right_value, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), trigger_right, &QSlider::setValue);
    connect(trigger_right, &QSlider::valueChanged, trigger_right_value, &QSpinBox::setValue);

    /* Create button controls */

    button_a = new QCheckBox("A");
    button_b = new QCheckBox("B");
    button_x = new QCheckBox("X");
    button_y = new QCheckBox("Y");

    button_back = new QCheckBox("Back");
    button_guide = new QCheckBox("Guide");
    button_start = new QCheckBox("Start");

    button_ls = new QCheckBox("Left stick");
    button_rs = new QCheckBox("Right stick");
    button_lb = new QCheckBox("Left button");
    button_rb = new QCheckBox("Right button");

    button_dpad_down = new QCheckBox("Down");
    button_dpad_right = new QCheckBox("Right");
    button_dpad_left = new QCheckBox("Left");
    button_dpad_up = new QCheckBox("Up");


    /* Layouts */

    /* Left Axis Layout */

    QGroupBox *leftStickBox = new QGroupBox(tr("Left Stick"));

    QHBoxLayout *leftAxesLayout = new QHBoxLayout;
    leftAxesLayout->addWidget(new QLabel("X:"));
    leftAxesLayout->addWidget(axis_left_x);
    leftAxesLayout->addWidget(new QLabel("Y:"));
    leftAxesLayout->addWidget(axis_left_y);

    QVBoxLayout *leftStickLayout = new QVBoxLayout;
    leftStickLayout->addLayout(leftAxesLayout);
    leftStickLayout->addWidget(axis_left);

    leftStickBox->setLayout(leftStickLayout);

    /* Right Axis Layout */

    QGroupBox *rightStickBox = new QGroupBox(tr("Right Stick"));

    QHBoxLayout *rightAxesLayout = new QHBoxLayout;
    rightAxesLayout->addWidget(new QLabel("X:"));
    rightAxesLayout->addWidget(axis_right_x);
    rightAxesLayout->addWidget(new QLabel("Y:"));
    rightAxesLayout->addWidget(axis_right_y);

    QVBoxLayout *rightStickLayout = new QVBoxLayout;
    rightStickLayout->addLayout(rightAxesLayout);
    rightStickLayout->addWidget(axis_right);

    rightStickBox->setLayout(rightStickLayout);

    /* Trigger Layout */

    QGroupBox *triggerBox = new QGroupBox(tr("Triggers"));

    QGroupBox *leftTriggerBox = new QGroupBox(tr("Left Trigger"));
    QVBoxLayout *leftTriggerLayout = new QVBoxLayout;
    leftTriggerLayout->addWidget(trigger_left);
    leftTriggerLayout->addWidget(trigger_left_value);
    leftTriggerBox->setLayout(leftTriggerLayout);

    QGroupBox *rightTriggerBox = new QGroupBox(tr("Right Trigger"));
    QVBoxLayout *rightTriggerLayout = new QVBoxLayout;
    rightTriggerLayout->addWidget(trigger_right);
    rightTriggerLayout->addWidget(trigger_right_value);
    rightTriggerBox->setLayout(rightTriggerLayout);

    QVBoxLayout *triggerLayout = new QVBoxLayout;
    triggerLayout->addWidget(leftTriggerBox);
    triggerLayout->addWidget(rightTriggerBox);

    triggerBox->setLayout(triggerLayout);

    /* Button Layout */

    QGroupBox *buttonBox = new QGroupBox(tr("Buttons"));
    QGridLayout *buttonLayout = new QGridLayout;
    buttonLayout->addWidget(button_a, 0, 0);
    buttonLayout->addWidget(button_b, 0, 1);
    buttonLayout->addWidget(button_x, 0, 2);
    buttonLayout->addWidget(button_y, 0, 3);

    buttonLayout->addWidget(button_lb, 1, 0, 1, 2);
    buttonLayout->addWidget(button_rb, 1, 2, 1, 2);
    buttonLayout->addWidget(button_ls, 2, 0, 1, 2);
    buttonLayout->addWidget(button_rs, 2, 2, 1, 2);

    buttonLayout->addWidget(button_back, 3, 0, 1, 1);
    buttonLayout->addWidget(button_guide, 3, 1, 1, 2);
    buttonLayout->addWidget(button_start, 3, 3, 1, 1);

    buttonLayout->addWidget(button_dpad_up, 4, 1, 1, 2);
    buttonLayout->addWidget(button_dpad_left, 5, 0, 1, 2);
    buttonLayout->addWidget(button_dpad_right, 5, 2, 1, 2);
    buttonLayout->addWidget(button_dpad_down, 6, 1, 1, 2);

    buttonBox->setLayout(buttonLayout);

    /* Main Layout */
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(leftStickBox, 0, 0);
    mainLayout->addWidget(rightStickBox, 0, 1);
    mainLayout->addWidget(triggerBox, 1, 0);
    mainLayout->addWidget(buttonBox, 1, 1);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 1);

    setLayout(mainLayout);
}
