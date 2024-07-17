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

#include "InputEventModel.h"

#include "utils.h"
#include "Context.h"

InputEventModel::InputEventModel(Context* c, QObject *parent) : QAbstractTableModel(parent), context(c) {}

int InputEventModel::rowCount(const QModelIndex & /*parent*/) const
{
    return events.size();
}

int InputEventModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant InputEventModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0) {
                return QString("Type");
            }
            else if (section == 1) {
                return QString("Which");
            }
            return QString("Value");
        }
    }
    return QVariant();
}

Qt::ItemFlags InputEventModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant InputEventModel::data(const QModelIndex &index, int role) const
{
    const auto& event = events[index.row()];

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return tr(SingleInput::typeToStr(event.type));
        }
        else if (index.column() == 1) {
            switch (event.type) {
                case SingleInput::IT_KEYBOARD:
                    return QString(context->config.km->input_description(event.which).c_str());
                case SingleInput::IT_POINTER_BUTTON:
                    return QString("Button %1").arg(event.which+1);
                case SingleInput::IT_CONTROLLER1_BUTTON:
                case SingleInput::IT_CONTROLLER2_BUTTON:
                case SingleInput::IT_CONTROLLER3_BUTTON:
                case SingleInput::IT_CONTROLLER4_BUTTON:
                    return tr(SingleInput::buttonToStr(event.which));
                case SingleInput::IT_CONTROLLER1_AXIS:
                case SingleInput::IT_CONTROLLER2_AXIS:
                case SingleInput::IT_CONTROLLER3_AXIS:
                case SingleInput::IT_CONTROLLER4_AXIS:
                    return tr(SingleInput::axisToStr(event.which));
            }
        }
        else {
            return QVariant(event.value);
        }
    }
    else if (role == Qt::EditRole) {
        if (index.column() == 0) {
            return event.type;
        }
        else if (index.column() == 1) {
            return ((unsigned long long)event.type << 32ULL) | event.which;
        }
        else {
            return event.value;
        }
    }
    return QVariant();
}

bool InputEventModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        auto& event = events[index.row()];

        if (index.column() == 0) {
            /* Custom item delegate sends the combobox **data** (being set to 
             * the SingleInput type and not the chosen string, so that we can 
             * store the data without an extra conversion */
            event.type = value.toInt();
        }
        else if (index.column() == 1) {
            event.which = value.toInt();
        }
        else {
            event.value = value.toInt();
        }
        return true;
    }
    return false;
}

void InputEventModel::setEvents(const std::vector<InputEvent>& frame_events)
{
    beginResetModel();
    events = frame_events;
    endResetModel();
}

void InputEventModel::append()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    events.push_back({0, 0, 0});
    endInsertRows();
}

void InputEventModel::duplicate(int min_row, int max_row)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+(max_row-min_row));
    for (int r = min_row; r <= max_row; r++)
        events.push_back(events[r]);
    endInsertRows();
}

void InputEventModel::remove(int min_row, int max_row)
{
    beginRemoveRows(QModelIndex(), min_row, max_row);
    events.erase(events.begin()+min_row, events.begin()+max_row+1);
    endRemoveRows();
}

void InputEventModel::clear()
{
    beginResetModel();
    events.clear();
    endResetModel();
}
