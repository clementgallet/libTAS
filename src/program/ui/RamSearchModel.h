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
#include "../ramsearch/CompareOperations.h"
#include "../ramsearch/MemScanner.h"

class RamSearchModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RamSearchModel(Context* c, QObject *parent = Q_NULLPTR);

    void update();

    /* Memory scanner */
    MemScanner memscanner;

    /* Flag if we display values in hex or decimal */
    bool hex;

    /* Comparison parameters so that we can display with addresses would be
     * removed by the search */
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;
    double different_value;

    void newWatches(int mem_flags, int type, CompareType ct, CompareOperator co, double cv, double dv);

    /* Precompute the size of the next scan (for progress bar) */
    int predictScanCount(int mem_flags);
    
    /* Total number of scan results */
    uint64_t scanCount();

    /* Total size of scan results (in bytes) */
    uint64_t scanSize();
    
    void searchWatches(CompareType ct, CompareOperator co, double cv, double dv);

    /* Return the address of the given row, used to fill ramwatch */
    uintptr_t address(int row);
    
    /* Clear all scan results */
    void clear();

private:
    Context *context;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif
