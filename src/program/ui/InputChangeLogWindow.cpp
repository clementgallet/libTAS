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
#include <QtWidgets/QListView>

#include <stdint.h>
#include <climits>

InputChangeLogWindow::InputChangeLogWindow(Context* c, MovieFile *m, QWidget *parent) : QDialog(parent), context(c), movie(m)
{
    setWindowTitle("Movie Change Log");

    inputChangeLogView = new QListView(this);

    inputChangeLogView->setSelectionMode(QAbstractItemView::SingleSelection);

    inputChangeLogModel = new InputChangeLogModel(context, movie, this);
    inputChangeLogView->setModel(inputChangeLogModel);

    connect(movie->changelog, &MovieFileChangeLog::updateChangeLog, this, &InputChangeLogWindow::update);
    connect(inputChangeLogView, &QListView::clicked, this, &InputChangeLogWindow::moveChangeLog);

    /* Layout */
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(inputChangeLogView);
    setLayout(mainLayout);
}

QSize InputChangeLogWindow::sizeHint() const
{
    return QSize(300, 500);
}

void InputChangeLogWindow::update()
{
    inputChangeLogModel->updateChangeLog();
    
    /* Set the selection to the current changelog item */
    inputChangeLogView->setCurrentIndex(inputChangeLogModel->index(movie->changelog->index(), 0, QModelIndex()));
}

void InputChangeLogWindow::moveChangeLog(const QModelIndex &index)
{
    disconnect(movie->changelog, &MovieFileChangeLog::updateChangeLog, this, &InputChangeLogWindow::update);

    int currentRow = movie->changelog->index();
    int newRow = index.row();
    
    if (newRow > currentRow) {
        bool didRedo = true;
        while ((currentRow < newRow) && didRedo) {
            didRedo = movie->changelog->redo();
            currentRow++;
        }
    }
    else {
        bool didUndo = true;
        while ((currentRow > newRow) && didUndo) {
            didUndo = movie->changelog->undo();
            currentRow--;
        }
    }
    
    update();
    connect(movie->changelog, &MovieFileChangeLog::updateChangeLog, this, &InputChangeLogWindow::update);
}
