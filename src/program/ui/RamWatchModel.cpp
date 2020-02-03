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

#include "RamWatchModel.h"

RamWatchModel::RamWatchModel(QObject *parent) : QAbstractTableModel(parent) {}

int RamWatchModel::rowCount(const QModelIndex & /*parent*/) const
{
   return ramwatches.size();
}

int RamWatchModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant RamWatchModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Address");
            case 1:
                return QString("Value");
            case 2:
                return QString("Label");
            }
        }
    }
    return QVariant();
}

QVariant RamWatchModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const std::unique_ptr<IRamWatchDetailed> &watch = ramwatches.at(index.row());
        switch(index.column()) {
            case 0:
                if (watch->isPointer)
                    return QString("P->%1").arg(watch->address, 0, 16);
                else
                    return QString("%1").arg(watch->address, 0, 16);
            case 1:
                return QString(watch->value_str().c_str());
            case 2:
                return QString(watch->label.c_str());
            default:
                return QString();
        }
    }
    return QVariant();
}

void RamWatchModel::addWatch(std::unique_ptr<IRamWatchDetailed> ramwatch)
{
    beginInsertRows(QModelIndex(), ramwatches.size(), ramwatches.size());
    ramwatches.push_back(std::move(ramwatch));
    endInsertRows();
}

void RamWatchModel::removeWatch(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    ramwatches.erase(ramwatches.begin() + row);
    endRemoveRows();
}


void RamWatchModel::update()
{
    emit dataChanged(createIndex(0,0), createIndex(rowCount(),1));
}
