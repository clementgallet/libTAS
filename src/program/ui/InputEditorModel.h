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

#ifndef LIBTAS_INPUTEDITORMODEL_H_INCLUDED
#define LIBTAS_INPUTEDITORMODEL_H_INCLUDED

#include "../shared/inputs/SingleInput.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QTimer>
#include <QtCore/QMimeData>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <chrono>
#include <map>

/* Forward declaration */
struct Context;
class MovieFile;
class AllInputs;

class InputEditorModel : public QAbstractTableModel {
    Q_OBJECT

public:
    
    enum {
        COLUMN_SAVESTATE = 0,
        COLUMN_FRAME = 1,
        COLUMN_SPECIAL_SIZE,
    };

    enum {
        ROW_EXTRAS = 10,
    };
    
    InputEditorModel(Context* c, MovieFile* m, QObject *parent = Q_NULLPTR);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;
    bool dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    /* Not to mess with row count, which may be larger due to blank rows at the end */
    unsigned int frameCount() const;

    /* Update the content of the table */
    void update();

    /* Build the unique set of inputs present in the movie */
    void buildInputSet();

    /* Start a paint procedure initiated by dragging the mouse */
    void startPaint(int col, int minRow, int maxRow, int value, int autofire);

    /* End paint procedure and set input range if possible */
    void endPaint();

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

    /* Add all new input columns from a range of movie inputs */
    void addUniqueInputs(int minRow, int maxRow);

    /* Add all new input columns from a list of inputs */
    void addUniqueInputs(const std::vector<AllInputs>& new_inputs);
    
    /* Clear a single input from the entire movie */
    void clearUniqueInput(int column);

    /* Remove a single input from the entire movie. Returns if the column was removed */
    bool removeUniqueInput(int column);

    /* Multiple values of a column with a factor */
    void columnFactor(int column, double factor);

    /* Insert rows, and specify if we want to duplicate the selected input row */
    bool insertRows(int row, int count, bool duplicate, const QModelIndex &parent = QModelIndex());

    bool hasMarker(int frame);
    std::string getMarkerText(int frame);

    /* Get lock status of a single input */
    bool isLockedUniqueInput(int column);

    /* Lock or unlock a single input */
    void lockUniqueInput(int column, bool locked);

    /* Clear input */
    void clearInputs(int row, int count);

    /* Copy selected inputs into the stream */
    void copyInputs(int row, int count, std::ostringstream& inputString);

    /* Paste selected inputs. Returns the number of pasted inputs */
    int pasteInputs(int row);

    /* Paste inputs inside the selected range */
    void pasteInputsInRange(int row, int count);

    /* Paste insert selected inputs. Returns the number of pasted inputs */
    int pasteInsertInputs(int row);

    /* User moved a column */
    void moveInputs(int oldIndex, int newIndex);

    /* Returns if scroll is frozen */
    bool isScrollFreeze();

    /* Set scroll freeze state */
    void setScrollFreeze(bool state);

    /* Set autohold of input */
    void setAutoholdInput(int index, bool checked);

    /* Check autohold of input */
    bool isAutoholdInput(int index) const;

    /* Set autofire of input */
    void setAutofireInput(int index, bool checked);

    /* Check autofire of input */
    bool isAutofireInput(int index) const;

public slots:
    /* Prepare for a change of inputs */
    void beginResetInputs();

    /* End change of inputs */
    void endResetInputs();

    /* Prepare for new inputs */
    void beginInsertInputs(int minRow, int maxRow);

    /* End new inputs */
    void endInsertInputs(int minRow, int maxRow);

    /* Prepare for remove inputs */
    void beginRemoveInputs(int minRow, int maxRow);

    /* End remove inputs */
    void endRemoveInputs(int minRow, int maxRow);

    /* Prepare for edit inputs */
    void beginEditInputs(int minRow, int maxRow);

    /* End edit inputs */
    void endEditInputs(int minRow, int maxRow);

    /* Register a savestate for display */
    void registerSavestate(int slot, unsigned long long frame);

    /* Save the new hovered cell */
    void setHoveredCell(const QModelIndex &index);

    /* Seek to marker frame */
    void seekToFrame(unsigned long long frame);

    /* Timer update function to highlight cells */
    void highlightUpdate();

    /* Shift markers after a given row by a specified number of rows */
    void shiftMarkers(int startRow, int offset);

    /* Remove markers within a specified range of rows */
    void removeMarkersInRange(int startRow, int endRow);

private:
    Context *context;
    MovieFile *movie;

    /* Last saved/loaded state */
    unsigned long long last_savestate = 0;

    /* Freeze the vertical scroll, used for rewind */
    bool freeze_scroll = false;

    /* Current hovered cell */
    QModelIndex hoveredIndex;
    
    /* Parameters of the current range being painted */
    SingleInput paintInput;
    bool paintOngoing;
    unsigned int paintMinRow;
    unsigned int paintMaxRow;
    unsigned int paintCol;
    int paintValue;
    unsigned int paintAutofire;

    /* Parameters to highlight some operations */
    unsigned int highlightMinRow, highlightMaxRow, highlightMinCol, highlightMaxCol;
    float highlightTimeoutSec;
    bool highlightValid; // to decide which color to use
    const float maxHighlightTimeoutSec = 0.5f;
    std::chrono::time_point<std::chrono::steady_clock> highlightStart;
    QTimer* highlightTimer;

    /* Rewind to frame, specify if we should enforce seeking to that frame, or
     * follow the setting. Return if succeeded */
    bool rewind(uint64_t framecount, bool enforce_seek);

    /* Rewind without enforce seeking */
    bool rewind(uint64_t framecount);

    /* Highlight a change in the input editor */
    void startHighlight(int minRow, int maxRow, int minCol, int maxCol, bool valid);

    /* Map to store markers with frame index as key and marker text as value */
    std::map<int, std::string> markers;

signals:
    void inputSetChanged();
    void stateLoaded();

};

#endif
