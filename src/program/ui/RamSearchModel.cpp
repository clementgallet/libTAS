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

#include "RamSearchModel.h"

#include "Context.h"
#include "qtutils.h"
#include "ramsearch/MemLayout.h"
#include "ramsearch/MemSection.h"

#include <QtWidgets/QMessageBox>
#include <memory>

RamSearchModel::RamSearchModel(Context* c, QObject *parent) : QAbstractTableModel(parent), context(c)
{
    if (isLightTheme())
        unmatchedColor.setRgbF(1.0f, 0.8f, 0.8f, 1.0f);
    else
        unmatchedColor.setRgbF(0.0f, 0.2f, 0.2f, 1.0f);
}

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
    MemValueType new_v;
    const MemValueType* old_v;

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0:
                return QString("%1").arg(memscanner.get_address(index.row()), 0, 16);
            case 1:
                new_v = memscanner.get_current_value(index.row());
                return QString(MemValue::to_string(&new_v, value_type, hex));
            case 2:
                old_v = memscanner.get_previous_value(index.row());
                return QString(MemValue::to_string(old_v, value_type, hex));
            default:
                return QString();
        }
    }
    if (role == Qt::BackgroundRole) {
        if (compare_type == CompareType::Previous) {
            new_v = memscanner.get_current_value(index.row());
            old_v = memscanner.get_previous_value(index.row());
            if (!CompareOperations::check_previous(&new_v, old_v))
                return QBrush(unmatchedColor);
        }
        else {
            new_v = memscanner.get_current_value(index.row());
            if (!CompareOperations::check_value(&new_v))
                return QBrush(unmatchedColor);
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

int RamSearchModel::newWatches(int mem_flags, int type, int alignment, CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv, uintptr_t ba, uintptr_t ea)
{
    value_type = type;
    compare_type = ct;

    beginResetModel();

    int err = memscanner.first_scan(mem_flags, type, alignment, ct, co, cv, dv, ba, ea);

    endResetModel();
    
    return err;
}

int RamSearchModel::searchWatches(CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv)
{
    beginResetModel();

    int err = memscanner.scan(false, ct, co, cv, dv);

    endResetModel();
    
    return err;
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

void RamSearchModel::updateParameters(CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv)
{
    compare_type = ct;
    CompareOperations::init(co, cv, dv);
}

void RamSearchModel::clear()
{
    beginResetModel();
    memscanner.clear();
    endResetModel();
}

void RamSearchModel::stopSearch()
{
    memscanner.is_stopped = true;
}
