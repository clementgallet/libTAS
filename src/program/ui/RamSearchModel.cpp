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

#include "RamSearchModel.h"
#include "../ramsearch/MemLayout.h"
#include "../ramsearch/MemSection.h"

#include <QtWidgets/QMessageBox>
#include <memory>

RamSearchModel::RamSearchModel(Context* c, QObject *parent) : QAbstractTableModel(parent), context(c) {}

int RamSearchModel::rowCount(const QModelIndex & /*parent*/) const
{
   return memscanner.display_scan_count();
}

int RamSearchModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant RamSearchModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Address");
            case 1:
                return QString("Value");
            case 2:
                return QString("Previous");
            }
        }
    }
    return QVariant();
}

QVariant RamSearchModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0:
                return QString("%1").arg(memscanner.get_address(index.row()), 0, 16);
            case 1:
                return QString(memscanner.get_current_value(index.row(), hex));
            case 2:
                return QString(memscanner.get_previous_value(index.row(), hex));
            default:
                return QString();
        }
    }
    return QVariant();
}

int RamSearchModel::predictScanCount(int mem_flags)
{
    std::unique_ptr<MemLayout> memlayout (new MemLayout(context->game_pid));
    return memlayout->totalSize(MemSection::MemAll, mem_flags);
}

uint64_t RamSearchModel::scanCount()
{
    return memscanner.scan_count();
}

uint64_t RamSearchModel::scanSize()
{
    return memscanner.scan_size();
}

void RamSearchModel::newWatches(int mem_flags, int type, CompareType ct, CompareOperator co, double cv, double dv)
{
    compare_type = ct;
    compare_operator = co;
    compare_value = cv;
    different_value = dv;

    beginResetModel();

    memscanner.first_scan(context->game_pid, mem_flags, type, ct, co, cv, dv);

    endResetModel();
}

void RamSearchModel::searchWatches(CompareType ct, CompareOperator co, double cv, double dv)
{
    beginResetModel();

    memscanner.scan(false, ct, co, cv, dv);

    endResetModel();
}

void RamSearchModel::update()
{
    if (rowCount() > 0)
        emit dataChanged(index(0,1), index(rowCount()-1,1), QVector<int>(Qt::DisplayRole));
}

uintptr_t RamSearchModel::address(int row)
{
    return memscanner.get_address(row);
}

void RamSearchModel::clear()
{
    beginResetModel();
    memscanner.clear();
    endResetModel();
}
