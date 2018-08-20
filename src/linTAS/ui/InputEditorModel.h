/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_INPUTEDITORMODEL_H_INCLUDED
#define LINTAS_INPUTEDITORMODEL_H_INCLUDED

#include <QAbstractTableModel>
#include <vector>

#include "../Context.h"
#include "../MovieFile.h"

class InputEditorModel : public QAbstractTableModel {
    Q_OBJECT

public:
    InputEditorModel(Context* c, MovieFile* m, QObject *parent = Q_NULLPTR);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /* Update the content of the table */
    void update();

    /* Build the unique set of inputs present in the movie */
    void buildInputSet();

    /* Return the current label of an input */
    std::string inputLabel(int column);

    /* Rename the label of an input */
    void renameLabel(int column, std::string label);

    /* Return the original description of an input */
    std::string inputDescription(int column);

    /* Add an input column */
    void addUniqueInput(const SingleInput &si);

    /* Add all new input columns from an AllInput object */
    void addUniqueInputs(const AllInputs &ai);

    /* Clear input */
    void clearInput(int row);

    /* Copy selected inputs */
    void copyInputs(int row, int count);

    /* Cut selected inputs */
    void cutInputs(int row, int count);

    /* Paste selected inputs. Returns the number of pasted inputs */
    int pasteInputs(int row);

    /* Paste insert selected inputs. Returns the number of pasted inputs */
    int pasteInsertInputs(int row);

public slots:
    /* Toggle a single input and return the new value */
    bool toggleInput(const QModelIndex &index);

    /* Prepare for a change of inputs */
    void beginModifyInputs();

    /* End change of inputs */
    void endModifyInputs();

    /* Prepare for new inputs */
    void beginAddInputs();

    /* End new inputs */
    void endAddInputs();

    /* Prepare for edit inputs */
    void beginEditInputs();

    /* End edit inputs */
    void endEditInputs();

private:
    Context *context;
    MovieFile *movie;

    /* Set of inputs present in the movie */
    std::vector<SingleInput> input_set;

signals:
    void frameCountChanged();

};

#endif
