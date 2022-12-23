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

#ifndef LIBTAS_TIMETRACEMODEL_H_INCLUDED
#define LIBTAS_TIMETRACEMODEL_H_INCLUDED

#include <QtCore/QAbstractTableModel>
#include <vector>
#include <map>
#include <memory>
#include <sys/types.h>
#include <stdint.h>

/* Forward declaration */
struct Context;

struct TimeCall {
    int type;
    unsigned int count;
    std::string stacktrace;
};

class TimeTraceModel : public QAbstractTableModel {
    Q_OBJECT

public:
    TimeTraceModel(Context* c, QObject *parent = Q_NULLPTR);

    /* Map of pointers (key) and addresses (value) */
    std::map<uint64_t,TimeCall> time_calls_map;

    /* Get the full stack trace of a given table index */
    std::string getStacktrace(int index);

    /* Clear the whole table */
    void clearData();

public slots:
    void addCall(int type, unsigned long long hash, std::string stacktrace);

private:
    Context *context;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif
