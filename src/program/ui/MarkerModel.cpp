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

#include "MarkerModel.h"
#include "qtutils.h"

#include "Context.h"
#include "movie/MovieFile.h"
#include "SaveStateList.h"

#include <QtGui/QBrush>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>

MarkerModel::MarkerModel(Context* c, MovieFile* m, QObject *parent) : QAbstractTableModel(parent), context(c), movie(m) {}

int MarkerModel::rowCount(const QModelIndex & /*parent*/) const
{
    return movie->editor->markers.size();
}

int MarkerModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 2;
}

Qt::ItemFlags MarkerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    if (index.column() == 1)
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractItemModel::flags(index);
}

QVariant MarkerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0)
                return QString(tr("Frame"));
            if (section == 1)
                return QString(tr(""));
        }
    }

    return QVariant();
}

QVariant MarkerModel::data(const QModelIndex &index, int role) const
{
    unsigned int row = index.row();

    if (row >= movie->editor->markers.size())
        return QVariant();

    auto it = movie->editor->markers.cbegin();
    std::advance(it, row);
    int framecount = it->first;
    const std::string& text = it->second;
    
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role == Qt::BackgroundRole) {
        /* Main color */
        QColor color = QGuiApplication::palette().window().color();
        bool lightTheme = isLightTheme();
        int r, g, b;
        color.getRgb(&r, &g, &b, nullptr);
        
        /* Greenzone */
        uint64_t root_frame = SaveStateList::rootStateFramecount();
        bool greenzone = (framecount < context->framecount) && root_frame && (framecount >= root_frame);

        if (lightTheme) {
            /* Light theme */
            if (framecount == context->framecount)
                color.setRgb(r - 0x30, g - 0x10, b);
            else if (framecount < context->framecount) {
                if (greenzone)
                    color.setRgb(r - 0x30, g, b - 0x30);
                else
                    color.setRgb(r - 0x20, g, b - 0x20);
            }
            else {
                color.setRgb(r, g, b - 0x10);
            }
        }
        else {
            /* Dark theme */
            if (framecount == context->framecount)
                color.setRgb(r, g + 0x10, b + 0x20);
            else if (framecount < context->framecount) {
                if (greenzone)
                    color.setRgb(r, g + 0x30, b);
                else
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
            return framecount;
        }
        if (index.column() == 1) {
            return QString(text.c_str());
        }
    }

    if (role == Qt::EditRole) {
        if (index.column() == 0) {
            return QVariant();
        }
        if (index.column() == 1) {
            return QString(text.c_str());
        }
    }

    return QVariant();
}

bool MarkerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        if (index.column() != 1)
            return false;

        unsigned int row = index.row();
        if (row >= movie->editor->markers.size())
            return false;
        
        auto it = movie->editor->markers.begin();
        std::advance(it, row);
        it->second = value.toString().toStdString();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

void MarkerModel::resetMarkers()
{
    beginResetModel();
    endResetModel();
}

void MarkerModel::addMarker(int frame, QString text)
{
    int row = 0;
    bool is_new = false;
    for (auto it = movie->editor->markers.begin(); it != movie->editor->markers.end(); row++, it++) {
        if (it->first >= frame) {
            is_new = (it->first != frame);
            break;
        }
    }
    
    if (is_new)
        beginInsertRows(QModelIndex(), row, row);
        
    movie->editor->markers[frame] = text.toStdString();
    
    if (is_new)
        endInsertRows();
    else
        emit dataChanged(index(row, 0), index(row, 1));
}

void MarkerModel::removeMarker(int frame)
{
    auto it = movie->editor->markers.find(frame);
    if (it == movie->editor->markers.end())
        return;
    
    int row = std::distance(movie->editor->markers.begin(), it);
    
    beginRemoveRows(QModelIndex(), row, row);
    movie->editor->markers.erase(it);
    endRemoveRows();    
}

bool MarkerModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row+count-1);

    auto it = movie->editor->markers.cbegin();
    if (count == 1) {
        std::advance(it, row);
        movie->editor->markers.erase(it);
    }
    else {
        movie->editor->markers.erase(std::next(it, row), std::next(it, row+count-1));
    }
    endRemoveRows();
    return true;
}
