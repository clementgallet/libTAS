/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "ControllerWindow.h"
#include "ControllerAxisWidget.h"
// #include "MainWindow.h"
// #include <iostream>

// static Fl_Callback axis_value_update_cb;
// static Fl_Callback axis_update_cb;

ControllerWindow::ControllerWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    button_a = new QCheckBox("A");
    button_b = new QCheckBox("B");
    button_x = new QCheckBox("X");
    button_y = new QCheckBox("Y");

    button_dpad_down = new QCheckBox("Down");
    button_dpad_right = new QCheckBox("Right");
    button_dpad_left = new QCheckBox("Left");
    button_dpad_up = new QCheckBox("Up");

    button_rb = new QCheckBox("Right button");
    button_lb = new QCheckBox("Left button");
    button_rs = new QCheckBox("Right stick");
    button_ls = new QCheckBox("Left stick");

    axis_left = new ControllerAxisWidget();
    axis_right = new ControllerAxisWidget();

    axis_left_x = new QSpinBox();
    axis_left_x->setRange(INT16_MIN, INT16_MAX);
    connect(axis_left_x, QOverload<int>::of(&QSpinBox::valueChanged), axis_left, &ControllerAxisWidget::slotSetXAxis);
    connect(axis_left, &ControllerAxisWidget::XAxisChanged, axis_left_x, &QSpinBox::setValue);

    axis_left_y = new QSpinBox();
    axis_left_y->setRange(INT16_MIN, INT16_MAX);
    connect(axis_left_y, QOverload<int>::of(&QSpinBox::valueChanged), axis_left, &ControllerAxisWidget::slotSetYAxis);
    connect(axis_left, &ControllerAxisWidget::YAxisChanged, axis_left_y, &QSpinBox::setValue);

    axis_right_x = new QSpinBox();
    axis_right_x->setRange(INT16_MIN, INT16_MAX);
    connect(axis_right_x, QOverload<int>::of(&QSpinBox::valueChanged), axis_right, &ControllerAxisWidget::slotSetXAxis);
    connect(axis_right, &ControllerAxisWidget::XAxisChanged, axis_right_x, &QSpinBox::setValue);

    axis_right_y = new QSpinBox();
    axis_right_y->setRange(INT16_MIN, INT16_MAX);
    connect(axis_right_y, QOverload<int>::of(&QSpinBox::valueChanged), axis_right, &ControllerAxisWidget::slotSetYAxis);
    connect(axis_right, &ControllerAxisWidget::YAxisChanged, axis_right_y, &QSpinBox::setValue);

    button_back = new QCheckBox("Back");
    button_guide = new QCheckBox("Guide");
    button_start = new QCheckBox("Start");

    trigger_right = new QSlider(Qt::Vertical);
    trigger_right->setMinimum(INT16_MIN);
    trigger_right->setMaximum(INT16_MAX);

    trigger_left = new QSlider(Qt::Vertical);
    trigger_left->setMinimum(INT16_MIN);
    trigger_left->setMaximum(INT16_MAX);

    trigger_right_value = new QSpinBox();
    trigger_right_value->setRange(INT16_MIN, INT16_MAX);
    connect(trigger_right_value, QOverload<int>::of(&QSpinBox::valueChanged), trigger_right, &QSlider::value);
    connect(trigger_right, &QSlider::valueChanged, trigger_right_value, &QSpinBox::setValue);

    trigger_left_value = new QSpinBox();
    trigger_left_value->setRange(INT16_MIN, INT16_MAX);
    connect(trigger_left_value, QOverload<int>::of(&QSpinBox::valueChanged), trigger_left, &QSlider::value);
    connect(trigger_left, &QSlider::valueChanged, trigger_left_value, &QSpinBox::setValue);

    /* Layouts */

    /* Left Axis Layout */

    QGroupBox *leftStickBox = new QGroupBox(tr("Left Stick"));

    QHBoxLayout *leftAxesLayout = new QHBoxLayout;
    leftAxesLayout->addWidget(new QLabel("X axis"));
    leftAxesLayout->addWidget(axis_left_x);
    leftAxesLayout->addWidget(new QLabel("Y axis"));
    leftAxesLayout->addWidget(axis_left_y);

    QVBoxLayout *leftStickLayout = new QVBoxLayout;
    leftStickLayout->addLayout(leftAxesLayout);
    leftStickLayout->addWidget(axis_left, 1);

    leftStickBox->setLayout(leftStickLayout);

    /* Right Axis Layout */

    QGroupBox *rightStickBox = new QGroupBox(tr("Right Stick"));

    QHBoxLayout *rightAxesLayout = new QHBoxLayout;
    rightAxesLayout->addWidget(new QLabel("X axis"));
    rightAxesLayout->addWidget(axis_right_x);
    rightAxesLayout->addWidget(new QLabel("Y axis"));
    rightAxesLayout->addWidget(axis_right_y);

    QVBoxLayout *rightStickLayout = new QVBoxLayout;
    rightStickLayout->addLayout(rightAxesLayout);
    rightStickLayout->addWidget(axis_right, 1);

    rightStickBox->setLayout(rightStickLayout);

    /* Trigger Layout */

    QGroupBox *triggerBox = new QGroupBox(tr("Triggers"));

    QGroupBox *leftTriggerBox = new QGroupBox(tr("Left Trigger"));
    QHBoxLayout *leftTriggerLayout = new QHBoxLayout;
    leftTriggerLayout->addWidget(trigger_left);
    leftTriggerLayout->addWidget(trigger_left_value);
    leftTriggerBox->setLayout(leftTriggerLayout);

    QGroupBox *rightTriggerBox = new QGroupBox(tr("Right Trigger"));
    QHBoxLayout *rightTriggerLayout = new QHBoxLayout;
    rightTriggerLayout->addWidget(trigger_right);
    rightTriggerLayout->addWidget(trigger_right_value);
    rightTriggerBox->setLayout(rightTriggerLayout);

    QHBoxLayout *triggerLayout = new QHBoxLayout;
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

    setLayout(mainLayout);

}

// void axis_value_update_cb(Fl_Widget* w, void* v)
// {
//     ControllerWindow* cw = (ControllerWindow*) v;
//
//     /* Update each axis canvas */
//     cw->axis_left->x_axis = static_cast<short>(cw->axis_left_x->value());
//     cw->axis_left->y_axis = static_cast<short>(cw->axis_left_y->value());
//     cw->axis_right->x_axis = static_cast<short>(cw->axis_right_x->value());
//     cw->axis_right->y_axis = static_cast<short>(cw->axis_right_y->value());
//
//     cw->axis_left->redraw();
//     cw->axis_right->redraw();
//
//     cw->trigger_right->value(cw->trigger_right_value->value());
//     cw->trigger_left->value(cw->trigger_left_value->value());
// }
//
// void axis_update_cb(Fl_Widget* w, void* v)
// {
//     ControllerWindow* cw = (ControllerWindow*) v;
//
//     /* Update each axis value */
//     cw->axis_left_x->value(cw->axis_left->x_axis);
//     cw->axis_left_y->value(cw->axis_left->y_axis);
//     cw->axis_right_x->value(cw->axis_right->x_axis);
//     cw->axis_right_y->value(cw->axis_right->y_axis);
//
//     cw->trigger_right_value->value(cw->trigger_right->value());
//     cw->trigger_left_value->value(cw->trigger_left->value());
//
// }
