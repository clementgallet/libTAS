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

#include "TimeTraceModel.h"
#include "../utils.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <QtGui/QColor>
#include <QtGui/QPalette>
#include <QtGui/QBrush>
#include <QtGui/QGuiApplication>

TimeTraceModel::TimeTraceModel(Context* c, QObject *parent) : QAbstractTableModel(parent), context(c) {}

void TimeTraceModel::addCall(int type, unsigned long long hash, std::string stacktrace)
{
    auto it = time_calls_map.find(hash);
    if (it != time_calls_map.end()) {
        if ((!stacktrace.empty()) && stacktrace.compare(it->second.stacktrace) != 0) {
            std::cerr << "Same hash but stack trace differ!" << std::endl;
            std::cerr << "Stored trace:" << std::endl;
            std::cerr << it->second.stacktrace << std::endl;
            std::cerr << "New trace:" << std::endl;
            std::cerr << stacktrace << std::endl;
        }
        it->second.count++;
        /* TODO: Get the row index? */
        emit dataChanged(index(0,1), index(rowCount()-1,1));
    }
    else {
        beginInsertRows(QModelIndex(), time_calls_map.size(), time_calls_map.size());
        time_calls_map[hash] = {type, 1, stacktrace};
        endInsertRows();
    }
}

int TimeTraceModel::rowCount(const QModelIndex & /*parent*/) const
{
    return time_calls_map.size();
}

int TimeTraceModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant TimeTraceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0) {
                return QString("Function");
            }
            else if (section == 1) {
                return QString("Hash");
            }
            return QString("Count");
        }
    }
    return QVariant();
}

QVariant TimeTraceModel::data(const QModelIndex &index, int role) const
{
    auto it = time_calls_map.begin();
    std::advance(it, index.row());

    if (role == Qt::BackgroundRole) {
        QColor color = QGuiApplication::palette().window().color();
        int r, g, b;
        color.getRgb(&r, &g, &b, nullptr);
        if (color.lightness() > 128) {
            /* Light theme */
            if (it->first == context->config.sc.busy_loop_hash)
                color.setRgb(r - 0x30, g - 0x10, b);
            else
                color.setRgb(r, g, b - 0x18);
        }
        else {
            /* Dark theme */
            if (it->first == context->config.sc.busy_loop_hash)
                color.setRgb(r, g + 0x10, b + 0x20);
            else
                color.setRgb(r + 0x08, g + 0x08, b);
        }
        return QBrush(color);
    }
    else if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            switch (it->second.type) {
                case SharedConfig::TIMETYPE_TIME:
                    return tr("time()");
                case SharedConfig::TIMETYPE_GETTIMEOFDAY:
                    return tr("gettimeofday()");
                case SharedConfig::TIMETYPE_CLOCK:
                    return tr("clock()");
                case SharedConfig::TIMETYPE_CLOCKGETTIME:
                    return tr("clock_gettime()");
                case SharedConfig::TIMETYPE_SDLGETTICKS:
                    return tr("SDL_GetTicks()");
                case SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER:
                    return tr("SDL_GetPerformanceCounter()");
                case SharedConfig::TIMETYPE_GETTICKCOUNT:
                    return tr("GetTickCount()");
                case SharedConfig::TIMETYPE_GETTICKCOUNT64:
                    return tr("GetTickCount64()");
                case SharedConfig::TIMETYPE_QUERYPERFORMANCECOUNTER:
                    return tr("QueryPerformanceCounter()");
                default:
                    return tr("Unknown");
            }
        }
        else if (index.column() == 1) {
            return QString("%1").arg(it->first, 0, 16);
        }
        else {
            return it->second.count;
        }
    }
    return QVariant();
}

std::string TimeTraceModel::getStacktrace(int index)
{
    auto it = time_calls_map.begin();
    std::advance(it, index);
    if (it == time_calls_map.end())
        return std::string("");

    return it->second.stacktrace;
}

void TimeTraceModel::clearData()
{
    beginResetModel();
    time_calls_map.clear();
    endResetModel();
}
