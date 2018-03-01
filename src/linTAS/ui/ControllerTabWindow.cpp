/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "ControllerTabWindow.h"
#include "../GameLoop.h"
#include "MainWindow.h"

ControllerTabWindow::ControllerTabWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Controller inputs");

    /* Create each controller widget */
    QTabWidget *tabWidget = new QTabWidget;

    for (int j=0; j<AllInputs::MAXJOYS; j++) {
        controllers[j] = new ControllerWidget();
        tabWidget->addTab(controllers[j], QString("Controller %1").arg(j+1));
    }

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    /* We need connections to the game loop, so we access it through our parent */
    MainWindow *mw = qobject_cast<MainWindow*>(parent);
    if (mw) {
        /* If the user press an input that is mapped to a controller button,
         * we must change the corresponding checkbox in this window.
         */
        connect(mw->gameLoop, &GameLoop::controllerButtonToggled, this, &ControllerTabWindow::slotButtonToggle);

        /* When the game loop will send the inputs to the game, we must set
         * the controller inputs in the AllInputs object.
         */
        connect(mw->gameLoop, &GameLoop::inputsToBeSent, this, &ControllerTabWindow::slotSetInputs, Qt::DirectConnection);
    }
}

void ControllerTabWindow::slotButtonToggle(int id, int button, bool pressed)
{
    switch(button) {
    case AllInputs::BUTTON_A:
        controllers[id]->button_a->setChecked(pressed);
        break;
    case AllInputs::BUTTON_B:
        controllers[id]->button_b->setChecked(pressed);
        break;
    case AllInputs::BUTTON_X:
        controllers[id]->button_x->setChecked(pressed);
        break;
    case AllInputs::BUTTON_Y:
        controllers[id]->button_y->setChecked(pressed);
        break;
    case AllInputs::BUTTON_BACK:
        controllers[id]->button_back->setChecked(pressed);
        break;
    case AllInputs::BUTTON_GUIDE:
        controllers[id]->button_guide->setChecked(pressed);
        break;
    case AllInputs::BUTTON_START:
        controllers[id]->button_start->setChecked(pressed);
        break;
    case AllInputs::BUTTON_LEFTSTICK:
        controllers[id]->button_ls->setChecked(pressed);
        break;
    case AllInputs::BUTTON_RIGHTSTICK:
        controllers[id]->button_rs->setChecked(pressed);
        break;
    case AllInputs::BUTTON_LEFTSHOULDER:
        controllers[id]->button_lb->setChecked(pressed);
        break;
    case AllInputs::BUTTON_RIGHTSHOULDER:
        controllers[id]->button_rb->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_UP:
        controllers[id]->button_dpad_up->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_DOWN:
        controllers[id]->button_dpad_down->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_LEFT:
        controllers[id]->button_dpad_left->setChecked(pressed);
        break;
    case AllInputs::BUTTON_DPAD_RIGHT:
        controllers[id]->button_dpad_right->setChecked(pressed);
        break;
    }
}

void ControllerTabWindow::slotSetInputs(AllInputs &ai)
{
    /* Don't set inputs from the controller input window if the window is hidden */
    if (!isVisible())
        return;
        
    for (int j=0; j<AllInputs::MAXJOYS; j++) {
        /* Set controller axes */
        ai.controller_axes[j][AllInputs::AXIS_LEFTX] = controllers[j]->axis_left->x_axis;
        ai.controller_axes[j][AllInputs::AXIS_LEFTY] = controllers[j]->axis_left->y_axis;
        ai.controller_axes[j][AllInputs::AXIS_RIGHTX] = controllers[j]->axis_right->x_axis;
        ai.controller_axes[j][AllInputs::AXIS_RIGHTY] = controllers[j]->axis_right->y_axis;
        ai.controller_axes[j][AllInputs::AXIS_TRIGGERLEFT] = static_cast<short>(controllers[j]->trigger_left_value->value());
        ai.controller_axes[j][AllInputs::AXIS_TRIGGERRIGHT] = static_cast<short>(controllers[j]->trigger_right_value->value());

        /* Set controller buttons */
        ai.controller_buttons[j] = 0;

        ai.controller_buttons[j] |= (controllers[j]->button_a->isChecked() << AllInputs::BUTTON_A);
        ai.controller_buttons[j] |= (controllers[j]->button_b->isChecked() << AllInputs::BUTTON_B);
        ai.controller_buttons[j] |= (controllers[j]->button_x->isChecked() << AllInputs::BUTTON_X);
        ai.controller_buttons[j] |= (controllers[j]->button_y->isChecked() << AllInputs::BUTTON_Y);
        ai.controller_buttons[j] |= (controllers[j]->button_back->isChecked() << AllInputs::BUTTON_BACK);
        ai.controller_buttons[j] |= (controllers[j]->button_guide->isChecked() << AllInputs::BUTTON_GUIDE);
        ai.controller_buttons[j] |= (controllers[j]->button_start->isChecked() << AllInputs::BUTTON_START);
        ai.controller_buttons[j] |= (controllers[j]->button_ls->isChecked() << AllInputs::BUTTON_LEFTSTICK);
        ai.controller_buttons[j] |= (controllers[j]->button_rs->isChecked() << AllInputs::BUTTON_RIGHTSTICK);
        ai.controller_buttons[j] |= (controllers[j]->button_lb->isChecked() << AllInputs::BUTTON_LEFTSHOULDER);
        ai.controller_buttons[j] |= (controllers[j]->button_rb->isChecked() << AllInputs::BUTTON_RIGHTSHOULDER);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_up->isChecked() << AllInputs::BUTTON_DPAD_UP);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_down->isChecked() << AllInputs::BUTTON_DPAD_DOWN);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_left->isChecked() << AllInputs::BUTTON_DPAD_LEFT);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_right->isChecked() << AllInputs::BUTTON_DPAD_RIGHT);
    }
}

void ControllerTabWindow::keyPressEvent(QKeyEvent *e)
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

void ControllerTabWindow::keyReleaseEvent(QKeyEvent *e)
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
