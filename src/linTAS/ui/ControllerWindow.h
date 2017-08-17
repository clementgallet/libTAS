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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
// #include <FL/Fl_Button.H>
//#include <FL/Fl_Input.H>
#include <FL/Fl_Value_Input.H>
// #include <FL/Fl_Output.H>
#include <Fl/Fl_Check_Button.H>
#include <Fl/Fl_Hor_Slider.H>

#include "../Context.h"
#include "ControllerAxisWidget.h"

class ControllerWindow {
    public:
        ControllerWindow(Context *c);

        /* Update UI elements when the config has changed */
        // void update_config();

        Context *context;

        Fl_Double_Window *window;

        Fl_Check_Button *button_a;
        Fl_Check_Button *button_b;
        Fl_Check_Button *button_x;
        Fl_Check_Button *button_y;

        Fl_Check_Button *button_dpad_left;
        Fl_Check_Button *button_dpad_right;
        Fl_Check_Button *button_dpad_up;
        Fl_Check_Button *button_dpad_down;

        Fl_Check_Button *button_rb;
        Fl_Check_Button *button_lb;
        Fl_Check_Button *button_rs;
        Fl_Check_Button *button_ls;

        ControllerAxisWidget *axis_left;
        ControllerAxisWidget *axis_right;

        Fl_Value_Input *axis_left_x;
        Fl_Value_Input *axis_left_y;
        Fl_Value_Input *axis_right_x;
        Fl_Value_Input *axis_right_y;

        Fl_Check_Button *button_back;
        Fl_Check_Button *button_guide;
        Fl_Check_Button *button_start;

        Fl_Hor_Slider *trigger_right;
        Fl_Hor_Slider *trigger_left;

        Fl_Value_Input *trigger_right_value;
        Fl_Value_Input *trigger_left_value;

};

#endif
