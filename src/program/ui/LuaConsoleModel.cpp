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

// #include <QtWidgets/QMessageBox>
#include "LuaConsoleModel.h"
#include "../lua/Callbacks.h"
#include "../lua/LuaFunctionList.h"
#include <iostream>
#include <stdint.h>

LuaConsoleModel::LuaConsoleModel(QObject *parent) : QAbstractTableModel(parent) {}

int LuaConsoleModel::rowCount(const QModelIndex & /*parent*/) const
{
    const Lua::LuaFunctionList& lfl = Lua::Callbacks::getList();
    return lfl.fileSet.size();
}

int LuaConsoleModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

Qt::ItemFlags LuaConsoleModel::flags(const QModelIndex &index) const
{
    if (index.column() == 0)
        return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
    return QAbstractTableModel::flags(index);
}

QVariant LuaConsoleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QVariant();
            case 1:
                return QString("Name");
            case 2:
                return QString("Path");
            }
        }
    }
    return QVariant();
}

QVariant LuaConsoleModel::data(const QModelIndex &index, int role) const
{
    const Lua::LuaFunctionList& lfl = Lua::Callbacks::getList();

    if (index.column() == 0 && role == Qt::CheckStateRole) {
        if (lfl.activeState(index.row()))
            return Qt::Checked;
        return Qt::Unchecked;        
    }

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0:
                return QString("");
            case 1:
                return QString(lfl.fileList[index.row()].filename.c_str());
            case 2:
                return QString(lfl.fileList[index.row()].file.c_str());
            default:
                return QString();
        }
    }
    return QVariant();
}

bool LuaConsoleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Lua::LuaFunctionList& lfl = Lua::Callbacks::getList();

    if (index.column() == 0 && role == Qt::CheckStateRole) {        
        lfl.switchForFile(index.row(), value == Qt::Checked);
        return true;
    }

    return QAbstractTableModel::setData(index, value, role);
}

void LuaConsoleModel::addFile(const QString& str)
{
    Lua::LuaFunctionList& lfl = Lua::Callbacks::getList();

    /* Check if file is already in the list before calling beginInsertRows */
    if (lfl.fileSet.find(str.toStdString()) != lfl.fileSet.end())
        return;

    beginInsertRows(QModelIndex(), lfl.fileSet.size(), lfl.fileSet.size());
    lfl.addFile(str.toStdString());
    endInsertRows();
}

void LuaConsoleModel::removeFile(int row)
{
    Lua::LuaFunctionList& lfl = Lua::Callbacks::getList();

    beginRemoveRows(QModelIndex(), row, row);
    lfl.removeForFile(row);
    endRemoveRows();
}

void LuaConsoleModel::clear()
{
    Lua::LuaFunctionList& lfl = Lua::Callbacks::getList();

    lfl.clear();
}

void LuaConsoleModel::update()
{
    Lua::Callbacks::getList().watchChanges();
}
