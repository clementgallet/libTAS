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

#include "InputWindow.h"
//#include "../main.h"
#include <iostream>
#include <X11/XKBlib.h>

InputWindow::InputWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(800, 500);

    /* We need to open a connection to the X server to access to key pressed */
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        /* TODO: Deal with the error */
        std::cerr << "Cannot open display" << std::endl;
    }

    /* Browsers */
    hotkey_browser = new Fl_Multi_Browser(10, 10, 350, 400, "Hotkeys");
    hotkey_browser->callback(select_cb, this);

    input_browser = new Fl_Multi_Browser(400, 10, 350, 400, "Inputs");
    input_browser->callback(select_cb, this);

    /* Set two columns */
    static int col_width[] = {220, 130, 0};
    hotkey_browser->column_widths(col_width);
    hotkey_browser->column_char('\t');
    input_browser->column_widths(col_width);
    input_browser->column_char('\t');

    /* Fill hotkey list */
    for (auto iter : context->config.km.hotkey_list) {
        std::string linestr(iter.description);
        /* Add the line in the browser */
        hotkey_browser->add(linestr.c_str());
    }

    /* Fill input list */
    for (auto iter : context->config.km.input_list) {
        std::string linestr(iter.description);
        /* Add the line in the browser */
        input_browser->add(linestr.c_str());
    }

    update();

    assign_button = new Fl_Button(250, 420, 90, 30, "Assign");
    assign_button->callback(assign_cb, this);
    assign_button->deactivate();

    default_button = new Fl_Button(350, 420, 90, 30, "Default");
    default_button->callback(default_cb, this);
    default_button->deactivate();

    disable_button = new Fl_Button(450, 420, 90, 30, "Disable");
    disable_button->callback(disable_cb, this);
    disable_button->deactivate();

    save_button = new Fl_Button(600, 460, 70, 30, "Ok");
    save_button->callback(save_cb, this);

    cancel_button = new Fl_Button(700, 460, 70, 30, "Cancel");
    cancel_button->callback(cancel_cb, this);

    window->end();
}

void InputWindow::update()
{
    /* Update hotkey list */
    int index = 1;
    for (auto iter : context->config.km.hotkey_list) {
        std::string linestr(iter.description);

        /* Check if a key is mapped to this hotkey */
        for (auto itermap : context->config.km.hotkey_mapping) {
            if (itermap.second == iter) {
                linestr += '\t';
                linestr += XKeysymToString(itermap.first);
                break;
            }
        }

        /* Modify the text in the browser */
        hotkey_browser->text(index, linestr.c_str());
        index++;
    }

    /* Update input list */
    index = 1;
    for (auto iter : context->config.km.input_list) {
        std::string linestr(iter.description);

        /* Check if a key is mapped to this input */
        for (auto itermap : context->config.km.input_mapping) {
            if (itermap.second == iter) {
                linestr += '\t';
                /* Special case for visibility:
                 * if mapped to itself print <self> */
                if ((iter.type == IT_KEYBOARD) && (iter.value == itermap.first))
                    linestr += "<self>";
                else
                    linestr += XKeysymToString(itermap.first);
                break;
            }
        }

        /* Modify the text in the browser */
        input_browser->text(index, linestr.c_str());
        index++;
    }
}

KeySym get_next_keypressed(Display* display)
{
    Window window;
    XEvent event;
    int revert;
    XGetInputFocus(display, &window, &revert);
    XSelectInput(display, window, KeyPressMask | KeyReleaseMask);

    /* Empty event queue */
    while (XPending (display)) {
        XNextEvent(display, &event);
    }

    while (1) {
        XNextEvent(display, &event);
        if (event.type == KeyPress)
        {
            //ui_print("KeyPress event");
            KeyCode kc = event.xkey.keycode;
            //ui_print("KeyCode is %d", kc);
            KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);
            return ks;
        }
    }
    return 0;
}

void select_cb(Fl_Widget* w, void* v)
{
    InputWindow* iw = (InputWindow*) v;
    Fl_Multi_Browser* cur_browser;

    /* Deselect the other browser */
    if (iw->input_browser->changed()) {
        iw->hotkey_browser->deselect();
        cur_browser = iw->input_browser;
    }
    if (iw->hotkey_browser->changed()) {
        iw->input_browser->deselect();
        cur_browser = iw->hotkey_browser;
    }

    /* Count how many lines are selected */
    int count = 0;
    for (int i = 1; i <= cur_browser->size(); i++) {
        count += cur_browser->selected(i);
    }

    /* Enable/disable the assign button */
    if (count >= 1) {
        if (count == 1) {
            iw->assign_button->activate();
        }
        else {
            iw->assign_button->deactivate();
        }
        iw->default_button->activate();
        iw->disable_button->activate();
    }
    else {
        iw->assign_button->deactivate();
        iw->default_button->deactivate();
        iw->disable_button->deactivate();
    }
}

void assign_cb(Fl_Widget* w, void* v)
{
    InputWindow* iw = (InputWindow*) v;

    iw->assign_button->label("Press key...");
    iw->assign_button->deactivate();
    Fl::flush();
    KeySym ks = get_next_keypressed(iw->display);
    iw->assign_button->label("Assign");
    iw->assign_button->activate();

    /* Check if the selected item is in the hotkey browser.
     * We cannot use value() function only, it is supposed to return 0 if
     * no item is selected, but after calling deselect(), it actually returns
     * the index of the last element. So we check if this element is selected.
     */
    int sel_hotkey = iw->hotkey_browser->value();
    if (iw->hotkey_browser->selected(sel_hotkey)) {
        iw->context->config.km.reassign_hotkey(sel_hotkey-1, ks);
    }

    int sel_input = iw->input_browser->value();
    if (iw->input_browser->selected(sel_input)) {
        iw->context->config.km.reassign_input(sel_input-1, ks);
    }

    iw->update();
}

void default_cb(Fl_Widget* w, void* v)
{
    InputWindow* iw = (InputWindow*) v;

    /* Check if the selected item is in the hotkey browser.
     * We cannot use value() function only, it is supposed to return 0 if
     * no item is selected, but after calling deselect(), it actually returns
     * the index of the last element. So we check if this element is selected.
     */
    int sel_hotkey = iw->hotkey_browser->value();
    if (iw->hotkey_browser->selected(sel_hotkey)) {
        for (int i = 1; i <= iw->hotkey_browser->size(); i++) {
            if (iw->hotkey_browser->selected(i)) {
                iw->context->config.km.default_hotkey(i-1);
            }
        }
    }

    int sel_input = iw->input_browser->value();
    if (iw->input_browser->selected(sel_input)) {
        for (int i = 1; i <= iw->input_browser->size(); i++) {
            if (iw->input_browser->selected(i)) {
                iw->context->config.km.default_input(i-1);
            }
        }
    }

    iw->update();
}

void disable_cb(Fl_Widget* w, void* v)
{
    InputWindow* iw = (InputWindow*) v;

    /* Check if the selected item is in the hotkey browser.
     * We cannot use value() function only, it is supposed to return 0 if
     * no item is selected, but after calling deselect(), it actually returns
     * the index of the last element. So we check if this element is selected.
     */
    int sel_hotkey = iw->hotkey_browser->value();
    if (iw->hotkey_browser->selected(sel_hotkey)) {
        for (int i = 1; i <= iw->hotkey_browser->size(); i++) {
            if (iw->hotkey_browser->selected(i)) {
                iw->context->config.km.reassign_hotkey(i-1, 0);
            }
        }
    }

    int sel_input = iw->input_browser->value();
    if (iw->input_browser->selected(sel_input)) {
        for (int i = 1; i <= iw->input_browser->size(); i++) {
            if (iw->input_browser->selected(i)) {
                iw->context->config.km.reassign_input(i-1, 0);
            }
        }
    }

    iw->update();
}

void save_cb(Fl_Widget* w, void* v)
{
    InputWindow* iw = (InputWindow*) v;

    /* TODO: Save mappings */

    /* Close window */
    iw->window->hide();
}

void cancel_cb(Fl_Widget* w, void* v)
{
    InputWindow* iw = (InputWindow*) v;

    /* Close window */
    iw->window->hide();
}
