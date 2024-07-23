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

#include "InputChangeLogModel.h"
#include "qtutils.h"

#include "Context.h"
#include "movie/MovieFile.h"

#include <QtGui/QBrush>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>

InputChangeLogModel::InputChangeLogModel(Context* c, MovieFile* m, QObject *parent) : QAbstractTableModel(parent), context(c), movie(m) {
    connect(movie->changelog, &MovieFileChangeLog::beginAddHistory, this, &InputChangeLogModel::beginAddHistory);
    connect(movie->changelog, &MovieFileChangeLog::endAddHistory, this, &InputChangeLogModel::endAddHistory);
    connect(movie->changelog, &MovieFileChangeLog::beginRemoveHistory, this, &InputChangeLogModel::beginRemoveHistory);
    connect(movie->changelog, &MovieFileChangeLog::endRemoveHistory, this, &InputChangeLogModel::endRemoveHistory);
    connect(movie->changelog, &MovieFileChangeLog::changeHistory, this, &InputChangeLogModel::changeHistory);
}

int InputChangeLogModel::rowCount(const QModelIndex & /*parent*/) const
{
    return movie->changelog->history.size();
}

int InputChangeLogModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 2;
}

Qt::ItemFlags InputChangeLogModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

QVariant InputChangeLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0)
                return QString(tr("Index"));
            if (section == 1)
                return QString(tr("Action"));
        }
    }

    return QVariant();
}

QVariant InputChangeLogModel::data(const QModelIndex &index, int role) const
{
    unsigned int row = index.row();

    if (row >= rowCount())
        return QVariant();

    if (role == Qt::BackgroundRole) {
        /* Main color */
        QColor color = QGuiApplication::palette().window().color();
        bool lightTheme = isLightTheme();
        int r, g, b;
        color.getRgb(&r, &g, &b, nullptr);
        
        if (lightTheme) {
            /* Light theme */
            if (row == (movie->changelog->history_index-1))
                color.setRgb(r - 0x30, g - 0x10, b);
            else if (row < (movie->changelog->history_index-1)) {
                color.setRgb(r - 0x20, g, b - 0x20);
            }
            else {
                color.setRgb(r, g, b - 0x10);
            }
        }
        else {
            /* Dark theme */
            if (row == movie->changelog->history_index)
                color.setRgb(r, g + 0x10, b + 0x20);
            else if (row < movie->changelog->history_index) {
                color.setRgb(r, g + 0x18, b);
            }
            else {
                color.setRgb(r + 0x08, g + 0x08, b);
            }
        }
        
        return QBrush(color);
    }

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return row;
        }
        if (index.column() == 1) {
            return QString((*std::next(movie->changelog->history.begin(), row))->description.c_str());
        }
    }

    return QVariant();
}

void InputChangeLogModel::beginAddHistory(int frame)
{
    beginInsertRows(QModelIndex(), frame, frame);
}

void InputChangeLogModel::endAddHistory()
{
    endInsertRows();
}

void InputChangeLogModel::beginRemoveHistory(int first_frame, int last_frame)
{
    beginRemoveRows(QModelIndex(), first_frame, last_frame);
}

void InputChangeLogModel::endRemoveHistory()
{
    endRemoveRows();
}

void InputChangeLogModel::changeHistory(int frame)
{
    emit dataChanged(index(frame-1,0), index(frame,1));

}
