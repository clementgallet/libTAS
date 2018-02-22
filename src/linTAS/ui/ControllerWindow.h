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

#ifndef LINTAS_CONTROLLERWINDOW_H_INCLUDED
#define LINTAS_CONTROLLERWINDOW_H_INCLUDED

#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>

// #include <FL/Fl.H>
// #include <FL/Fl_Double_Window.H>
// // #include <FL/Fl_Button.H>
// //#include <FL/Fl_Input.H>
// #include <FL/Fl_Value_Input.H>
// // #include <FL/Fl_Output.H>
// #include <FL/Fl_Check_Button.H>
// #include <FL/Fl_Hor_Slider.H>

#include "ControllerAxisWidget.h"
#include "../Context.h"

class ControllerWindow : public QDialog {
    Q_OBJECT

public:
    // ControllerWindow(Context *c);
    ControllerWindow(Context *c, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = 0);

    /* Update UI elements when the config has changed */
    // void update_config();

    Context *context;

    QCheckBox *button_a;
    QCheckBox *button_b;
    QCheckBox *button_x;
    QCheckBox *button_y;

    QCheckBox *button_dpad_left;
    QCheckBox *button_dpad_right;
    QCheckBox *button_dpad_up;
    QCheckBox *button_dpad_down;

    QCheckBox *button_rb;
    QCheckBox *button_lb;
    QCheckBox *button_rs;
    QCheckBox *button_ls;

    ControllerAxisWidget *axis_left;
    ControllerAxisWidget *axis_right;

    QSpinBox *axis_left_x;
    QSpinBox *axis_left_y;
    QSpinBox *axis_right_x;
    QSpinBox *axis_right_y;

    QCheckBox *button_back;
    QCheckBox *button_guide;
    QCheckBox *button_start;

    QSlider *trigger_right;
    QSlider *trigger_left;

    QSpinBox *trigger_right_value;
    QSpinBox *trigger_left_value;
};

#endif
