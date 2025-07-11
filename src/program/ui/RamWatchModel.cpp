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

#include "RamWatchModel.h"

#include "ramsearch/RamWatchDetailed.h"

#include <QtWidgets/QMessageBox>
#include <QtGui/QGuiApplication>
#include <stdint.h>

static const char mimeType[] = "application/x-libtas-rownumber";

RamWatchModel::RamWatchModel(QObject *parent) : QAbstractTableModel(parent) {}

int RamWatchModel::rowCount(const QModelIndex & /*parent*/) const
{
   return ramwatches.size();
}

int RamWatchModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

Qt::ItemFlags RamWatchModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (!index.isValid())
        flags |= Qt::ItemIsDropEnabled;
    else {
        flags |= Qt::ItemIsDragEnabled;
        /* Value and label are editable */
        if (index.column() != 0)
            flags |= Qt::ItemIsEditable;
    }
    return flags;
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
    const std::unique_ptr<RamWatchDetailed> &watch = ramwatches.at(index.row());
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch(index.column()) {
            case 0:
                if (watch->is_pointer)
                    return QString("P->%1").arg(watch->address, 0, 16);
                else
                    return QString("%1").arg(watch->address, 0, 16);
            case 1:
                return QString(watch->value_str());
            case 2:
                return QString(watch->label.c_str());
            default:
                return QString();
        }
    }
    if (role == Qt::ForegroundRole) {
        if (index.column() == 1) {
            if (watch->is_frozen) {
                return QBrush(QColorConstants::Red);
            }
        }
        return QGuiApplication::palette().text();
    }
    return QVariant();
}

bool RamWatchModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    std::unique_ptr<RamWatchDetailed> &watch = ramwatches.at(index.row());

    switch(index.column()) {
        case 0:
            return false;
        case 1:
            {
                int res = watch->poke_value(value.toString().toLocal8Bit().constData());
                if (res < 0) {
                    if (res == EFAULT) {
                        QMessageBox::critical(nullptr, "Error", QString("Poking failed because address is outside the game's accessible address space."));
                    }
                    else if (res == EPERM) {
                        QMessageBox::critical(nullptr, "Error", QString("Poking failed because we don't have permission to write at the game address."));
                    }
                    else if (res == ESRCH) {
                        QMessageBox::critical(nullptr, "Error", QString("Poking failed because the game does not appear to be running."));
                    }
                    else {
                        QMessageBox::critical(nullptr, "Error", QString("Poking failed."));
                    }
                    return false;
                }
            }
            return true;
        case 2:
            watch->label = value.toString().toLocal8Bit().constData();
            return true;
        default:
            return false;
    }
    return false;
}

bool RamWatchModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (!beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild))
        return false;

    for (int i = 0; i < count; ++i) {
        int inserted_index = destinationChild + (destinationChild > sourceRow ? 0 : i);
        int moved_index = sourceRow + (destinationChild > sourceRow ? 0 : i+1);
        
        ramwatches.insert(ramwatches.begin() + inserted_index, std::unique_ptr<RamWatchDetailed>(nullptr));
        ramwatches.at(inserted_index).swap(ramwatches.at(moved_index));
        ramwatches.erase(ramwatches.begin() + moved_index);
    }

    endMoveRows();
    return true;
}

void RamWatchModel::addWatch(std::unique_ptr<RamWatchDetailed> ramwatch)
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

Qt::DropActions RamWatchModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions RamWatchModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

/* mime code taken from <https://www.kdab.com/modelview-drag-and-drop-part-1/>
 * under MIT licence */
QStringList RamWatchModel::mimeTypes() const
{
    return {QString::fromLatin1(mimeType)};
}

QMimeData *RamWatchModel::mimeData(const QModelIndexList &indexes) const
{
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    int minRow = 1 << 30;
    int maxRow = 0;
    for (const QModelIndex &index : indexes) {
        if (index.row() < minRow)
            minRow = index.row();
        if (index.row() > maxRow)
            maxRow = index.row();
    }
    int count = maxRow - minRow + 1;
    stream << minRow << count;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(mimeType, encodedData);
    return mimeData;
}

bool RamWatchModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    // check if the format is supported
    if (!mimeData->hasFormat(mimeType))
        return false;
    // only drop between items (just to be safe, given that dropping onto items is forbidden by our flags() implementation)
    if (parent.isValid() && row == -1)
        return false;
    // drop into empty area = append
    if (row == -1)
        row = rowCount(parent);

    // decode data
    const QByteArray encodedData = mimeData->data(mimeType);
    QDataStream stream(encodedData);
    if (stream.atEnd())
        return false;

    int minRow;
    int count;

    stream >> minRow >> count;
    moveRows(parent, minRow, count, parent, row);

    return false; // we handled the move, not just the insertion, so don't let the caller do
                  // the removal of the source rows
}

void RamWatchModel::saveSettings(QSettings& watchSettings)
{
    watchSettings.beginWriteArray("watches");
    int i = 0;
    for (const std::unique_ptr<RamWatchDetailed>& w : ramwatches) {
        watchSettings.setArrayIndex(i++);
        watchSettings.setValue("address", static_cast<unsigned long long>(w->address));
        watchSettings.setValue("label", w->label.c_str());
        watchSettings.setValue("type", w->value_type);
        watchSettings.setValue("hex", w->hex);
        watchSettings.setValue("isPointer", w->is_pointer);
        if (w->is_pointer) {
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

        std::unique_ptr<RamWatchDetailed> ramwatch;
        int type = watchSettings.value("type").toInt();
        uintptr_t addr = watchSettings.value("address").toULongLong();

        /* Build the ram watch using the right type as template */
        ramwatch.reset(new RamWatchDetailed(addr, type));

        ramwatch->label = watchSettings.value("label").toString().toStdString();
        ramwatch->hex = watchSettings.value("hex").toBool();
        ramwatch->is_pointer = watchSettings.value("isPointer").toBool();
        if (ramwatch->is_pointer) {
            ramwatch->base_address = 0;
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

void RamWatchModel::update_frozen()
{
    for (std::unique_ptr<RamWatchDetailed>& w : ramwatches) {
        w->keep_frozen();
    }
}
