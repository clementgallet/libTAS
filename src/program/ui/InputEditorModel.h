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

#ifndef LIBTAS_INPUTEDITORMODEL_H_INCLUDED
#define LIBTAS_INPUTEDITORMODEL_H_INCLUDED

#include <QtCore/QAbstractTableModel>
#include <vector>
#include <stdint.h>

#include "../Context.h"
#include "../movie/MovieFile.h"

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

    /* Reset the content of the table */
    void resetInputs();

    /* Build the unique set of inputs present in the movie */
    void buildInputSet();

    /* Return the current label of an input */
    std::string inputLabel(int column);

    /* Rename the label of an input */
    void renameLabel(int column, std::string label);

    /* Return the original description of an input */
    std::string inputDescription(int column);

    /* Return if a column contains an analog input */
    bool isInputAnalog(int column);

    /* Add an input column */
    void addUniqueInput(const SingleInput &si);

    /* Add all new input columns from an AllInput object */
    void addUniqueInputs(const AllInputs &ai);

    /* Clear a single input from the entire movie */
    void clearUniqueInput(int column);

    /* Remove a single input from the entire movie */
    void removeUniqueInput(int column);

    /* Get lock status of a single input */
    bool isLockedUniqueInput(int column);

    /* Lock or unlock a single input */
    void lockUniqueInput(int column, bool locked);

    /* Clear input */
    void clearInput(int row);

    /* Clear clipboard before copying inputs */
    void clearClipboard();

    /* Copy selected inputs */
    void copyInputs(int row, int count);

    /* Paste selected inputs. Returns the number of pasted inputs */
    int pasteInputs(int row);

    /* Paste inputs inside the selected range */
    void pasteInputsInRange(int row, int count);

    /* Paste insert selected inputs. Returns the number of pasted inputs */
    int pasteInsertInputs(int row);

    /* User moved a column */
    void moveInputs(int oldIndex, int newIndex);

    /* Rewind to frame, specify if it was called from input toggle or seeking
     * return if succeeded */
    bool rewind(uint64_t framecount, bool toggle);

    /* Returns if scroll is frozen */
    bool isScrollFreeze();

    /* Set scroll freeze state */
    void setScrollFreeze(bool state);

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
    void beginEditInputs(unsigned long long framecount);

    /* End edit inputs */
    void endEditInputs(unsigned long long framecount);

    /* Register a savestate for display */
    void registerSavestate(int slot, unsigned long long frame);

    /* Invalidate all savestates */
    void invalidateSavestates();

private:
    Context *context;
    MovieFile *movie;

    /* Last saved/loaded state */
    unsigned long long last_savestate = 0;

    /* Framecount of the last invalidation */
    uint64_t invalid_frame = 0;

    /* Freeze the vertical scroll, used for rewind */
    bool freeze_scroll = false;

signals:
    void inputSetChanged();

};

#endif
