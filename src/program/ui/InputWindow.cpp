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

#include <QtCore/QStringList>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTabWidget>

#include "InputWindow.h"
#include "KeyPressedDialog.h"

InputWindow::InputWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Input mapping");

    keyDialog = new KeyPressedDialog(c, this);

    /* Tables */
    hotkeyTable = new QTableWidget(context->config.km->hotkey_list.size(), 2, this);
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

    for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
        inputTable[i] = new QTableWidget(context->config.km->input_list[i].size(), 2, Q_NULLPTR);
        QStringList inputHeader;
        inputHeader << "Hotkey" << "Mapping";
        inputTable[i]->setHorizontalHeaderLabels(inputHeader);
        inputTable[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
        inputTable[i]->setShowGrid(false);
        inputTable[i]->setAlternatingRowColors(true);
        inputTable[i]->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        inputTable[i]->horizontalHeader()->setHighlightSections(false);
        inputTable[i]->verticalHeader()->hide();
        inputTable[i]->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(inputTable[i], &QTableWidget::itemSelectionChanged, this, &InputWindow::slotSelect);
        connect(inputTable[i], &QTableWidget::cellDoubleClicked, this, &InputWindow::slotAssign);
    }

    QTabWidget* tabInputs = new QTabWidget(this);
    tabInputs->addTab(hotkeyTable, tr("Hotkeys"));
    tabInputs->addTab(inputTable[KeyMapping::INPUTLIST_KEYBOARD_LATIN], tr("Common keys"));
    tabInputs->addTab(inputTable[KeyMapping::INPUTLIST_KEYBOARD_MISC], tr("Misc keys"));
    tabInputs->addTab(inputTable[KeyMapping::INPUTLIST_FLAG], tr("Flags"));
    tabInputs->addTab(inputTable[KeyMapping::INPUTLIST_MOUSE], tr("Mouse"));
    tabInputs->addTab(inputTable[KeyMapping::INPUTLIST_CONTROLLER], tr("Controllers"));

    /* Fill hotkey list */
    int r = 0;
    for (auto iter : context->config.km->hotkey_list) {
        hotkeyTable->setItem(r, 0, new QTableWidgetItem(iter.description.c_str()));
        hotkeyTable->setItem(r++, 1, new QTableWidgetItem());
    }

    /* Fill input list */
    for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
        r = 0;
        for (auto iter : context->config.km->input_list[i]) {
            inputTable[i]->setItem(r, 0, new QTableWidgetItem(iter.description.c_str()));
            inputTable[i]->setItem(r++, 1, new QTableWidgetItem());
        }
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
    // QHBoxLayout *tableLayout = new QHBoxLayout;
    // tableLayout->addWidget(hotkeyTable);
    // tableLayout->addWidget(tabInputs);

    /* Create the main layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(tabInputs);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

QSize InputWindow::sizeHint() const
{
    return QSize(600, 700);
}

void InputWindow::updateHotkeyRow(int row)
{
    /* Check if a key is mapped to this hotkey */
    for (auto itermap : context->config.km->hotkey_mapping) {
        if (itermap.second == context->config.km->hotkey_list[row]) {
            /* Build the key string with modifiers */
            keysym_t ks = itermap.first;
            QString str = context->config.km->input_description_mod(ks).c_str();
            hotkeyTable->item(row, 1)->setText(str);
            return;
        }
    }

    hotkeyTable->item(row, 1)->setText("");
}

void InputWindow::updateInputRow(int tab, int row)
{
    SingleInput si = context->config.km->input_list[tab][row];

    /* Check if a key is mapped to this input */
    for (auto itermap : context->config.km->input_mapping) {
        if (itermap.second == si) {
            QString str;
            /* Special case for visibility:
             * if mapped to itself print <self> */
            if ((si.type == SingleInput::IT_KEYBOARD) && (si.value == itermap.first))
                str += "<self>";
            else
                str += context->config.km->input_description(itermap.first).c_str();

            inputTable[tab]->item(row, 1)->setText(str);
            return;
        }
    }

    inputTable[tab]->item(row, 1)->setText("");
}

void InputWindow::update()
{
    /* Update hotkey list */
    for (unsigned int row = 0; row < context->config.km->hotkey_list.size(); row++) {
        updateHotkeyRow(row);
    }

    /* Update input list */
    for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
        for (unsigned int row = 0; row < context->config.km->input_list[i].size(); row++) {
            updateInputRow(i, row);
        }
    }
}

void InputWindow::slotSelect()
{
    QTableWidget* table = static_cast<QTableWidget*>(sender());

    /* Deselect the other browser */
    if (table == hotkeyTable) {
        for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
            inputTable[i]->clearSelection();
        }
    }
    else {
        hotkeyTable->clearSelection();
        for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
            if (table != inputTable[i])
                inputTable[i]->clearSelection();
        }
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
    /* Get the table of the selected items */
    QTableWidget *selTable = nullptr;
    int tab = 0;
    
    if (hotkeyTable->selectionModel()->hasSelection())
        selTable = hotkeyTable;
    else {
        for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
            if (inputTable[i]->selectionModel()->hasSelection()) {
                selTable = inputTable[i];
                tab = i;
                break;
            }
        }
    }
    if (!selTable)
        return;
    
    int row = selTable->row(selTable->selectedItems().first());

    keyDialog->withModifiers = (selTable == hotkeyTable);
    keysym_t ks = keyDialog->exec();

    if (ks != 0) {
        if (selTable == hotkeyTable) {
            context->config.km->reassign_hotkey(row, ks);
        }
        else {
            context->config.km->reassign_input(tab, row, ks);
        }
    }

    update();
}

void InputWindow::slotDefault()
{
    QList<QModelIndex> hotkeySelList = hotkeyTable->selectionModel()->selectedRows();
    for (auto item : hotkeySelList) {
        context->config.km->default_hotkey(item.row());
    }

    for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
        QList<QModelIndex> inputSelList = inputTable[i]->selectionModel()->selectedRows();
        for (auto item : inputSelList) {
            context->config.km->default_input(i, item.row());
        }
    }

    update();
}

void InputWindow::slotDisable()
{
    QList<QModelIndex> hotkeySelList = hotkeyTable->selectionModel()->selectedRows();
    for (auto item : hotkeySelList) {
        context->config.km->reassign_hotkey(item.row(), 0);
        updateHotkeyRow(item.row());
    }

    for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
        QList<QModelIndex> inputSelList = inputTable[i]->selectionModel()->selectedRows();
        for (auto item : inputSelList) {
            context->config.km->reassign_input(i, item.row(), 0);
            updateInputRow(i, item.row());
        }
    }
}
