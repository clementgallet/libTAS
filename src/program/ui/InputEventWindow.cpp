/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "InputEventWindow.h"
#include "InputEventModel.h"
#include "ComboBoxItemDelegate.h"

#include "Context.h"
//#include "../shared/inputs/AllInputs.h"
#include "movie/MovieFile.h"

#include <QtWidgets/QTableView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>

InputEventWindow::InputEventWindow(Context* c, MovieFile *m, QWidget *parent) : QDialog(parent), context(c), movie(m)
{
    setWindowTitle("Edit Frame Events");

    /* Table */
    inputEventView = new QTableView(this);
    inputEventView->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputEventView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    inputEventView->setShowGrid(false);
    inputEventView->setAlternatingRowColors(true);
    inputEventView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    inputEventView->horizontalHeader()->setHighlightSections(false);
    inputEventView->verticalHeader()->setDefaultSectionSize(inputEventView->verticalHeader()->minimumSectionSize());
    inputEventView->verticalHeader()->hide();

    inputEventModel = new InputEventModel(context);
    inputEventView->setModel(inputEventModel);

    ComboBoxItemDelegate* cbid = new ComboBoxItemDelegate(movie, inputEventView);
    inputEventView->setItemDelegateForColumn(0, cbid);
    inputEventView->setItemDelegateForColumn(1, cbid);

    connect(inputEventView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &InputEventWindow::updateSelection);

    /* Buttons */
    // QPushButton *recordButton = new QPushButton(tr("Record Inputs"));
    // connect(recordButton, &QAbstractButton::clicked, this, &InputEventWindow::slotRecord);
    // 
    newButton = new QPushButton(tr("New Input"));
    connect(newButton, &QAbstractButton::clicked, this, &InputEventWindow::slotNew);
    
    QPushButton *removeButton = new QPushButton(tr("Remove Input"));
    connect(removeButton, &QAbstractButton::clicked, this, &InputEventWindow::slotRemove);

    QPushButton *clearButton = new QPushButton(tr("Clear Inputs"));
    connect(clearButton, &QAbstractButton::clicked, this, &InputEventWindow::slotClear);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    // buttonBox->addButton(recordButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(removeButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &InputEventWindow::slotSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &InputEventWindow::reject);

    /* Layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(inputEventView);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
}

void InputEventWindow::setFrame(uint64_t f)
{
    frame = f;
    setWindowTitle(QString("Edit Frame %1 Events").arg(frame));
    
    /* Give to the model the current state of movie events for the specified frame */
    inputEventModel->setEvents(movie->inputs->getInputs(frame).events);
}

void InputEventWindow::updateSelection()
{
    const QModelIndexList indexes = inputEventView->selectionModel()->selectedRows();
    if (indexes.count() == 0)
        newButton->setText(tr("New Input"));
    else
        newButton->setText(tr("Duplicate Inputs"));
}

void InputEventWindow::slotSave()
{
    AllInputs ai;
    ai.clear();
    ai.events = inputEventModel->events;
    ai.processEvents();
    movie->inputs->setInputs(ai, frame);
    accept();
}

void InputEventWindow::slotNew()
{
    const QModelIndexList indexes = inputEventView->selectionModel()->selectedRows();

    if (indexes.count() == 0) {
        inputEventModel->append();
        return;
    }

    int min_row = 1 << 30;
    int max_row = 0;
    for (const QModelIndex index : indexes) {
        if (index.row() < min_row)
            min_row = index.row();
        if (index.row() > max_row)
            max_row = index.row();
    }

    inputEventModel->duplicate(min_row, max_row);
}

void InputEventWindow::slotRemove()
{
    int min_row = 1 << 30;
    int max_row = 0;
    const QModelIndexList indexes = inputEventView->selectionModel()->selectedRows();

    for (const QModelIndex index : indexes) {
        if (index.row() < min_row)
            min_row = index.row();
        if (index.row() > max_row)
            max_row = index.row();
    }

    inputEventModel->remove(min_row, max_row);
}

void InputEventWindow::slotClear()
{
    inputEventModel->clear();
}
