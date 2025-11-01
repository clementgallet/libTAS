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

InputChangeLogModel::InputChangeLogModel(Context* c, MovieFile* m, QObject *parent) : QAbstractItemModel(parent), context(c), movie(m) {
    // connect(movie->changelog, &MovieFileChangeLog::updateChangeLog, this, &InputChangeLogModel::updateChangeLog);
    connect(movie->changelog, &MovieFileChangeLog::historyToBeInserted, this, &InputChangeLogModel::beginAddHistory);
    connect(movie->changelog, &MovieFileChangeLog::historyInserted, this, &InputChangeLogModel::endAddHistory);
    connect(movie->changelog, &MovieFileChangeLog::historyToBeRemoved, this, &InputChangeLogModel::beginRemoveHistory);
    connect(movie->changelog, &MovieFileChangeLog::historyRemoved, this, &InputChangeLogModel::endRemoveHistory);
    // connect(movie->changelog, &MovieFileChangeLog::changeHistory, this, &InputChangeLogModel::changeHistory);
    connect(movie->inputs, &MovieFileInputs::inputsToBeReset, this, &InputChangeLogModel::beginResetModel);
    connect(movie->inputs, &MovieFileInputs::inputsReset, this, &InputChangeLogModel::endResetModel);
}

int InputChangeLogModel::rowCount(const QModelIndex & /*parent*/) const
{
    return movie->changelog->count() + 1;
}

int InputChangeLogModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}

QModelIndex InputChangeLogModel::index(int row, int column, const QModelIndex &parent = QModelIndex()) const
{
    return createIndex(row, column);
}

QModelIndex InputChangeLogModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

Qt::ItemFlags InputChangeLogModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flag = QAbstractItemModel::flags(index);
    if (index.row() <= disabledUndoRow)
        flag &= ~Qt::ItemIsEnabled;
    if (index.row() >= disabledRedoRow)
        flag &= ~Qt::ItemIsEnabled;
    return flag;
}

QVariant InputChangeLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return QString(tr("Action"));
        }
    }

    return QVariant();
}

QVariant InputChangeLogModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (index.row() >= rowCount())
        return QVariant();

    // if (role == Qt::BackgroundRole) {
    //     /* Main color */
    //     QColor color = QGuiApplication::palette().window().color();
    //     bool lightTheme = isLightTheme();
    //     int r, g, b;
    //     color.getRgb(&r, &g, &b, nullptr);
    // 
    //     if (lightTheme) {
    //         /* Light theme */
    //         if (row == (movie->changelog->index()-1))
    //             color.setRgb(r - 0x30, g - 0x10, b);
    //         else if (row < (movie->changelog->index()-1)) {
    //             color.setRgb(r - 0x20, g, b - 0x20);
    //         }
    //         else {
    //             color.setRgb(r, g, b - 0x10);
    //         }
    //     }
    //     else {
    //         /* Dark theme */
    //         if (row == movie->changelog->index())
    //             color.setRgb(r, g + 0x10, b + 0x20);
    //         else if (row < movie->changelog->index()) {
    //             color.setRgb(r, g + 0x18, b);
    //         }
    //         else {
    //             color.setRgb(r + 0x08, g + 0x08, b);
    //         }
    //     }
    // 
    //     return QBrush(color);
    // }

    if (role == Qt::DisplayRole) {
        if (row == 0)
            return QString(tr("--- start ---"));
        else
            return QString(movie->changelog->command(row-1)->text());
    }

    return QVariant();
}

void InputChangeLogModel::updateChangeLog()
{
    // beginResetModel();
    
    /* Disable all items that would need a rewind */
    int currentRow = movie->changelog->index();
    disabledUndoRow = -1;
    for (int i = currentRow-1; i >= 0; i--) {
        const IMovieAction* action = dynamic_cast<const IMovieAction*>(movie->changelog->command(i));
        if (action->first_frame < context->framecount) {
            disabledUndoRow = i;
            break;
        }
    }

    disabledRedoRow = 1<<30;
    for (int i = currentRow; i < movie->changelog->count(); i++) {
        const IMovieAction* action = dynamic_cast<const IMovieAction*>(movie->changelog->command(i));
        if (action->first_frame < context->framecount) {
            disabledRedoRow = i+1;
            break;
        }
    }
    
    // endResetModel();
}

void InputChangeLogModel::beginAddHistory(int row)
{
    beginInsertRows(QModelIndex(), row, row);
}

void InputChangeLogModel::endAddHistory()
{
    endInsertRows();
    updateChangeLog();
}

void InputChangeLogModel::beginRemoveHistory(int first_row, int last_row)
{
    beginRemoveRows(QModelIndex(), first_row, last_row);
}

void InputChangeLogModel::endRemoveHistory()
{
    endRemoveRows();
    updateChangeLog();
}

// 
// void InputChangeLogModel::changeHistory(int frame)
// {
//     emit dataChanged(index(frame-1,0), index(frame,1));
// }
