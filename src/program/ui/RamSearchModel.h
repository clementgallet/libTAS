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

#ifndef LIBTAS_RAMSEARCHMODEL_H_INCLUDED
#define LIBTAS_RAMSEARCHMODEL_H_INCLUDED

#include <QtCore/QAbstractTableModel>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sstream>
#include <fstream>
#include <iostream>

#include "../Context.h"
#include "../ramsearch/CompareEnums.h"
#include "../ramsearch/RamWatch.h"
#include "../ramsearch/MemSection.h"

class RamSearchModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RamSearchModel(Context* c, QObject *parent = Q_NULLPTR);

    void update();

    /* List of watches */
    std::vector<RamWatch> ramwatches;

    /* Flag if we display values in hex or decimal */
    bool hex;

    /* Comparison parameters so that we can display with addresses would be
     * removed by the search */
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    double different_value;

    // template <class T>
    // void new_watches(pid_t pid, int type_filter, CompareType compare_type, CompareOperator compare_operator, double compare_value, Fl_Hor_Fill_Slider *search_progress)
    void newWatches(int mem_filter, int type, CompareType ct, CompareOperator co, double cv, double dv);

    int predictWatchCount(int type_filter);
    int watchCount();
    void searchWatches(CompareType ct, CompareOperator co, double cv, double dv);

private:
    Context *context;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:
    void signalProgress(int);

};

#endif
