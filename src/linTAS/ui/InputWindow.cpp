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

#include <QStringList>
#include <QTableWidgetItem>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
// #include <X11/Xlib.h> // XKeysymToString

#include "InputWindow.h"

InputWindow::InputWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Input mapping");

    keyDialog = new KeyPressedDialog(this);

    /* Tables */
    hotkeyTable = new QTableWidget(context->config.km.hotkey_list.size(), 2, this);
    QStringList hotkeyHeader;
    hotkeyHeader << "Hotkey" << "Mapping";
    hotkeyTable->setHorizontalHeaderLabels(hotkeyHeader);
    hotkeyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    hotkeyTable->setShowGrid(false);
    hotkeyTable->setAlternatingRowColors(true);
    hotkeyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    hotkeyTable->horizontalHeader()->setHighlightSections(false);
    hotkeyTable->verticalHeader()->hide();
    hotkeyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(hotkeyTable, &QTableWidget::itemSelectionChanged, this, &InputWindow::slotSelect);
    connect(hotkeyTable, &QTableWidget::cellDoubleClicked, this, &InputWindow::slotAssign);

    inputTable = new QTableWidget(context->config.km.input_list.size(), 2, this);
    QStringList inputHeader;
    inputHeader << "Hotkey" << "Mapping";
    inputTable->setHorizontalHeaderLabels(inputHeader);
    inputTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputTable->setShowGrid(false);
    inputTable->setAlternatingRowColors(true);
    inputTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    inputTable->horizontalHeader()->setHighlightSections(false);
    inputTable->verticalHeader()->hide();
    inputTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(inputTable, &QTableWidget::itemSelectionChanged, this, &InputWindow::slotSelect);
    connect(inputTable, &QTableWidget::cellDoubleClicked, this, &InputWindow::slotAssign);

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

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->addButton(assignButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(defaultButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(disableButton, QDialogButtonBox::ActionRole);

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

            str += context->config.km.input_description(ks).c_str();
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
            if ((si.type == SingleInput::IT_KEYBOARD) && (si.value == itermap.first))
                str += "<self>";
            else
                str += context->config.km.input_description(itermap.first).c_str();

            inputTable->item(row, 1)->setText(str);
            return;
        }
    }

    inputTable->item(row, 1)->setText("");
}

void InputWindow::update()
{
    /* Update hotkey list */
    for (unsigned int row = 0; row < context->config.km.hotkey_list.size(); row++) {
        updateHotkeyRow(row);
    }

    /* Update input list */
    for (unsigned int row = 0; row < context->config.km.input_list.size(); row++) {
        updateInputRow(row);
    }
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
    int count = table->selectionModel()->selectedRows().count();

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
}

void InputWindow::slotAssign()
{

    QList<QModelIndex> hotkeySelList = hotkeyTable->selectionModel()->selectedRows();
    QList<QModelIndex> inputSelList = inputTable->selectionModel()->selectedRows();

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

    keyDialog->withModifiers = isHotkey;
    xcb_keysym_t ks = keyDialog->exec();

    if (ks != 0) {
        if (isHotkey) {
            context->config.km.reassign_hotkey(row, ks);
        }
        else {
            context->config.km.reassign_input(row, ks);
        }
    }

    update();
}

void InputWindow::slotDefault()
{
    QList<QModelIndex> hotkeySelList = hotkeyTable->selectionModel()->selectedRows();
    for (auto item : hotkeySelList) {
        context->config.km.default_hotkey(item.row());
    }

    QList<QModelIndex> inputSelList = inputTable->selectionModel()->selectedRows();
    for (auto item : inputSelList) {
        context->config.km.default_input(item.row());
    }

    update();
}

void InputWindow::slotDisable()
{
    QList<QModelIndex> hotkeySelList = hotkeyTable->selectionModel()->selectedRows();
    for (auto item : hotkeySelList) {
        context->config.km.reassign_hotkey(item.row(), 0);
        updateHotkeyRow(item.row());
    }

    QList<QModelIndex> inputSelList = inputTable->selectionModel()->selectedRows();
    for (auto item : inputSelList) {
        context->config.km.reassign_input(item.row(), 0);
        updateInputRow(item.row());
    }
}
