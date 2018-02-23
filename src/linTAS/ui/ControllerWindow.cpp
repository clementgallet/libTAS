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
#include "../GameLoop.h"
#include "../../shared/AllInputs.h"
#include "MainWindow.h"

ControllerWindow::ControllerWindow(Context* c, int id, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), controller_id(id), context(c)
{
    setWindowTitle(QString("Controller %1 inputs").arg(controller_id+1));

    /* Create axis controls */

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

    trigger_left = new QSlider(Qt::Vertical);
    trigger_left->setMinimum(INT16_MIN);
    trigger_left->setMaximum(INT16_MAX);

    trigger_left_value = new QSpinBox();
    trigger_left_value->setRange(INT16_MIN, INT16_MAX);
    connect(trigger_left_value, QOverload<int>::of(&QSpinBox::valueChanged), trigger_left, &QSlider::value);
    connect(trigger_left, &QSlider::valueChanged, trigger_left_value, &QSpinBox::setValue);

    trigger_right = new QSlider(Qt::Vertical);
    trigger_right->setMinimum(INT16_MIN);
    trigger_right->setMaximum(INT16_MAX);

    trigger_right_value = new QSpinBox();
    trigger_right_value->setRange(INT16_MIN, INT16_MAX);
    connect(trigger_right_value, QOverload<int>::of(&QSpinBox::valueChanged), trigger_right, &QSlider::value);
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

    /* We need connections to the game loop, so we access it through our parent */
    MainWindow *mw = qobject_cast<MainWindow*>(parent);
    if (mw) {
        /* If the user press an input that is mapped to a controller button,
         * we must change the corresponding checkbox in this window.
         */
        connect(mw->gameLoop, &GameLoop::controllerButtonToggled, this, &ControllerWindow::slotButtonToggle);

        /* When the game loop will send the inputs to the game, we must set
         * the controller inputs in the AllInputs object.
         */
        connect(mw->gameLoop, &GameLoop::inputsToBeSent, this, &ControllerWindow::slotSetInputs, Qt::DirectConnection);
    }

}

void ControllerWindow::slotButtonToggle(int id, int button, bool pressed)
{
    if (id != controller_id)
        return;

    switch(button) {
    case AllInputs::BUTTON_A:
        button_a->setChecked(pressed);
        break;
    case AllInputs::BUTTON_B:
        button_b->setChecked(pressed);
        break;
    case AllInputs::BUTTON_X:
        button_x->setChecked(pressed);
        break;
    case AllInputs::BUTTON_Y:
        button_y->setChecked(pressed);
        break;
    case AllInputs::BUTTON_BACK:
        button_back->setChecked(pressed);
        break;
    case AllInputs::BUTTON_GUIDE:
        button_guide->setChecked(pressed);
        break;
    case AllInputs::BUTTON_START:
        button_start->setChecked(pressed);
        break;
    case AllInputs::BUTTON_LEFTSTICK:
        button_ls->setChecked(pressed);
        break;
    case AllInputs::BUTTON_RIGHTSTICK:
        button_rs->setChecked(pressed);
        break;
    case AllInputs::BUTTON_LEFTSHOULDER:
        button_lb->setChecked(pressed);
        break;
    case AllInputs::BUTTON_RIGHTSHOULDER:
        button_rb->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_UP:
        button_dpad_up->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_DOWN:
        button_dpad_down->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_LEFT:
        button_dpad_left->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_RIGHT:
        button_dpad_right->setChecked(pressed);
        break;
    }
}

void ControllerWindow::slotSetInputs(AllInputs &ai)
{
    /* Set controller axes */
    ai.controller_axes[controller_id][AllInputs::AXIS_LEFTX] = axis_left->x_axis;
    ai.controller_axes[controller_id][AllInputs::AXIS_LEFTY] = axis_left->y_axis;
    ai.controller_axes[controller_id][AllInputs::AXIS_RIGHTX] = axis_right->x_axis;
    ai.controller_axes[controller_id][AllInputs::AXIS_RIGHTY] = axis_right->y_axis;
    ai.controller_axes[controller_id][AllInputs::AXIS_TRIGGERLEFT] = static_cast<short>(trigger_left_value->value());
    ai.controller_axes[controller_id][AllInputs::AXIS_TRIGGERRIGHT] = static_cast<short>(trigger_right_value->value());

    /* Set controller buttons */
    ai.controller_buttons[controller_id] = 0;

    ai.controller_buttons[controller_id] |= (button_a->isChecked() << AllInputs::BUTTON_A);
    ai.controller_buttons[controller_id] |= (button_b->isChecked() << AllInputs::BUTTON_B);
    ai.controller_buttons[controller_id] |= (button_x->isChecked() << AllInputs::BUTTON_X);
    ai.controller_buttons[controller_id] |= (button_y->isChecked() << AllInputs::BUTTON_Y);
    ai.controller_buttons[controller_id] |= (button_back->isChecked() << AllInputs::BUTTON_BACK);
    ai.controller_buttons[controller_id] |= (button_guide->isChecked() << AllInputs::BUTTON_GUIDE);
    ai.controller_buttons[controller_id] |= (button_start->isChecked() << AllInputs::BUTTON_START);
    ai.controller_buttons[controller_id] |= (button_ls->isChecked() << AllInputs::BUTTON_LEFTSTICK);
    ai.controller_buttons[controller_id] |= (button_rs->isChecked() << AllInputs::BUTTON_RIGHTSTICK);
    ai.controller_buttons[controller_id] |= (button_lb->isChecked() << AllInputs::BUTTON_LEFTSHOULDER);
    ai.controller_buttons[controller_id] |= (button_rb->isChecked() << AllInputs::BUTTON_RIGHTSHOULDER);
    ai.controller_buttons[controller_id] |= (button_dpad_up->isChecked() << AllInputs::BUTTON_DPAD_UP);
    ai.controller_buttons[controller_id] |= (button_dpad_down->isChecked() << AllInputs::BUTTON_DPAD_DOWN);
    ai.controller_buttons[controller_id] |= (button_dpad_left->isChecked() << AllInputs::BUTTON_DPAD_LEFT);
    ai.controller_buttons[controller_id] |= (button_dpad_right->isChecked() << AllInputs::BUTTON_DPAD_RIGHT);

}

void ControllerWindow::keyPressEvent(QKeyEvent *e)
{
    if (context->config.km.input_mapping.find(e->nativeVirtualKey()) != context->config.km.input_mapping.end()) {
        SingleInput si = context->config.km.input_mapping[e->nativeVirtualKey()];

        if (si.type >= IT_CONTROLLER1_BUTTON_A && si.type <= IT_CONTROLLER1_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(0, si.type - IT_CONTROLLER1_BUTTON_A, true);
        if (si.type >= IT_CONTROLLER2_BUTTON_A && si.type <= IT_CONTROLLER2_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(1, si.type - IT_CONTROLLER2_BUTTON_A, true);
        if (si.type >= IT_CONTROLLER3_BUTTON_A && si.type <= IT_CONTROLLER3_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(2, si.type - IT_CONTROLLER3_BUTTON_A, true);
        if (si.type >= IT_CONTROLLER4_BUTTON_A && si.type <= IT_CONTROLLER4_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(3, si.type - IT_CONTROLLER4_BUTTON_A, true);
    }

    QWidget::keyPressEvent(e);
}

void ControllerWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (context->config.km.input_mapping.find(e->nativeVirtualKey()) != context->config.km.input_mapping.end()) {
        SingleInput si = context->config.km.input_mapping[e->nativeVirtualKey()];

        if (si.type >= IT_CONTROLLER1_BUTTON_A && si.type <= IT_CONTROLLER1_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(0, si.type - IT_CONTROLLER1_BUTTON_A, false);
        if (si.type >= IT_CONTROLLER2_BUTTON_A && si.type <= IT_CONTROLLER2_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(1, si.type - IT_CONTROLLER2_BUTTON_A, false);
        if (si.type >= IT_CONTROLLER3_BUTTON_A && si.type <= IT_CONTROLLER3_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(2, si.type - IT_CONTROLLER3_BUTTON_A, false);
        if (si.type >= IT_CONTROLLER4_BUTTON_A && si.type <= IT_CONTROLLER4_BUTTON_DPAD_RIGHT)
            return slotButtonToggle(3, si.type - IT_CONTROLLER4_BUTTON_A, false);
    }

    QWidget::keyReleaseEvent(e);
}
