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

#ifndef LIBTAS_INPUTCHANGELOGMODEL_H_INCLUDED
#define LIBTAS_INPUTCHANGELOGMODEL_H_INCLUDED

#include <QtCore/QAbstractItemModel>

/* Forward declaration */
struct Context;
class MovieFile;

class InputChangeLogModel : public QAbstractItemModel {
    Q_OBJECT

public:
    InputChangeLogModel(Context* c, MovieFile* m, QObject *parent = Q_NULLPTR);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    Context *context;
    MovieFile *movie;
    int disabledUndoRow;
    int disabledRedoRow;
    
public slots:
    void updateChangeLog();
    // void endResetHistory();
    // void beginAddHistory(int frame);
    // void endAddHistory();
    // void beginRemoveHistory(int first_frame, int last_frame);
    // void endRemoveHistory();
    // void changeHistory(int frame);
};

#endif
