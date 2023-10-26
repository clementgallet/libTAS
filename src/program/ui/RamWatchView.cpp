/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>
#include <QtGui/QMouseEvent>

#include "RamWatchView.h"
#include "RamWatchModel.h"
#include "RamWatchEditWindow.h"
#include "../Context.h"

RamWatchView::RamWatchView(Context* c, QWidget *parent) : QTableView(parent), context(c)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setShowGrid(false);
    setAlternatingRowColors(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setHighlightSections(false);
    verticalHeader()->hide();

    ramWatchModel = new RamWatchModel();
    setModel(ramWatchModel);
    
    editWindow = new RamWatchEditWindow(this);
}

void RamWatchView::update()
{
    ramWatchModel->update();
}

void RamWatchView::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mouseDoubleClickEvent(event);
    }

    int row = index.row();

    /* Fill and show the watch edit window */
    editWindow->fill(ramWatchModel->ramwatches.at(row));
    editWindow->exec();

    /* Modify the watch */
    if (editWindow->ramwatch) {
        ramWatchModel->ramwatches[row] = std::move(editWindow->ramwatch);
        ramWatchModel->update();
    }

    event->accept();
}

void RamWatchView::slotAdd()
{
    editWindow->exec();

    if (editWindow->ramwatch) {
        ramWatchModel->addWatch(std::move(editWindow->ramwatch));
    }
}

void RamWatchView::slotGet(std::string &watch)
{
    static unsigned int index = 0;

    if (index >= ramWatchModel->ramwatches.size()) {
        /* We sent all watches, returning NULL */
        watch = "";
        index = 0;
        return;
    }

    watch = ramWatchModel->ramwatches[index]->label;
    watch += ": ";
    watch += ramWatchModel->ramwatches[index]->value_str();

    index++;
}

void RamWatchView::slotEdit()
{
    const QModelIndex index = selectionModel()->currentIndex();

    /* If no watch was selected, return */
    if (!index.isValid())
        return;

    int row = index.row();

    /* Fill and show the watch edit window */
    editWindow->fill(ramWatchModel->ramwatches.at(row));
    editWindow->exec();

    /* Modify the watch */
    if (editWindow->ramwatch) {
        ramWatchModel->ramwatches[row] = std::move(editWindow->ramwatch);
        ramWatchModel->update();
    }
}

void RamWatchView::slotRemove()
{
    QModelIndexList indexlist = selectionModel()->selectedRows();
    selectionModel()->clear();

    /* If no watch was selected, return */
    if (indexlist.isEmpty())
        return;

    for (const QModelIndex index : indexlist) {
        ramWatchModel->removeWatch(index.row());
    }
}
