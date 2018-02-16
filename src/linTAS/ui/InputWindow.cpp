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

#include <QStringList>
#include <QTableWidgetItem>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "InputWindow.h"
#include <iostream>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/Xlib.h> // XKeysymToString

static xcb_keysym_t get_next_keypressed(xcb_connection_t* conn, xcb_window_t window, bool with_modifiers);

InputWindow::InputWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setFixedSize(800, 500);
    setWindowTitle("Input mapping");

    /* Tables */
    hotkeyTable = new QTableWidget(context->config.km.hotkey_list.size(), 2, this);
    QStringList hotkeyHeader;
    hotkeyHeader << "Hotkey" << "Mapping";
    hotkeyTable->setHorizontalHeaderLabels(hotkeyHeader);
    connect(hotkeyTable, &QTableWidget::itemSelectionChanged, this, &InputWindow::slotSelect);

    inputTable = new QTableWidget(context->config.km.input_list.size(), 2, this);
    QStringList inputHeader;
    inputHeader << "Hotkey" << "Mapping";
    inputTable->setHorizontalHeaderLabels(inputHeader);
    connect(inputTable, &QTableWidget::itemSelectionChanged, this, &InputWindow::slotSelect);

    /* Fill hotkey list */
    int r = 0;
    for (auto iter : context->config.km.hotkey_list) {
        hotkeyTable->setItem(r, 0, new QTableWidgetItem(iter.description.c_str()));
        hotkeyTable->setItem(r++, 1, new QTableWidgetItem());
    }

    /* Fill input list */
    r = 0;
    for (auto iter : context->config.km.input_list) {
        inputTable->setItem(r, 0, new QTableWidgetItem(iter.description.c_str()));
        inputTable->setItem(r++, 1, new QTableWidgetItem());
    }

    update();

    assignButton = new QPushButton("Assign");
    assignButton->setEnabled(false);
    connect(assignButton, &QAbstractButton::clicked, this, &InputWindow::slotAssign);

    defaultButton = new QPushButton("Default");
    defaultButton->setEnabled(false);
    connect(defaultButton, &QAbstractButton::clicked, this, &InputWindow::slotDefault);

    disableButton = new QPushButton("Disable");
    disableButton->setEnabled(false);
    connect(disableButton, &QAbstractButton::clicked, this, &InputWindow::slotDisable);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttonBox->addButton(assignButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(defaultButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(disableButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &InputWindow::slotSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &InputWindow::reject);

    /* Layouts */
    QHBoxLayout *tableLayout = new QHBoxLayout;
    tableLayout->addWidget(hotkeyTable);
    tableLayout->addWidget(inputTable);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addLayout(tableLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void InputWindow::updateHotkeyRow(int row)
{
    /* Check if a key is mapped to this hotkey */
    for (auto itermap : context->config.km.hotkey_mapping) {
        if (itermap.second == context->config.km.hotkey_list[row]) {
            /* Build the key string with modifiers */
            QString str;
            xcb_keysym_t ks = itermap.first;
            for (ModifierKey modifier : modifier_list) {
                if (ks & modifier.flag) {
                    str += modifier.description.c_str();
                    str += "+";
                    ks ^= modifier.flag;
                }
            }

            str += XKeysymToString(ks);
            hotkeyTable->item(row, 1)->setText(str);
            return;
        }
    }

    hotkeyTable->item(row, 1)->setText("");
}

void InputWindow::updateInputRow(int row)
{
    SingleInput si = context->config.km.input_list[row];

    /* Check if a key is mapped to this input */
    for (auto itermap : context->config.km.input_mapping) {
        if (itermap.second == si) {
            QString str;
            /* Special case for visibility:
             * if mapped to itself print <self> */
            if ((si.type == IT_KEYBOARD) && (si.value == itermap.first))
                str += "<self>";
            else
                str += XKeysymToString(itermap.first);

            inputTable->item(row, 1)->setText(str);
            return;
        }
    }

    inputTable->item(row, 1)->setText("");
}

void InputWindow::update()
{
    /* Update hotkey list */
    for (int row = 0; row < context->config.km.hotkey_list.size(); row++) {
        updateHotkeyRow(row);
    }

    /* Update input list */
    for (int row = 0; row < context->config.km.input_list.size(); row++) {
        updateInputRow(row);
    }
}

static xcb_keysym_t get_next_keypressed(xcb_connection_t* conn, xcb_window_t window, bool with_modifiers)
{
    /* Grab keyboard */
    xcb_grab_keyboard_cookie_t grab_keyboard_cookie = xcb_grab_keyboard(conn, true, window, XCB_CURRENT_TIME, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    std::unique_ptr<xcb_grab_keyboard_reply_t> grab_keyboard_reply(xcb_grab_keyboard_reply(conn, grab_keyboard_cookie, nullptr));

    if (grab_keyboard_reply && grab_keyboard_reply->status != XCB_GRAB_STATUS_SUCCESS)
        std::cerr << "Could not grab keyboard" << std::endl;

    xcb_generic_error_t* error;
    xcb_key_symbols_t *keysyms;
    if (!(keysyms = xcb_key_symbols_alloc(conn))) {
        std::cerr << "Could not allocate key symbols" << std::endl;
        return 0;
    }

    while (1) {
        std::unique_ptr<xcb_key_press_event_t> key_event (reinterpret_cast<xcb_key_press_event_t*>(xcb_wait_for_event (conn)));
        if ((key_event->response_type & ~0x80) == XCB_KEY_PRESS)
        {
            xcb_keycode_t kc = key_event->detail;
            xcb_keysym_t ks = xcb_key_symbols_get_keysym(keysyms, kc, 0);

            if (with_modifiers) {
                if (is_modifier(ks)) {
                    continue;
                }

                /* Get keyboard inputs */
                xcb_query_keymap_cookie_t keymap_cookie = xcb_query_keymap(conn);
                std::unique_ptr<xcb_query_keymap_reply_t> keymap_reply(xcb_query_keymap_reply(conn, keymap_cookie, &error));

                if (error) {
                    std::cerr << "Could not get keymap, X error" << error->error_code << std::endl;
                    return 0;
                }

                xcb_keysym_t modifiers = build_modifiers(keymap_reply->keys, conn);
                ks |= modifiers;
            }

            xcb_key_symbols_free(keysyms);

            /* Ungrab keyboard */
            xcb_ungrab_keyboard(conn, XCB_CURRENT_TIME);

            // xcb_disconnect(new_conn);
            return ks;
        }
    }
    return 0;
}

void InputWindow::slotSelect()
{
    QTableWidget* table = static_cast<QTableWidget*>(sender());

    /* Deselect the other browser */
    if (table == hotkeyTable) {
        inputTable->clearSelection();
    }
    else {
        hotkeyTable->clearSelection();
    }

    /* Count how many lines are selected */

    int count = table->selectedItems().count();

    /* Enable/disable the assign button */
    if (count >= 1) {
        if (count == 1) {
            assignButton->setEnabled(true);
        }
        else {
            assignButton->setEnabled(false);
        }
        defaultButton->setEnabled(true);
        disableButton->setEnabled(true);
    }
    else {
        assignButton->setEnabled(false);
        defaultButton->setEnabled(false);
        disableButton->setEnabled(false);
    }

    /* Check if we had a double-click with a single line selected, and
     * trigger the assignment if so */

    // if ((count == 1) && (Fl::event_clicks() != 0)) {
    //     Fl::event_clicks(0); // Prevent from triggering this again
    //     iw->assign_button->do_callback(iw->assign_button, v);
    // }
}

void InputWindow::slotAssign()
{
    /* We need to ignore calls that arrive just after another one, this means
     * that the user queued multiple events that trigger this callback.
     * Unfortunately, I didn't manage to ignore or delete mouse events, either
     * in FLTK or Xlib level, so this is a fallback, using system time... sorry.
     */
    // static timespec lastassign = {0, 0};
    // timespec thisassign;
    // clock_gettime(CLOCK_MONOTONIC, &thisassign);
    //
    // if ( ((thisassign.tv_sec - lastassign.tv_sec) <= 1) && // This first check is necessary for 32-bit arch
    //     (1000L*1000L*1000L*(thisassign.tv_sec - lastassign.tv_sec) +
    //     thisassign.tv_nsec - lastassign.tv_nsec) < 10L*1000L*1000L )
    //     return;

    /* Check if the selected item is in the hotkey browser.
     * We cannot use value() function only, it is supposed to return 0 if
     * no item is selected, but after calling deselect(), it actually returns
     * the index of the last element. So we check if this element is selected.
     */
    // int sel_hotkey = hotkey_browser->value();
    // int sel_input = input_browser->value();
    //
    // bool is_hotkey = iw->hotkey_browser->selected(sel_hotkey);

    QList<QTableWidgetItem*> hotkeySelList = hotkeyTable->selectedItems();
    QList<QTableWidgetItem*> inputSelList = inputTable->selectedItems();

    if (hotkeySelList.count() + inputSelList.count() != 1)
        return;

    QTableWidget *selTable;
    bool isHotkey = (hotkeySelList.count() == 1);
    if (isHotkey) {
        selTable = hotkeyTable;
    }
    else {
        selTable = inputTable;
    }

    int row = selTable->row(selTable->selectedItems().first());

    assignButton->setText(tr("Press key..."));
    assignButton->setEnabled(false);

    selTable->item(row, 1)->setText(tr("<press key>"));

    // Fl::flush();
    /* TODO: replace the following function with an implementation of
     * keyPressEvent(QKeyEvent *ev), then access ev->nativeVirtualKey() and
     * ev->modifiers(). We need some synchronisation to recover the keysym here.
     */
    xcb_keysym_t ks = get_next_keypressed(context->conn, winId(), isHotkey);

    assignButton->setText(tr("Assign"));
    assignButton->setEnabled(true);

    if (isHotkey) {
        context->config.km.reassign_hotkey(row, ks);
    }
    else {
        context->config.km.reassign_input(row, ks);
    }

    // clock_gettime(CLOCK_MONOTONIC, &lastassign);
    update();
}

void InputWindow::slotDefault()
{
    QList<QTableWidgetItem*> hotkeySelList = hotkeyTable->selectedItems();
    for (auto item : hotkeySelList) {
        int row = hotkeyTable->row(item);
        context->config.km.default_hotkey(row);
        updateHotkeyRow(row);
    }

    QList<QTableWidgetItem*> inputSelList = inputTable->selectedItems();
    for (auto item : inputSelList) {
        int row = inputTable->row(item);
        context->config.km.default_input(row);
        updateInputRow(row);
    }
}

void InputWindow::slotDisable()
{
    QList<QTableWidgetItem*> hotkeySelList = hotkeyTable->selectedItems();
    for (auto item : hotkeySelList) {
        int row = hotkeyTable->row(item);
        context->config.km.reassign_hotkey(row, 0);
        updateHotkeyRow(row);
    }

    QList<QTableWidgetItem*> inputSelList = inputTable->selectedItems();
    for (auto item : inputSelList) {
        int row = inputTable->row(item);
        context->config.km.reassign_input(row, 0);
        updateInputRow(row);
    }
}

void InputWindow::slotSave()
{
    /* TODO: Save mappings */

    /* Close window */
    accept();
}
