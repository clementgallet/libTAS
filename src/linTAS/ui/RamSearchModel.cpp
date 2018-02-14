/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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
// #include <FL/fl_draw.H>
// #include <inttypes.h> // PRIxPTR

RamSearchModel::RamSearchModel(std::vector<std::unique_ptr<IRamWatch>> *rw, QObject *parent) : QAbstractTableModel(parent)
{
    ramwatches = rw;
}

int RamSearchModel::rowCount(const QModelIndex & /*parent*/) const
{
   return ramwatches->size();
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
                return QString("Previous Value");
            }
        }
    }
    return QVariant();
}

QVariant RamSearchModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const std::unique_ptr<IRamWatch> &watch = ramwatches->at(index.row());
        switch(index.column()) {
            case 0:
                return QString("%1").arg(watch->address, 0, 16);
            case 1:
                return QString(watch->tostring_current(hex));
            case 2:
                return QString(watch->tostring(hex));
            default:
                return QString();
        }
    }
    return QVariant();
}

void RamSearchModel::update()
{
    emit dataChanged(createIndex(0,1), createIndex(rowCount(),1));
}
