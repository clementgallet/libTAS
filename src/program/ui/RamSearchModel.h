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

#ifndef LIBTAS_RAMSEARCHMODEL_H_INCLUDED
#define LIBTAS_RAMSEARCHMODEL_H_INCLUDED

#include "ramsearch/CompareOperations.h"
#include "ramsearch/MemScanner.h"
#include "ramsearch/MemValue.h"

#include <QtCore/QAbstractTableModel>
#include <QtGui/QBrush>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sstream>
#include <fstream>
#include <iostream>

/* Forward declaration */
struct Context;

class RamSearchModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RamSearchModel(Context* c, QObject *parent = Q_NULLPTR);

    void update();

    /* Memory scanner */
    MemScanner memscanner;

    /* Type of the searched values */
    int value_type;
    
    /* Flag if we display values in hex or decimal */
    bool hex;

    /* Comparison parameters so that we can display witch addresses would be
     * removed by the search */
    CompareType compare_type;

    /* Perform a new search and returns the error code */
    int newWatches(int mem_flags, int type, int alignment, CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv, uintptr_t ba, uintptr_t ea);

    /* Precompute the size of the next scan (for progress bar) */
    int predictScanCount(int mem_flags);
    
    /* Total number of scan results */
    uint64_t scanCount();

    /* Total size of scan results (in bytes) */
    uint64_t scanSize();

    /* Perform a following search and returns the error code */
    int searchWatches(CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv);

    /* Return the address of the given row, used to fill ramwatch */
    uintptr_t address(int row);
    
    void updateParameters(CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv);
    
    /* Clear all scan results */
    void clear();

    /* Force stop the search */
    void stopSearch();

private:
    Context *context;

    QColor unmatchedColor;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif
