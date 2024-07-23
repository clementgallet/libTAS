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

#include "InputChangeLogWindow.h"
#include "InputChangeLogModel.h"
#include "MainWindow.h"

#include "Context.h"

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTableView>

#include <stdint.h>
#include <climits>

InputChangeLogWindow::InputChangeLogWindow(Context* c, MovieFile *movie, QWidget *parent) : QDialog(parent), context(c)
{
    setWindowTitle("Movie Change Log");

    inputChangeLogView = new QTableView(this);

    inputChangeLogView->setSelectionMode(QAbstractItemView::NoSelection);
    inputChangeLogView->setShowGrid(true);
    inputChangeLogView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    inputChangeLogView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    inputChangeLogModel = new InputChangeLogModel(context, movie);
    inputChangeLogView->setModel(inputChangeLogModel);

    /* Horizontal header */
    inputChangeLogView->horizontalHeader()->setMinimumSectionSize(20);
    inputChangeLogView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    inputChangeLogView->horizontalHeader()->setResizeContentsPrecision(1);

    /* Frame column is fixed */
    inputChangeLogView->horizontalHeader()->resizeSection(0, 60);
    inputChangeLogView->horizontalHeader()->setStretchLastSection(true);

    /* Vertical header */
    inputChangeLogView->verticalHeader()->setVisible(false);
    inputChangeLogView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    inputChangeLogView->verticalHeader()->setDefaultSectionSize(inputChangeLogView->verticalHeader()->minimumSectionSize());
    
    /* Layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(inputChangeLogView);
    setLayout(mainLayout);
}
