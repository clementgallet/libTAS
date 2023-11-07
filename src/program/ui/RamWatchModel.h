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

#ifndef LIBTAS_RAMWATCHMODEL_H_INCLUDED
#define LIBTAS_RAMWATCHMODEL_H_INCLUDED

#include "ramsearch/IRamWatchDetailed.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QSettings>
#include <vector>
#include <memory>

class RamWatchModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RamWatchModel(QObject *parent = Q_NULLPTR);

    /* A reference to the vector of addresses to watch */
    std::vector<std::unique_ptr<IRamWatchDetailed>> ramwatches;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addWatch(std::unique_ptr<IRamWatchDetailed> ramwatch);
    void removeWatch(int row);

    void saveSettings(QSettings& watchSettings);
    void loadSettings(QSettings& watchSettings);

    void update();
};

#endif
