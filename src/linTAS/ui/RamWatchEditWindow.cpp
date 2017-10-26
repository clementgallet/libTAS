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

#include "RamWatchEditWindow.h"
#include "../ramsearch/RamWatchDetailed.h"

#include <sstream>

static Fl_Callback save_cb;
static Fl_Callback cancel_cb;

RamWatchEditWindow::RamWatchEditWindow()
{
    window = new Fl_Double_Window(350, 300, "Edit Watch");

    edit_pack = new Fl_Pack(90, 10, 250, 280, "");
    edit_pack->type(Fl_Pack::VERTICAL);
    edit_pack->spacing(20);

    address_input = new Fl_Int_Input(0, 0, 180, 30, "Address: ");
    address_input->align(FL_ALIGN_LEFT);

    label_input = new Fl_Input(0, 0, 180, 30, "Label: ");
    label_input->align(FL_ALIGN_LEFT);

    type_choice = new Fl_Choice(0, 0, 180, 30, "Type: ");
    type_choice->align(FL_ALIGN_LEFT);
    type_choice->menu(type_items);

    display_choice = new Fl_Choice(0, 0, 180, 30, "Display: ");
    display_choice->align(FL_ALIGN_LEFT);
    display_choice->menu(display_items);

    button_pack = new Fl_Pack(0, 0, 330, 30, "");
    button_pack->type(Fl_Pack::HORIZONTAL);
    button_pack->spacing(30);

    save_button = new Fl_Button(0, 0, 80, 30, "Save");
    save_button->callback(save_cb, this);

    cancel_button = new Fl_Button(0, 0, 80, 30, "Cancel");
    cancel_button->callback(cancel_cb, this);

    button_pack->end();
    edit_pack->end();

    window->end();
}

Fl_Menu_Item RamWatchEditWindow::type_items[] = {
    {"unsigned char"},
    {"char"},
    {"unsigned short"},
    {"short"},
    {"unsigned int"},
    {"int"},
    {"unsigned int64"},
    {"int64"},
    {"float"},
    {"double"},
    {nullptr}
};

Fl_Menu_Item RamWatchEditWindow::display_items[] = {
    {"Decimal"},
    {"Hexadecimal"},
    {nullptr}
};

void RamWatchEditWindow::fill(std::unique_ptr<IRamWatchDetailed> &watch)
{
    /* Fill address */
    std::ostringstream oss;
    oss << std::hex << watch->address;
    address_input->value(oss.str().c_str());

    /* Fill label */
    label_input->value(watch->label.c_str());

    /* Fill display */
    if (watch->hex)
        display_choice->value(1);
    else
        display_choice->value(0);
}

void RamWatchEditWindow::fill(std::unique_ptr<IRamWatch> &watch)
{
    /* Fill address */
    std::ostringstream oss;
    oss << std::hex << watch->address;
    address_input->value(oss.str().c_str());
}

static void save_cb(Fl_Widget* w, void* v)
{
    RamWatchEditWindow* rwew = static_cast<RamWatchEditWindow*>(v);

    uintptr_t addr = strtoul(rwew->address_input->value(), nullptr, 16);

    /* Build the ram watch using the right type as template */
    switch (rwew->type_choice->value()) {
        case 0:
            rwew->ramwatch.reset(new RamWatchDetailed<unsigned char>(addr));
            break;
        case 1:
            rwew->ramwatch.reset(new RamWatchDetailed<char>(addr));
            break;
        case 2:
            rwew->ramwatch.reset(new RamWatchDetailed<unsigned short>(addr));
            break;
        case 3:
            rwew->ramwatch.reset(new RamWatchDetailed<short>(addr));
            break;
        case 4:
            rwew->ramwatch.reset(new RamWatchDetailed<unsigned int>(addr));
            break;
        case 5:
            rwew->ramwatch.reset(new RamWatchDetailed<int>(addr));
            break;
        case 6:
            rwew->ramwatch.reset(new RamWatchDetailed<uint64_t>(addr));
            break;
        case 7:
            rwew->ramwatch.reset(new RamWatchDetailed<int64_t>(addr));
            break;
        case 8:
            rwew->ramwatch.reset(new RamWatchDetailed<float>(addr));
            break;
        case 9:
            rwew->ramwatch.reset(new RamWatchDetailed<double>(addr));
            break;
    }

    rwew->ramwatch->hex = (rwew->display_choice->value() == 1);
    rwew->ramwatch->label = rwew->label_input->value();

    rwew->window->hide();
}

static void cancel_cb(Fl_Widget* w, void* v)
{
    RamWatchEditWindow* rwew = static_cast<RamWatchEditWindow*>(v);
    rwew->ramwatch.reset(nullptr);
    rwew->window->hide();
}
