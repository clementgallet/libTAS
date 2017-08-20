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

#include "ControllerWindow.h"
#include "ControllerAxisWidget.h"
// #include "MainWindow.h"
// #include <iostream>

static Fl_Callback axis_value_update_cb;
static Fl_Callback axis_update_cb;

ControllerWindow::ControllerWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(600, 200);

    button_a = new Fl_Check_Button(500, 170, 70, 30, "A");
    button_b = new Fl_Check_Button(540, 140, 70, 30, "B");
    button_x = new Fl_Check_Button(460, 140, 70, 30, "X");
    button_y = new Fl_Check_Button(500, 110, 70, 30, "Y");

    button_dpad_down = new Fl_Check_Button(50, 170, 70, 30, "Down");
    button_dpad_right = new Fl_Check_Button(90, 140, 70, 30, "Right");
    button_dpad_left = new Fl_Check_Button(10, 140, 70, 30, "Left");
    button_dpad_up = new Fl_Check_Button(50, 110, 70, 30, "Up");

    button_rb = new Fl_Check_Button(500, 85, 70, 30, "RB");
    button_lb = new Fl_Check_Button(50, 85, 70, 30, "LB");
    button_rs = new Fl_Check_Button(355, 170, 70, 30, "RS");
    button_ls = new Fl_Check_Button(220, 170, 70, 30, "LS");

    axis_left = new ControllerAxisWidget(190, 80, 90, 90, "Left axis");
    axis_left->callback(axis_update_cb, this);
    axis_right = new ControllerAxisWidget(320, 80, 90, 90, "Right axis");
    axis_right->callback(axis_update_cb, this);

    axis_left_x = new Fl_Value_Input(170, 50, 60, 25, "X value");
    axis_left_x->align(FL_ALIGN_TOP_LEFT);
    axis_left_x->bounds(INT16_MIN, INT16_MAX);
    axis_left_x->step(1);
    axis_left_x->callback(axis_value_update_cb, this);
    axis_left_y = new Fl_Value_Input(240, 50, 60, 25, "Y value");
    axis_left_y->align(FL_ALIGN_TOP_LEFT);
    axis_left_y->bounds(INT16_MIN, INT16_MAX);
    axis_left_y->step(1);
    axis_left_y->callback(axis_value_update_cb, this);
    axis_right_x = new Fl_Value_Input(320, 50, 60, 25, "X value");
    axis_right_x->align(FL_ALIGN_TOP_LEFT);
    axis_right_x->bounds(INT16_MIN, INT16_MAX);
    axis_right_x->step(1);
    axis_right_x->callback(axis_value_update_cb, this);
    axis_right_y = new Fl_Value_Input(390, 50, 60, 25, "Y value");
    axis_right_y->align(FL_ALIGN_TOP_LEFT);
    axis_right_y->bounds(INT16_MIN, INT16_MAX);
    axis_right_y->step(1);
    axis_right_y->callback(axis_value_update_cb, this);

    button_back = new Fl_Check_Button(170, 5, 70, 30, "Back");
    button_guide = new Fl_Check_Button(280, 5, 70, 30, "Guide");
    button_start = new Fl_Check_Button(390, 5, 70, 30, "Start");

    trigger_right = new Fl_Hor_Slider(470, 60, 120, 25, "");
    trigger_right->bounds(INT16_MIN, INT16_MAX);
    trigger_right->step(1);
    trigger_right->callback(axis_update_cb, this);
    trigger_left = new Fl_Hor_Slider(10, 60, 120, 25, "");
    trigger_left->bounds(INT16_MIN, INT16_MAX);
    trigger_left->step(1);
    trigger_left->callback(axis_update_cb, this);

    trigger_right_value = new Fl_Value_Input(500, 25, 60, 25, "Right Trigger");
    trigger_right_value->align(FL_ALIGN_TOP_LEFT);
    trigger_right_value->bounds(INT16_MIN, INT16_MAX);
    trigger_right_value->step(1);
    trigger_right_value->callback(axis_value_update_cb, this);

    trigger_left_value = new Fl_Value_Input(10, 25, 60, 25, "Left Trigger");
    trigger_left_value->align(FL_ALIGN_TOP_LEFT);
    trigger_left_value->bounds(INT16_MIN, INT16_MAX);
    trigger_left_value->step(1);
    trigger_left_value->callback(axis_value_update_cb, this);


    // update_config();
    window->end();
}

void axis_value_update_cb(Fl_Widget* w, void* v)
{
    ControllerWindow* cw = (ControllerWindow*) v;

    /* Update each axis canvas */
    cw->axis_left->x_axis = static_cast<short>(cw->axis_left_x->value());
    cw->axis_left->y_axis = static_cast<short>(cw->axis_left_y->value());
    cw->axis_right->x_axis = static_cast<short>(cw->axis_right_x->value());
    cw->axis_right->y_axis = static_cast<short>(cw->axis_right_y->value());

    cw->axis_left->redraw();
    cw->axis_right->redraw();

    cw->trigger_right->value(cw->trigger_right_value->value());
    cw->trigger_left->value(cw->trigger_left_value->value());
}

void axis_update_cb(Fl_Widget* w, void* v)
{
    ControllerWindow* cw = (ControllerWindow*) v;

    /* Update each axis value */
    cw->axis_left_x->value(cw->axis_left->x_axis);
    cw->axis_left_y->value(cw->axis_left->y_axis);
    cw->axis_right_x->value(cw->axis_right->x_axis);
    cw->axis_right_y->value(cw->axis_right->y_axis);

    cw->trigger_right_value->value(cw->trigger_right->value());
    cw->trigger_left_value->value(cw->trigger_left->value());

}
