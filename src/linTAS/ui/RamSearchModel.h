/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_RAMSEARCHMODEL_H_INCLUDED
#define LINTAS_RAMSEARCHMODEL_H_INCLUDED

#include <QAbstractTableModel>
#include <vector>
#include <memory>

#include "../ramsearch/IRamWatch.h"
#include "../ramsearch/CompareEnums.h"

// #include <FL/Fl.H>
// #include <FL/Fl_Double_Window.H>
// #include <FL/Fl_Table_Row.H>

class RamSearchModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RamSearchModel(std::vector<std::unique_ptr<IRamWatch>> *ramwatches, QObject *parent = Q_NULLPTR);
    void update();

    /* Flag if we display values in hex or decimal */
    bool hex;

    /* Comparison parameters so that we can display with addresses would be
     * removed by the search */
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value_db;

private:
    /* A reference to the vector of addresses to watch */
    std::vector<std::unique_ptr<IRamWatch>> *ramwatches;


    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

};

#endif
