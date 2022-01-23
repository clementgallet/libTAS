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

#include <QtWidgets/QMessageBox>
#include "RamWatchModel.h"
#include "../ramsearch/IRamWatchDetailed.h"
#include "../ramsearch/RamWatchDetailed.h"

#include <stdint.h>

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

void RamWatchModel::saveSettings(QSettings& watchSettings)
{
    watchSettings.beginWriteArray("watches");
    int i = 0;
    for (const std::unique_ptr<IRamWatchDetailed>& w : ramwatches) {
        watchSettings.setArrayIndex(i++);
        watchSettings.setValue("address", static_cast<unsigned long long>(w->address));
        watchSettings.setValue("label", w->label.c_str());
        watchSettings.setValue("type", w->type());
        watchSettings.setValue("hex", w->hex);
        watchSettings.setValue("isPointer", w->isPointer);
        if (w->isPointer) {
            watchSettings.setValue("base_file", w->base_file.c_str());
            watchSettings.setValue("base_file_offset", static_cast<long long>(w->base_file_offset));
            watchSettings.beginWriteArray("offsets");
            int j = 0;
            for (int o : w->pointer_offsets) {
                watchSettings.setArrayIndex(j++);
                watchSettings.setValue("offset", o);
            }
            watchSettings.endArray();
        }
    }
    watchSettings.endArray();
}

void RamWatchModel::loadSettings(QSettings& watchSettings)
{
    beginResetModel();

    int size = watchSettings.beginReadArray("watches");
    ramwatches.clear();
    for (int i = 0; i < size; ++i) {
        watchSettings.setArrayIndex(i);

        std::unique_ptr<IRamWatchDetailed> ramwatch;
        int type = watchSettings.value("type").toInt();
        uintptr_t addr = watchSettings.value("address").toULongLong();

        /* Build the ram watch using the right type as template */
        switch (type) {
            case RamUnsignedChar:
                ramwatch.reset(new RamWatchDetailed<unsigned char>(addr));
                break;
            case RamChar:
                ramwatch.reset(new RamWatchDetailed<char>(addr));
                break;
            case RamUnsignedShort:
                ramwatch.reset(new RamWatchDetailed<unsigned short>(addr));
                break;
            case RamShort:
                ramwatch.reset(new RamWatchDetailed<short>(addr));
                break;
            case RamUnsignedInt:
                ramwatch.reset(new RamWatchDetailed<unsigned int>(addr));
                break;
            case RamInt:
                ramwatch.reset(new RamWatchDetailed<int>(addr));
                break;
            case RamUnsignedLong:
                ramwatch.reset(new RamWatchDetailed<uint64_t>(addr));
                break;
            case RamLong:
                ramwatch.reset(new RamWatchDetailed<int64_t>(addr));
                break;
            case RamFloat:
                ramwatch.reset(new RamWatchDetailed<float>(addr));
                break;
            case RamDouble:
                ramwatch.reset(new RamWatchDetailed<double>(addr));
                break;
            default:
                QMessageBox::critical(nullptr, "Error", "Could not determine type of ram watch");
                return;
        }
        ramwatch->label = watchSettings.value("label").toString().toStdString();
        ramwatch->hex = watchSettings.value("hex").toBool();
        ramwatch->isPointer = watchSettings.value("isPointer").toBool();
        if (ramwatch->isPointer) {
            ramwatch->base_file = watchSettings.value("base_file").toString().toStdString();
            ramwatch->base_file_offset = watchSettings.value("base_file_offset").toLongLong();
            int size_off = watchSettings.beginReadArray("offsets");
            for (int j = 0; j < size_off; ++j) {
                watchSettings.setArrayIndex(j);
                ramwatch->pointer_offsets.push_back(watchSettings.value("offset").toInt());
            }
            watchSettings.endArray();
        }
        ramwatches.push_back(std::move(ramwatch));
    }
    watchSettings.endArray();

    endResetModel();
}


void RamWatchModel::update()
{
    emit dataChanged(index(0,0), index(rowCount()-1,1), QVector<int>(Qt::DisplayRole));
}
