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

#include <QTableView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QAction>
#include <QHeaderView>
#include <QMessageBox>

#include "InputEditorView.h"
#include "MainWindow.h"

InputEditorView::InputEditorView(Context* c, QWidget *parent) : QTableView(parent), context(c)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setShowGrid(true);

    MovieFile *movie = nullptr;
    MainWindow *mw = qobject_cast<MainWindow*>(parent->parent());
    if (mw) {
        movie = &mw->gameLoop->movie;
    }

    inputEditorModel = new InputEditorModel(context, movie);
    setModel(inputEditorModel);
    // connect(this, &QAbstractItemView::pressed, this, &InputEditorView::toggleInput);

    /* Horizontal header */
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setSectionsMovable(true);
    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setDropIndicatorShown(true);
    horizontalHeader()->setDragEnabled(true);
    horizontalHeader()->setDragDropMode(QTableView::InternalMove);

    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &InputEditorView::horizontalMenu);

    /* Horizontal menu */
    horMenu = new QMenu(this);
    horMenu->addAction(tr("Rename label"), this, &InputEditorView::renameLabel);

    /* Vertical header */
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());

    /* Main menu */
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &InputEditorView::mainMenu);

    menu = new QMenu(this);
    menu->addAction(tr("Insert input before"), this, &InputEditorView::insertInput);
    menu->addAction(tr("Delete input"), this, &InputEditorView::deleteInput);
}

void InputEditorView::update()
{
    inputEditorModel->update();

    /* Scroll to make the current frame visible */
    QModelIndex index = inputEditorModel->index(context->framecount-1, 0);
    if (index.isValid())
        scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void InputEditorView::mousePressEvent(QMouseEvent *event)
{
    mouseSection = -1;

    if (event->button() != Qt::LeftButton) {
        return QTableView::mousePressEvent(event);
    }

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mousePressEvent(event);
    }

    if (index.column() == 0) {
        return QTableView::mousePressEvent(event);
    }

    selectionModel()->clear();
    mouseSection = index.column();
    mouseValue = inputEditorModel->toggleInput(index);
    event->accept();
}

void InputEditorView::mouseMoveEvent(QMouseEvent *event)
{
    /* Check if the mouse press event was valid */
    if (mouseSection < 0) {
        return QTableView::mouseMoveEvent(event);
    }

    if (!(event->buttons() & Qt::LeftButton)) {
        return QTableView::mouseMoveEvent(event);
    }

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mouseMoveEvent(event);
    }
    // std::cout << "valid move" << std::endl;

    /* Toggle the cell with the same row as the cell under the mouse */

    QModelIndex toggle_index = inputEditorModel->index(index.row(), mouseSection);
    // std::cout << "mouse move" << std::endl;

    // selectionModel()->clear();
    inputEditorModel->editInput(toggle_index, mouseValue);
    event->accept();
}

// void InputEditorView::toggleInput(const QModelIndex &index)
// {
//     // Qt::MouseButtons buttons = QApplication::mouseButtons()
//
//     /* If we toggle an input, do not select the row */
//     if (index.column() != 0) {
//         setSelectionMode(QAbstractItemView::NoSelection);
//     }
//
//     inputEditorModel->toggleInput(index);
//
//     if (index.column() != 0) {
//         setSelectionMode(QAbstractItemView::ExtendedSelection);
//     }
// }

void InputEditorView::horizontalMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    contextSection = horizontalHeader()->logicalIndexAt(pos);

    /* Display the context menu */
    horMenu->popup(horizontalHeader()->viewport()->mapToGlobal(pos));
}

void InputEditorView::renameLabel()
{
    if (contextSection == 0)
        return;

    QString text = QString("New label for input %1 is: ").arg(inputEditorModel->inputDescription(contextSection).c_str());
    QString newLabel = QInputDialog::getText(this, tr("Rename label"), text, QLineEdit::Normal, QString(inputEditorModel->inputLabel(contextSection).c_str()));

    if (!newLabel.isEmpty()) {
        inputEditorModel->renameLabel(contextSection, newLabel.toStdString());
    }
}

void InputEditorView::mainMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    // contextSection = inputEditorView->verticalHeader()->logicalIndexAt(pos);

    /* Display the context menu */
    menu->popup(viewport()->mapToGlobal(pos));
}

void InputEditorView::insertInput()
{
    const QModelIndex index = selectionModel()->currentIndex();

    /* If no row was selected, return */
    if (!index.isValid())
        return;

    inputEditorModel->insertRows(index.row(), 1);
}

void InputEditorView::deleteInput()
{
    if (!selectionModel()->hasSelection())
        return;

    /* Selection mode garanties that we select a contiguous range, so getting
     * the min and max row.
     */
    int min_row = 2000000000;
    int max_row = -1;
    for (const QModelIndex index : selectionModel()->selectedRows()) {
        min_row = (index.row()<min_row)?index.row():min_row;
        max_row = (index.row()>max_row)?index.row():max_row;
    }
    inputEditorModel->removeRows(min_row, max_row-min_row+1);
}
