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

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>

#include "ControllerTabWindow.h"
#include "../GameLoop.h"
#include "MainWindow.h"
#include "../../shared/SingleInput.h"
#include "qtutils.h"

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

    qRegisterMetaType<AllInputs>("AllInputs");

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
        connect(mw->gameLoop, &GameLoop::fillControllerInputs, this, &ControllerTabWindow::slotSetInputs, Qt::DirectConnection);

        /* When the game loop is playing back a movie and wants to display the
         * controller inputs in this window.
         */
        connect(mw->gameLoop, &GameLoop::showControllerInputs, this, &ControllerTabWindow::slotGetInputs);
    }
}

void ControllerTabWindow::slotButtonToggle(int id, int button, bool pressed)
{
    switch(button) {
    case SingleInput::BUTTON_A:
        controllers[id]->button_a->setChecked(pressed);
        break;
    case SingleInput::BUTTON_B:
        controllers[id]->button_b->setChecked(pressed);
        break;
    case SingleInput::BUTTON_X:
        controllers[id]->button_x->setChecked(pressed);
        break;
    case SingleInput::BUTTON_Y:
        controllers[id]->button_y->setChecked(pressed);
        break;
    case SingleInput::BUTTON_BACK:
        controllers[id]->button_back->setChecked(pressed);
        break;
    case SingleInput::BUTTON_GUIDE:
        controllers[id]->button_guide->setChecked(pressed);
        break;
    case SingleInput::BUTTON_START:
        controllers[id]->button_start->setChecked(pressed);
        break;
    case SingleInput::BUTTON_LEFTSTICK:
        controllers[id]->button_ls->setChecked(pressed);
        break;
    case SingleInput::BUTTON_RIGHTSTICK:
        controllers[id]->button_rs->setChecked(pressed);
        break;
    case SingleInput::BUTTON_LEFTSHOULDER:
        controllers[id]->button_lb->setChecked(pressed);
        break;
    case SingleInput::BUTTON_RIGHTSHOULDER:
        controllers[id]->button_rb->setChecked(pressed);
        break;
    case SingleInput::BUTTON_DPAD_UP:
        controllers[id]->button_dpad_up->setChecked(pressed);
        break;
    case SingleInput::BUTTON_DPAD_DOWN:
        controllers[id]->button_dpad_down->setChecked(pressed);
        break;
    case SingleInput::BUTTON_DPAD_LEFT:
        controllers[id]->button_dpad_left->setChecked(pressed);
        break;
    case SingleInput::BUTTON_DPAD_RIGHT:
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
        ai.controller_axes[j][SingleInput::AXIS_LEFTX] = controllers[j]->axis_left->x_axis;
        ai.controller_axes[j][SingleInput::AXIS_LEFTY] = controllers[j]->axis_left->y_axis;
        ai.controller_axes[j][SingleInput::AXIS_RIGHTX] = controllers[j]->axis_right->x_axis;
        ai.controller_axes[j][SingleInput::AXIS_RIGHTY] = controllers[j]->axis_right->y_axis;
        ai.controller_axes[j][SingleInput::AXIS_TRIGGERLEFT] = static_cast<short>(controllers[j]->trigger_left_value->value());
        ai.controller_axes[j][SingleInput::AXIS_TRIGGERRIGHT] = static_cast<short>(controllers[j]->trigger_right_value->value());

        /* Set controller buttons */
        ai.controller_buttons[j] = 0;

        ai.controller_buttons[j] |= (controllers[j]->button_a->isChecked() << SingleInput::BUTTON_A);
        ai.controller_buttons[j] |= (controllers[j]->button_b->isChecked() << SingleInput::BUTTON_B);
        ai.controller_buttons[j] |= (controllers[j]->button_x->isChecked() << SingleInput::BUTTON_X);
        ai.controller_buttons[j] |= (controllers[j]->button_y->isChecked() << SingleInput::BUTTON_Y);
        ai.controller_buttons[j] |= (controllers[j]->button_back->isChecked() << SingleInput::BUTTON_BACK);
        ai.controller_buttons[j] |= (controllers[j]->button_guide->isChecked() << SingleInput::BUTTON_GUIDE);
        ai.controller_buttons[j] |= (controllers[j]->button_start->isChecked() << SingleInput::BUTTON_START);
        ai.controller_buttons[j] |= (controllers[j]->button_ls->isChecked() << SingleInput::BUTTON_LEFTSTICK);
        ai.controller_buttons[j] |= (controllers[j]->button_rs->isChecked() << SingleInput::BUTTON_RIGHTSTICK);
        ai.controller_buttons[j] |= (controllers[j]->button_lb->isChecked() << SingleInput::BUTTON_LEFTSHOULDER);
        ai.controller_buttons[j] |= (controllers[j]->button_rb->isChecked() << SingleInput::BUTTON_RIGHTSHOULDER);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_up->isChecked() << SingleInput::BUTTON_DPAD_UP);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_down->isChecked() << SingleInput::BUTTON_DPAD_DOWN);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_left->isChecked() << SingleInput::BUTTON_DPAD_LEFT);
        ai.controller_buttons[j] |= (controllers[j]->button_dpad_right->isChecked() << SingleInput::BUTTON_DPAD_RIGHT);
    }
}

void ControllerTabWindow::slotGetInputs(const AllInputs &ai)
{
    /* Don't grab inputs if the window is hidden */
    if (!isVisible())
        return;

    for (int j=0; j<AllInputs::MAXJOYS; j++) {
        /* Get controller axes */
        controllers[j]->axis_left_x->setValue(ai.controller_axes[j][SingleInput::AXIS_LEFTX]);
        controllers[j]->axis_left_y->setValue(ai.controller_axes[j][SingleInput::AXIS_LEFTY]);
        controllers[j]->axis_right_x->setValue(ai.controller_axes[j][SingleInput::AXIS_RIGHTX]);
        controllers[j]->axis_right_y->setValue(ai.controller_axes[j][SingleInput::AXIS_RIGHTY]);
        controllers[j]->trigger_left_value->setValue(ai.controller_axes[j][SingleInput::AXIS_TRIGGERLEFT]);
        controllers[j]->trigger_right_value->setValue(ai.controller_axes[j][SingleInput::AXIS_TRIGGERRIGHT]);

        /* Get controller buttons */
        for (int b = SingleInput::BUTTON_A; b < SingleInput::BUTTON_LAST; b++) {
            slotButtonToggle(j, b, ai.controller_buttons[j] & (1 << b));
        }
    }
}

void ControllerTabWindow::keyPressEvent(QKeyEvent *e)
{
    xcb_keysym_t mod = convertQtModifiers(e->modifiers());

    if (context->config.km.hotkey_mapping.find(e->nativeVirtualKey() | mod) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[e->nativeVirtualKey() | mod];
        context->hotkey_pressed_queue.push(hk.type);
        return;
    }
    if (context->config.km.hotkey_mapping.find(e->nativeVirtualKey()) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[e->nativeVirtualKey()];
        context->hotkey_pressed_queue.push(hk.type);
        return;
    }
    if (context->config.km.input_mapping.find(e->nativeVirtualKey()) != context->config.km.input_mapping.end()) {
        SingleInput si = context->config.km.input_mapping[e->nativeVirtualKey()];
        if (si.inputTypeIsController())
            return slotButtonToggle(si.inputTypeToControllerNumber(), si.inputTypeToInputNumber(), true);
    }

    QWidget::keyPressEvent(e);
}

void ControllerTabWindow::keyReleaseEvent(QKeyEvent *e)
{
    xcb_keysym_t mod = convertQtModifiers(e->modifiers());

    if (context->config.km.hotkey_mapping.find(e->nativeVirtualKey() | mod) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[e->nativeVirtualKey() | mod];
        context->hotkey_released_queue.push(hk.type);
        return;
    }
    if (context->config.km.hotkey_mapping.find(e->nativeVirtualKey()) != context->config.km.hotkey_mapping.end()) {
        HotKey hk = context->config.km.hotkey_mapping[e->nativeVirtualKey()];
        context->hotkey_released_queue.push(hk.type);
        return;
    }
    if (context->config.km.input_mapping.find(e->nativeVirtualKey()) != context->config.km.input_mapping.end()) {
        SingleInput si = context->config.km.input_mapping[e->nativeVirtualKey()];
        if (si.inputTypeIsController())
            return slotButtonToggle(si.inputTypeToControllerNumber(), si.inputTypeToInputNumber(), false);
    }

    QWidget::keyReleaseEvent(e);
}
