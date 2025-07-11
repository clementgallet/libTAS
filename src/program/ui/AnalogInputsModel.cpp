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

#include "AnalogInputsModel.h"

#include "utils.h"
#include "Context.h"
#include "KeyMapping.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <QtGui/QColor>
#include <QtGui/QPalette>
#include <QtGui/QBrush>
#include <QtGui/QGuiApplication>

static const char mimeType[] = "application/x-libtas-analoginput";

AnalogInputsModel::AnalogInputsModel(Context* c, QObject *parent) : QAbstractListModel(parent), context(c) {}

int AnalogInputsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return context->config.km->input_list[KeyMapping::INPUTLIST_ANALOG].size();
}

QVariant AnalogInputsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        return QString("Analog Input");
    }
    return QVariant();
}

QVariant AnalogInputsModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return QString(context->config.km->input_list[KeyMapping::INPUTLIST_ANALOG][index.row()].description.c_str());
    }

    return QVariant();
}

Qt::ItemFlags AnalogInputsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (index.isValid())
        flags |= Qt::ItemIsDragEnabled;

    return flags;
}

Qt::DropActions AnalogInputsModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

QStringList AnalogInputsModel::mimeTypes() const
{
    return {QString::fromLatin1(mimeType)};
}

QMimeData *AnalogInputsModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.empty())
        return nullptr;

    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    
    SingleInput si = context->config.km->input_list[KeyMapping::INPUTLIST_ANALOG][indexes[0].row()];
    
    stream << si.type << si.which << QString(si.description.c_str());

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(mimeType, encodedData);
    return mimeData;
}
