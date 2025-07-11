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

#include "AnalogInputsWindow.h"
#include "AnalogInputsModel.h"

#include "Context.h"

#include <QtWidgets/QListView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

AnalogInputsWindow::AnalogInputsWindow(Context* c, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Analog Inputs");

    /* Table */
    analogInputsView = new QListView(this);
    analogInputsView->setSelectionMode(QAbstractItemView::SingleSelection);
    analogInputsView->setAlternatingRowColors(true);
    analogInputsView->setDragEnabled(true);
    analogInputsView->setDragDropMode(QAbstractItemView::DragOnly);
    analogInputsView->setDefaultDropAction(Qt::CopyAction);
    analogInputsView->setDragDropOverwriteMode(false);

    analogInputsModel = new AnalogInputsModel(context);
    analogInputsView->setModel(analogInputsModel);

    /* Layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(new QLabel("Drag and Drop into input editor to add column"));
    mainLayout->addWidget(analogInputsView);

    setLayout(mainLayout);
}

QSize AnalogInputsWindow::sizeHint() const
{
    return QSize(400, 600);
}
