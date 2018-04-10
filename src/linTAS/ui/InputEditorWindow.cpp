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

#include "InputEditorWindow.h"
#include "MainWindow.h"

InputEditorWindow::InputEditorWindow(Context* c, QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags), context(c)
{
    setWindowTitle("Input Editor");

    /* Table */
    inputEditorView = new QTableView(this);
    inputEditorView->setSelectionMode(QAbstractItemView::NoSelection);
    inputEditorView->setShowGrid(true);
    inputEditorView->setAlternatingRowColors(true);
    // inputEditorView->setDragEnabled(true);
    // inputEditorView->setDragDropMode(QTableView::InternalMove);

    MovieFile *movie = nullptr;
    MainWindow *mw = qobject_cast<MainWindow*>(parent);
    if (mw) {
        movie = &mw->gameLoop->movie;
    }

    inputEditorModel = new InputEditorModel(context, movie);
    inputEditorView->setModel(inputEditorModel);
    connect(inputEditorView, &QAbstractItemView::clicked, inputEditorModel, &InputEditorModel::toggleInput);

    /* Horizontal header */
    inputEditorView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    inputEditorView->horizontalHeader()->setSectionsMovable(true);
    inputEditorView->horizontalHeader()->setDropIndicatorShown(true);
    inputEditorView->horizontalHeader()->setDragEnabled(true);
    inputEditorView->horizontalHeader()->setDragDropMode(QTableView::InternalMove);

    inputEditorView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(inputEditorView->horizontalHeader(), &QWidget::customContextMenuRequested, this, &InputEditorWindow::horizontalMenu);

    /* Horizontal menu */
    horMenu = new QMenu(this);
    horMenu->addAction(tr("Rename label"), this, &InputEditorWindow::renameLabel);

    /* Vertical header */
    inputEditorView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    inputEditorView->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(inputEditorView->verticalHeader(), &QWidget::customContextMenuRequested, this, &InputEditorWindow::verticalMenu);

    /* Vertical menu */
    vertMenu = new QMenu(this);
    vertMenu->addAction(tr("Insert input before"), this, &InputEditorWindow::insertInput);
    vertMenu->addAction(tr("Delete input"), this, &InputEditorWindow::deleteInput);

    /* Layout */
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(inputEditorView);
    setLayout(mainLayout);
}

void InputEditorWindow::update()
{
    inputEditorModel->update();

    /* Scroll to make the current frame visible */
    QModelIndex index = inputEditorModel->index(context->framecount, 0);
    inputEditorView->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void InputEditorWindow::horizontalMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    contextSection = inputEditorView->horizontalHeader()->logicalIndexAt(pos);

    /* Display the context menu */
    horMenu->popup(inputEditorView->horizontalHeader()->viewport()->mapToGlobal(pos));
}

void InputEditorWindow::renameLabel()
{
    QString text = QString("New label for input %1 is: ").arg(inputEditorModel->inputDescription(contextSection).c_str());

    QString newLabel = QInputDialog::getText(this, tr("Rename label"), text, QLineEdit::Normal, QString(inputEditorModel->inputLabel(contextSection).c_str()));

    if (!newLabel.isEmpty()) {
        inputEditorModel->renameLabel(contextSection, newLabel.toStdString());
    }
}

void InputEditorWindow::verticalMenu(QPoint pos)
{
    /* Storing the index of the section where context menu was shown */
    contextSection = inputEditorView->verticalHeader()->logicalIndexAt(pos);

    /* Display the context menu */
    vertMenu->popup(inputEditorView->verticalHeader()->viewport()->mapToGlobal(pos));
}

void InputEditorWindow::insertInput()
{
    inputEditorModel->insertRows(contextSection, 1);
}

void InputEditorWindow::deleteInput()
{
    inputEditorModel->removeRows(contextSection, 1);
}
