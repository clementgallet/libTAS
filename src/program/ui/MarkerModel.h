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

#ifndef LIBTAS_MARKERMODEL_H_INCLUDED
#define LIBTAS_MARKERMODEL_H_INCLUDED

#include <QtCore/QAbstractTableModel>
#include <vector>
#include <sstream>
#include <stdint.h>

/* Forward declaration */
struct Context;
class MovieFile;
class SingleInput;
class AllInputs;

class MarkerModel : public QAbstractTableModel {
    Q_OBJECT

public:
    
    MarkerModel(Context* c, MovieFile* m, QObject *parent = Q_NULLPTR);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    // bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /* Reset the content of the table */
    void resetMarkers();

public slots:
    void addMarker(int frame, QString text);
    void removeMarker(int frame);

private:
    Context *context;
    MovieFile *movie;

signals:
    void inputSetChanged();

};

#endif
