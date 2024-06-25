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

#include "MarkerView.h"
#include "MarkerModel.h"
#include "MainWindow.h"

#include "Context.h"

#include <QtWidgets/QHeaderView>

#include <stdint.h>
#include <climits>

MarkerView::MarkerView(Context* c, QWidget *parent, QWidget *gp) : QTableView(parent), context(c)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setShowGrid(true);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    MovieFile *movie = nullptr;
    MainWindow *mw = qobject_cast<MainWindow*>(gp);
    if (mw) {
        movie = &mw->gameLoop->movie;
    }

    markerModel = new MarkerModel(context, movie);
    setModel(markerModel);

    /* Horizontal header */
    horizontalHeader()->setMinimumSectionSize(20);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setResizeContentsPrecision(1);

    /* Frame column is fixed */
    horizontalHeader()->resizeSection(0, 80);
    horizontalHeader()->setStretchLastSection(true);

    /* Vertical header */
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize());
    
    fillMenu();
}

void MarkerView::fillMenu()
{
    menu = new QMenu(this);
    
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &MarkerView::mainMenu);

    seekAct = menu->addAction(tr("Seek To"), this, &MarkerView::seekSlot);
    scrollAct = menu->addAction(tr("Scroll To"), this, &MarkerView::scrollSlot);
    removeAct = menu->addAction(tr("Remove"), this, &MarkerView::removeSlot);
}

void MarkerView::resetMarkers()
{
    markerModel->resetMarkers();
}

void MarkerView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return QTableView::mouseDoubleClickEvent(event);
    }

    /* Get the table cell under the mouse position */
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {
        return QTableView::mouseDoubleClickEvent(event);
    }

    if (index.column() != 0)
        return QTableView::mouseDoubleClickEvent(event);
    
    uint64_t frame = markerModel->data(index, Qt::DisplayRole).toULongLong();

    emit seekSignal(frame);
    event->accept();
}

void MarkerView::mainMenu(QPoint pos)
{
    /* Display the context menu */
    menu->popup(viewport()->mapToGlobal(pos));
}

unsigned long long MarkerView::getMarkerFrameFromSelection()
{
    /* Helper to get the marker info from the first selected row */
    const QModelIndexList indexes = selectionModel()->selectedRows();

    /* If no row was selected, return */
    if (indexes.count() == 0)
        return ULLONG_MAX;

    QModelIndex toggle_index = markerModel->index(indexes[0].row(), 0);
    return markerModel->data(toggle_index, Qt::DisplayRole).toULongLong();
}

void MarkerView::seekSlot()
{
    unsigned long long frame = getMarkerFrameFromSelection();
    if (frame != ULLONG_MAX)
        emit seekSignal(frame);
}

void MarkerView::scrollSlot()
{
    unsigned long long frame = getMarkerFrameFromSelection();
    if (frame != ULLONG_MAX)
        emit scrollSignal(frame);
}

void MarkerView::removeSlot()
{
    const QModelIndexList indexes = selectionModel()->selectedRows();

    for (int i = indexes.count()-1; i >= 0; i--) {
        markerModel->removeRows(indexes[i].row(), 1);
    }
}
