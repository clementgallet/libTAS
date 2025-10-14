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

#include "InputEditorModel.h"
#include "qtutils.h"

#include "Context.h"
#include "movie/MovieFile.h"
#include "movie/InputSerialization.h"
#include "SaveStateList.h"
#include "SaveState.h"
#include "../shared/inputs/SingleInput.h"
#include "../shared/inputs/AllInputs.h"

#include <QtGui/QBrush>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#include <QtGui/QFont>
#include <sstream>
#include <iostream>
#include <set>

InputEditorModel::InputEditorModel(Context* c, MovieFile* m, QObject *parent) : QAbstractTableModel(parent), context(c), movie(m) {
    connect(movie->inputs, &MovieFileInputs::inputsToBeRemoved, this, &InputEditorModel::beginRemoveInputs);
    connect(movie->inputs, &MovieFileInputs::inputsRemoved, this, &InputEditorModel::endRemoveInputs);
    connect(movie->inputs, &MovieFileInputs::inputsToBeInserted, this, &InputEditorModel::beginInsertInputs);
    connect(movie->inputs, &MovieFileInputs::inputsInserted, this, &InputEditorModel::endInsertInputs);
    connect(movie->inputs, &MovieFileInputs::inputsToBeEdited, this, &InputEditorModel::beginEditInputs);
    connect(movie->inputs, &MovieFileInputs::inputsEdited, this, &InputEditorModel::endEditInputs);
    connect(movie->inputs, &MovieFileInputs::inputsToBeReset, this, &InputEditorModel::beginResetInputs);
    connect(movie->inputs, &MovieFileInputs::inputsReset, this, &InputEditorModel::endResetInputs);

    paintOngoing = false;
    undoTimeoutSec = 0;
    undoTimer = new QTimer(this);
    connect(undoTimer, &QTimer::timeout, this, &InputEditorModel::highlightUndo);
}

unsigned int InputEditorModel::frameCount() const
{
    return movie->inputs->nbFrames();
}

int InputEditorModel::rowCount(const QModelIndex & /*parent*/) const
{
    return frameCount() + ROW_EXTRAS;
}

int InputEditorModel::columnCount(const QModelIndex & /*parent*/) const
{
    return movie->editor->input_set.size() + COLUMN_SPECIAL_SIZE;
}

Qt::ItemFlags InputEditorModel::flags(const QModelIndex &index) const
{
    unsigned int row = index.row();
    Qt::ItemFlags index_flags = QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

    if (!index.isValid())
        return index_flags;

    if (index.column() < COLUMN_SPECIAL_SIZE)
        return index_flags;

    /* Don't toggle past inputs before root savestate */
    uint64_t root_frame = SaveStateList::rootStateFramecount();
    if (!root_frame) {
        if (row < context->framecount)
            return index_flags;
    }
    else {
        if (row < root_frame)
            return index_flags;
    }

    if (row >= frameCount())
        return index_flags;

    const AllInputs& ai = movie->inputs->getInputs(row);
    const SingleInput si = movie->editor->input_set[index.column()-COLUMN_SPECIAL_SIZE];

    /* Don't edit locked input */
    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        return index_flags;

    /* Don't edit inputs that have events */
    if (!ai.events.empty())
        return index_flags;

    if (si.isAnalog())
        return index_flags | Qt::ItemIsEditable;

    return index_flags;
}

QVariant InputEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == COLUMN_SAVESTATE)
                return QString(tr(""));
            if (section == COLUMN_FRAME)
                return QString(tr("Frame"));
            if (static_cast<unsigned int>(section-COLUMN_SPECIAL_SIZE) < movie->editor->input_set.size())
                return QString(movie->editor->input_set[section-COLUMN_SPECIAL_SIZE].description.c_str());
        }
    }
    
    if (role == Qt::BackgroundRole) {
        if (orientation == Qt::Horizontal) {
            if (section >= COLUMN_SPECIAL_SIZE) {
                /* Main color */
                QColor color = QGuiApplication::palette().window().color();
                bool lightTheme = isLightTheme();
                
                /* Highlight current column */
                if (hoveredIndex.isValid() && (section == hoveredIndex.column())) {
                    if (lightTheme)
                        color = color.darker(110);
                    else
                        color = color.lighter(110);

                }

                /* Highlight autohold columns */
                if (isAutoholdInput(section)) {
                    int r, g, b;
                    color.getRgb(&r, &g, &b, nullptr);

                    if (lightTheme)
                        color.setRgb(r - 0x10, g - 0x10, b);
                    else
                        color.setRgb(r, g, b + 0x10);
                }
                else if (isAutofireInput(section)) {
                    int r, g, b;
                    color.getRgb(&r, &g, &b, nullptr);

                    if (lightTheme)
                        color.setRgb(r, g - 0x10, b);
                    else
                        color.setRgb(r + 0x10, g, b + 0x10);
                }

                return QBrush(color);
            }
        }
    }

    return QVariant();
}

QVariant InputEditorModel::data(const QModelIndex &index, int role) const
{
    unsigned int row = index.row();
    unsigned int col = index.column();
    
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role == Qt::FontRole) {
        QFont font;
        if (col == COLUMN_SAVESTATE && row == last_savestate) {
            font.setBold(true);
        }
        else if (col == COLUMN_FRAME && movie->editor->markers.count(row)) {
            font.setStretch(QFont::ExtraExpanded);
            font.setBold(true);
        }
        return font;
    }

    if (role == Qt::ForegroundRole) {
        if (row >= frameCount()) {
            return QGuiApplication::palette().text();
        }
        if (col < COLUMN_SPECIAL_SIZE) {
            return QGuiApplication::palette().text();
        }

        QColor color = QGuiApplication::palette().text().color();
        const SingleInput si = movie->editor->input_set[col-COLUMN_SPECIAL_SIZE];
        const AllInputs& ai = movie->inputs->getInputs(row);
        int current_value = ai.getInput(si);

        /* Show inputs with transparancy when they are pending due to rewind */
        bool pending_input = false;
        // movie->inputs->input_queue.lock();
        // for (auto it = movie->inputs->input_queue.begin(); it != movie->inputs->input_queue.end(); it++) {
        //     if (it->framecount != row)
        //         continue;
        // 
        //     if (si == it->si) {
        //         pending_input = true;
        //         /* For analog, use half-transparancy. Otherwise,
        //          * use strong/weak transparancy of set/clear input */
        //         if (si.isAnalog()) {
        //             color.setAlpha(128);
        //         }
        //         else {
        //             int alpha = 0;
        //             if (it->value) alpha += 192;
        //             if (current_value) alpha += 63;
        //             color.setAlpha(alpha);
        //         }
        //         /* We don't return the brush immediatly, because users may change
        //          * multiple times the same input. */
        //     }
        // }
        // movie->inputs->input_queue.unlock();

        /* Show the current paint operation */
        if (paintOngoing) {
            if ((row >= paintMinRow) && (row <= paintMaxRow) && (si == paintInput)) {
                pending_input = true;
                /* For analog, use half-transparancy. Otherwise,
                 * use strong/weak transparancy of set/clear input */
                if (si.isAnalog()) {
                    color.setAlpha(128);
                }
                else {
                    int alpha = 0;
                    int newpaintValue = paintValue;
                    if ((((row+1)%2)+1) == paintAutofire) newpaintValue = !paintValue;
                    if (newpaintValue) alpha += 192;
                    if (current_value) alpha += 63;
                    color.setAlpha(alpha);
                }
            }
        }

        /* If hovering on the cell, show a preview of the input for the following:
         * - the cell is blank
         * - not analog input
         * - not pending input due to rewind */
        if (!pending_input &&
                (int)col == hoveredIndex.column() &&
                (int)row == hoveredIndex.row() &&
                !si.isAnalog()) {
            const AllInputs& ai = movie->inputs->getInputs(row);
            int value = ai.getInput(si);
            if (!value) {
                color.setAlpha(128);
            }
        }
        
        return QBrush(color);
    }

    if (role == Qt::BackgroundRole) {
        /* Main color */
        QColor color = QGuiApplication::palette().window().color();
        
        if (row >= frameCount())
            return QBrush(color);
            
        bool lightTheme = isLightTheme();
        int r, g, b;
        color.getRgb(&r, &g, &b, nullptr);
        
        /* Greenzone */
        uint64_t root_frame = SaveStateList::rootStateFramecount();
        bool greenzone = (row < context->framecount) && root_frame && (row >= root_frame);

        if (lightTheme) {
            /* Light theme */
            if (row == context->framecount)
                color.setRgb(r - 0x30, g - 0x10, b);
            else if (row < context->framecount) {
                if (movie->editor->isDraw(row)) {                    
                    if (greenzone)
                        color.setRgb(r - 0x30, g, b - 0x30);
                    else
                        color.setRgb(r - 0x20, g, b - 0x20);
                }
                else {
                    if (greenzone)
                        color.setRgb(r, g - 0x20, b - 0x20);
                    else
                        color.setRgb(r, g - 0x18, b - 0x18);
                }
            }
            else {
                if (movie->editor->isDraw(row))
                    color.setRgb(r, g, b - 0x10);
                else
                    color.setRgb(r, g - 0x10, b - 0x10);
            }
        }
        else {
            /* Dark theme */
            if (row == context->framecount)
                color.setRgb(r, g + 0x10, b + 0x20);
            else if (row < context->framecount) {
                if (movie->editor->isDraw(row)) {
                    if (greenzone)
                        color.setRgb(r, g + 0x30, b);
                    else
                        color.setRgb(r, g + 0x18, b);
                }
                else {
                    if (greenzone)
                        color.setRgb(r + 0x30, g, b);
                    else
                        color.setRgb(r + 0x18, g, b);
                }
            }
            else {
                if (movie->editor->isDraw(row))
                    color.setRgb(r + 0x08, g + 0x08, b);
                else
                    color.setRgb(r + 0x08, g, b);
            }
        }

        /* Special columns */
        if (col < COLUMN_SPECIAL_SIZE) {
            color = color.lighter(105);
        }
        else {
            /* Check for locked input */
            if (!movie->editor->locked_inputs.empty()) {
                const SingleInput si = movie->editor->input_set[col-COLUMN_SPECIAL_SIZE];
                if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end()) {
                    color = color.darker(150);
                }
            }
            
            /* Highlight autohold columns */
            if (isAutoholdInput(col)) {
                int r, g, b;
                color.getRgb(&r, &g, &b, nullptr);

                if (lightTheme)
                    color.setRgb(r - 0x10, g - 0x10, b);
                else
                    color.setRgb(r, g, b + 0x10);
            }
            else if (isAutofireInput(col)) {
                int r, g, b;
                color.getRgb(&r, &g, &b, nullptr);

                if (lightTheme)
                    color.setRgb(r, g - 0x10, b);
                else
                    color.setRgb(r + 0x10, g, b + 0x10);
            }
        }
        
        /* Frame containing a savestate */
        int savestate_frame = SaveStateList::stateAtFrame(row);
        if (savestate_frame != -1)
            color = color.darker(105);

        /* Highlight last undo/redo */
        if (undoTimeoutSec > 0.0f) {
            if (row >= undoMinRow && row <= undoMaxRow &&
                col >= undoMinCol && col <= undoMaxCol) {
                
                int r, g, b;
                color.getRgb(&r, &g, &b, nullptr);

                if (isLightTheme()) {
                    color.setRgb(r-60.0f*undoTimeoutSec, g-60.0f*undoTimeoutSec, b);
                }
                else {
                    color.setRgb(r, g, b+60.0f*undoTimeoutSec);
                }
            }
        }

        const AllInputs& ai = movie->inputs->getInputs(row);
//        return QBrush(color, ai.events.empty()?Qt::SolidPattern:Qt::Dense3Pattern);
        return QBrush(color, ai.events.empty()?Qt::SolidPattern:Qt::BDiagPattern);
    }

    if (role == Qt::DisplayRole) {
        if (row >= frameCount()) {
            /* If one of the extra rows, the only thing we can display is a
             * preview of the input and current paint range */
            if (col >= COLUMN_SPECIAL_SIZE) {
                const SingleInput si = movie->editor->input_set[col-COLUMN_SPECIAL_SIZE];

                if ((int)col == hoveredIndex.column() &&
                    (int)row == hoveredIndex.row() &&
                    !si.isAnalog()) {
                    return QString(si.description.c_str());
                }
                
                if (paintOngoing) {
                    if ((row >= paintMinRow) && (row <= paintMaxRow) && (si == paintInput)) {
                        if (si.isAnalog()) {
                            return QString().setNum(paintValue);
                        }
                        else if (paintValue) {
                            return QString(si.description.c_str());
                        }
                    }
                }
            }
            return QVariant();
        }
        
        if (col == COLUMN_SAVESTATE) {
            int savestate_frame = SaveStateList::stateAtFrame(row);
            if (savestate_frame != -1) {
                if (savestate_frame == 10)
                    return QString("B");
                else
                    return savestate_frame;
            }
            return QVariant();
        }
        if (col == COLUMN_FRAME) {
            return row;
        }

        const AllInputs& ai = movie->inputs->getInputs(row);
        const SingleInput si = movie->editor->input_set[col-COLUMN_SPECIAL_SIZE];

        /* Get the value of the single input in movie inputs */
        int value = ai.getInput(si);
        
        /* If hovering on the cell, show a preview of the input */
        if ((int)col == hoveredIndex.column() &&
            (int)row == hoveredIndex.row() &&
            !si.isAnalog()) {
            value = 1;
        }

        /* If the value is currently being modified, load the new value */
        // movie->inputs->input_queue.lock();
        // for (auto it = movie->inputs->input_queue.begin(); it != movie->inputs->input_queue.end(); it++) {
        //     if (it->framecount != row)
        //         continue;
        //     if (si == it->si) {
        //         if (si.isAnalog()) {
        //             value = it->value;
        //         }
        //         else {
        //             /* For non-analog values, always print the value, and the
        //              * transparancy value will indicate if the value is being
        //              * cleared or set. */
        //             value = 1;
        //         }
        //     }
        // }
        // movie->inputs->input_queue.unlock();

        /* If the current value is being painted, load the new value */
        if (paintOngoing) {
            if ((row >= paintMinRow) && (row <= paintMaxRow) && (si == paintInput)) {
                if (si.isAnalog()) {
                    value = paintValue;
                }
                else {
                    /* For non-analog values, always print the value, and the
                     * transparancy value will indicate if the value is being
                     * cleared or set. */
                    value = 1;
                }
            }
        }

        if (si.isAnalog()) {
            /* Default framerate has a value of 0, which may be confusing,
             * so we just print `-` in place. */
            if ((si.type == SingleInput::IT_FRAMERATE_NUM) || (si.type == SingleInput::IT_FRAMERATE_DEN)) {
                if (!ai.misc)
                    return QVariant();
                if ((ai.misc->framerate_num == movie->header->framerate_num) && 
                    (ai.misc->framerate_den == movie->header->framerate_den))
                    return QVariant();
            }
            if ((si.type == SingleInput::IT_REALTIME_SEC) && (value == 0))
                return QVariant();
            if ((si.type == SingleInput::IT_REALTIME_NSEC) && (value == 0))
                return QVariant();
            return QString().setNum(value);
        }

        if (value) {
            return QString(si.description.c_str());
        }
        else {
            return QVariant();
        }
    }

    if (role == Qt::EditRole) {
        if (row >= frameCount()) {
            return QVariant();
        }
        if (col < COLUMN_SPECIAL_SIZE) {
            return QVariant();
        }

        const SingleInput si = movie->editor->input_set[col-COLUMN_SPECIAL_SIZE];

        /* Don't edit locked input */
        if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
            return QVariant();

        const AllInputs& ai = movie->inputs->getInputs(row);

        /* Get the value of the single input in movie inputs */
        int value = ai.getInput(si);
        return QVariant(value);
    }

    return QVariant();
}

bool InputEditorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        unsigned int row = index.row();

        if (index.column() < COLUMN_SPECIAL_SIZE)
            return false;

        const SingleInput si = movie->editor->input_set[index.column()-COLUMN_SPECIAL_SIZE];

        /* Don't edit locked input */
        if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
            return false;

        /* Add rows if necessary */
        if (row >= frameCount()) {
            insertRows(movie->inputs->nbFrames(), row - frameCount() + 1, QModelIndex());
        }

        /* Rewind to past frame is needed */
        if (row < context->framecount) {
            bool ret = rewind(row);
            if (!ret)
                return false;
        }

        const AllInputs& ai = movie->inputs->getInputs(row);
        
        /* Don't modify inputs when frame has events */
        if (!ai.events.empty())
            return false;
        
        /* Check if the data is different */
        if (value.toInt() == ai.getInput(si))
            return false;

        /* Update the seek frame if we changed an earlier frame */
        if ((context->seek_frame) && !context->config.editor_rewind_seek
                                        && (row < context->seek_frame)) {
            context->seek_frame = row;
        }

        /* Modifying the movie is only performed by the main thread */
        movie->inputs->paintInput(si, value.toInt(), row, row);
        return true;
    }
    return false;
}

Qt::DropActions InputEditorModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

static const char mimeType[] = "application/x-libtas-analoginput";

QStringList InputEditorModel::mimeTypes() const
{
    return {QString::fromLatin1(mimeType)};
}

bool InputEditorModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    // check if the format is supported
    if (!mimeData->hasFormat(mimeType))
        return false;

    const QByteArray encodedData = mimeData->data(mimeType);
    QDataStream stream(encodedData);
    if (stream.atEnd())
        return false;

    SingleInput si;

    QString description;
    stream >> si.type >> si.which >> description;
    si.description = description.toStdString();
    addUniqueInput(si);

    return false;
}

void InputEditorModel::buildInputSet()
{
    std::set<SingleInput> new_input_set;

    /* Gather all unique inputs from the movie */
    movie->inputs->extractInputs(new_input_set);

    /* Remove inputs already on the list */
    for (SingleInput si : movie->editor->input_set) {
        new_input_set.erase(si);
    }

    /* Add the new inputs if any */
    for (SingleInput si : new_input_set) {

        /* Gather input description */
        for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
            for (SingleInput ti : context->config.km->input_list[i]) {
                if (si == ti) {
                    si.description = ti.description;
                    break;
                }
            }
        }

        /* Insert input */
        movie->editor->input_set.push_back(si);
    }
}

void InputEditorModel::startPaint(int col, int minRow, int maxRow, int value, int autofire)
{
    if (col < COLUMN_SPECIAL_SIZE)
        return;
        
    paintInput = movie->editor->input_set[col-COLUMN_SPECIAL_SIZE];

    /* Don't edit locked input */
    if (movie->editor->locked_inputs.find(paintInput) != movie->editor->locked_inputs.end())
        return;

    paintOngoing = true;
    paintMinRow = minRow;
    paintMaxRow = maxRow;
    paintValue = value;
    if (autofire >= 0)
        paintAutofire = autofire;
    emit dataChanged(index(minRow, col), index(maxRow, col));
}

void InputEditorModel::endPaint()
{
    if (!paintOngoing) return;

    /* Rewind to past frame if needed, otherwise paint whatever is possible */
    if (paintMinRow < context->framecount) {
        bool ret = rewind(paintMinRow);
        if (!ret) {
            /* Try rewinding to the earliest frame possible and paint what is possible */
            uint64_t root_frame = SaveStateList::rootStateFramecount();
            if (root_frame > paintMaxRow) {
                paintOngoing = false;
                return;
            }
            if (root_frame > paintMinRow)
                paintMinRow = root_frame;
            ret = rewind(root_frame);
            if (!ret) {
                paintOngoing = false;
                return;
            }
        }
    }

    /* Update the seek frame if we changed an earlier frame */
    if ((context->seek_frame) && !context->config.editor_rewind_seek
                                    && (paintMinRow < context->seek_frame)) {
        context->seek_frame = paintMinRow;
    }

    /* Add rows if necessary */
    if (paintMaxRow >= frameCount()) {
        insertRows(movie->inputs->nbFrames(), paintMaxRow - frameCount() + 1, QModelIndex());
    }

    if (paintAutofire == 0) {
        movie->inputs->paintInput(paintInput, paintValue, paintMinRow, paintMaxRow);
    }
    else {
        for (unsigned int r = paintMinRow; r <= paintMaxRow; r++) {
            movie->inputs->paintInput(paintInput, ((r%2)==(paintAutofire%2))?!paintValue:paintValue, r, r);
        }
    }
    paintOngoing = false;
}

std::string InputEditorModel::inputLabel(int column)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return "";

    return movie->editor->input_set[column-COLUMN_SPECIAL_SIZE].description;
}

void InputEditorModel::renameLabel(int column, std::string label)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return;

    /* Don't change label if it has only whitespaces */
    if (label.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
        return;
    
    movie->editor->input_set[column-COLUMN_SPECIAL_SIZE].description = label;
    emit dataChanged(index(0, column), index(rowCount(), column));
    emit inputSetChanged();
}

std::string InputEditorModel::inputDescription(int column)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return "";

    SingleInput si = movie->editor->input_set[column-COLUMN_SPECIAL_SIZE];

    /* Gather input description */
    for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
        for (SingleInput ti : context->config.km->input_list[i]) {
            if (si == ti) {
                return ti.description;
            }
        }
    }

    return "";
}

bool InputEditorModel::isInputAnalog(int column)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return false;

    const SingleInput si = movie->editor->input_set[column-COLUMN_SPECIAL_SIZE];
    return si.isAnalog();
}

bool InputEditorModel::insertRows(int row, int count, const QModelIndex &parent)
{
    return insertRows(row, count, false, parent);
}

bool InputEditorModel::insertRows(int row, int count, bool duplicate, const QModelIndex &parent)
{
    /* Rewind to past frame if needed */
    if (row < static_cast<int>(context->framecount)) {
        bool ret = rewind(row);
        if (!ret) {
            return 0;
        }
    }

    if (duplicate) {
        std::vector<AllInputs> duplicate_ais;
        for (int i=0; i<count; i++) {
            duplicate_ais.push_back(movie->inputs->getInputs(row + i));
        }

        movie->inputs->insertInputsBefore(duplicate_ais, row);
    }
    else {
        movie->inputs->insertInputsBefore(row, count);
    }

    return true;
}

bool InputEditorModel::removeRows(int row, int count, const QModelIndex &parent)
{
    /* Rewind to past frame if needed */
    if (row < static_cast<int>(context->framecount)) {
        bool ret = rewind(row);
        if (!ret) {
            return 0;
        }
    }

    movie->inputs->deleteInputs(row, count);

    return true;
}

bool InputEditorModel::hasMarker(int frame)
{
    return movie->editor->markers.count(frame) != 0;
}

std::string InputEditorModel::getMarkerText(int frame)
{
    if (hasMarker(frame))
        return movie->editor->markers[frame];

    return "";
}

void InputEditorModel::copyInputs(int row, int count, std::ostringstream& inputString)
{
    /* Translate inputs into a string */
    for (int r=row; r < row+count; r++) {
        const AllInputs& ai = movie->inputs->getInputs(r);
        InputSerialization::writeFrame(inputString, ai);
    }
}

int InputEditorModel::pasteInputs(int row)
{
    /* Rewind to past frame if needed */
    if (row < static_cast<int>(context->framecount)) {
        bool ret = rewind(row);
        if (!ret) {
            return 0;
        }
    }
    
    QClipboard *clipboard = QGuiApplication::clipboard();
    std::istringstream inputString(clipboard->text().toStdString());

    std::vector<AllInputs> paste_ais;
    InputSerialization::readInputs(inputString, paste_ais);

    /* Check if we need to insert frames */
    int insertedFrames = row + paste_ais.size() - movie->inputs->nbFrames();

    if (insertedFrames > 0) {
        movie->inputs->insertInputsBefore(movie->inputs->nbFrames(), insertedFrames);
    }

    movie->inputs->editInputs(paste_ais, row);

    return paste_ais.size();
}

void InputEditorModel::pasteInputsInRange(int row, int count)
{
    /* Rewind to past frame if needed */
    if (row < static_cast<int>(context->framecount)) {
        bool ret = rewind(row);
        if (!ret) {
            return;
        }
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    std::istringstream inputString(clipboard->text().toStdString());

    std::vector<AllInputs> paste_ais;
    InputSerialization::readInputs(inputString, paste_ais);

    /* Update the movie by ranges of pasted inputs */
    for (int i = 0; i < count; i+=paste_ais.size()) {
        size_t range_size = std::min(paste_ais.size(), (size_t)(count-i));
        movie->inputs->editInputs(paste_ais, row+i, range_size);
    }
}

int InputEditorModel::pasteInsertInputs(int row)
{
    /* Rewind to past frame if needed */
    if (row < static_cast<int>(context->framecount)) {
        bool ret = rewind(row);
        if (!ret) {
            return 0;
        }
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    std::istringstream inputString(clipboard->text().toStdString());

    std::vector<AllInputs> paste_ais;
    InputSerialization::readInputs(inputString, paste_ais);

    movie->inputs->insertInputsBefore(paste_ais, row);

    return paste_ais.size();
}


void InputEditorModel::addUniqueInput(const SingleInput &si)
{
    /* Check if input is already present */
    for (SingleInput ti : movie->editor->input_set) {
        if (si == ti) {
            return;
        }
    }

    beginInsertColumns(QModelIndex(), columnCount(), columnCount());
    movie->editor->input_set.push_back(si);
    endInsertColumns();
    emit inputSetChanged();
}

void InputEditorModel::addUniqueInputs(const AllInputs &ai)
{
    std::set<SingleInput> new_input_set;
    ai.extractInputs(new_input_set);

    /* Check if added inputs are already in the list */
    bool any_new_input = false;
    for (SingleInput si : new_input_set) {

        bool new_input = true;
        for (SingleInput ti : movie->editor->input_set) {
            if (si == ti) {
                new_input = false;
                break;
            }
        }

        /* Insert input if new */
        if (new_input) {
            any_new_input = true;

            /* Gather input description */
            for (int i=0; i<KeyMapping::INPUTLIST_SIZE; i++) {
                for (SingleInput ti : context->config.km->input_list[i]) {
                    if (si == ti) {
                        si.description = ti.description;
                        break;
                    }
                }
            }

            beginInsertColumns(QModelIndex(), columnCount(), columnCount());
            movie->editor->input_set.push_back(si);
            endInsertColumns();
        }
    }
    
    if (any_new_input)
        emit inputSetChanged();
}

void InputEditorModel::addUniqueInputs(int minRow, int maxRow)
{
    AllInputs newais;
    newais.clear();
    for (int row = minRow; row <= maxRow; row++) {
        const AllInputs& ai = movie->inputs->getInputsUnprotected(row);
        newais |= ai;
    }
    addUniqueInputs(newais);
}

void InputEditorModel::addUniqueInputs(const std::vector<AllInputs>& new_inputs)
{
    AllInputs newais;
    newais.clear();
    for (const AllInputs& ai : new_inputs) {
        newais |= ai;
    }
    addUniqueInputs(newais);
}

void InputEditorModel::clearUniqueInput(int column)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return;

    SingleInput si = movie->editor->input_set[column-COLUMN_SPECIAL_SIZE];

    /* Don't clear locked input */
    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        return;

    movie->inputs->paintInput(si, 0, context->framecount, movie->inputs->nbFrames()-1);
}

bool InputEditorModel::removeUniqueInput(int column)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return false;

    SingleInput si = movie->editor->input_set[column-COLUMN_SPECIAL_SIZE];

    /* Check if the input is set in past frames */
    for (unsigned int f = 0; f < context->framecount; f++) {
        const AllInputs& ai = movie->inputs->getInputs(f);
        if (ai.getInput(si))
            return false;
    }

    /* Clear remaining frames */
    movie->inputs->paintInput(si, 0, context->framecount, movie->inputs->nbFrames()-1);

    /* Remove clear locked state */
    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        movie->editor->locked_inputs.erase(si);

    /* Remove the column */
    beginRemoveColumns(QModelIndex(), column, column);
    movie->editor->input_set.erase(movie->editor->input_set.begin() + (column-COLUMN_SPECIAL_SIZE));
    endRemoveColumns();
    
    return true;
}

void InputEditorModel::columnFactor(int column, double factor)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return;

    SingleInput si = movie->editor->input_set[column-COLUMN_SPECIAL_SIZE];

    std::vector<int> new_values;

    for (unsigned int f = context->framecount; f < movie->inputs->nbFrames(); f++) {
        const AllInputs& ai = movie->inputs->getInputs(f);
        int value = ai.getInput(si);
        new_values.push_back(factor*value);
    }
    
    movie->inputs->paintInput(si, new_values, context->framecount);
}

bool InputEditorModel::isLockedUniqueInput(int column)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return false;

    SingleInput si = movie->editor->input_set[column-COLUMN_SPECIAL_SIZE];

    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        return true;

    return false;
}


void InputEditorModel::lockUniqueInput(int column, bool locked)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return;

    SingleInput si = movie->editor->input_set[column-COLUMN_SPECIAL_SIZE];

    if (locked) {
        movie->editor->locked_inputs.insert(si);
    }
    else {
        movie->editor->locked_inputs.erase(si);
    }

    /* Update the input column */
    emit dataChanged(index(0,column), index(rowCount()-1,column));
}


void InputEditorModel::clearInputs(int min_row, int max_row)
{
    /* Rewind to past frame if needed */
    if (min_row < static_cast<int>(context->framecount)) {
        bool ret = rewind(min_row);
        if (!ret) {
            return;
        }
    }

    movie->inputs->clearInputs(min_row, max_row);
}

void InputEditorModel::beginResetInputs()
{
    beginResetModel();
}

void InputEditorModel::endResetInputs()
{
    buildInputSet();
    last_savestate = 0;
    endResetModel();
    emit inputSetChanged();
}

void InputEditorModel::beginInsertInputs(int minRow, int maxRow)
{
    beginInsertRows(QModelIndex(), minRow, maxRow);
}

void InputEditorModel::endInsertInputs(int minRow, int maxRow)
{
    endInsertRows();

    /* We have to check if new inputs were added */
    addUniqueInputs(minRow, maxRow);
    
    /* Detect undo/redo operation */
    // if (!movie->changelog->is_recording) {
        undoMinRow = minRow;
        undoMaxRow = maxRow;
        undoMinCol = 0;
        undoMaxCol = columnCount()-1;
        undoTimeoutSec = maxUndoTimeoutSec;
        undoStart = std::chrono::steady_clock::now();
        undoTimer->start(50);
    // }
}

void InputEditorModel::beginEditInputs(int minRow, int maxRow)
{
}

void InputEditorModel::endEditInputs(int minRow, int maxRow)
{
    emit dataChanged(index(minRow,0), index(maxRow,columnCount()-1));

    /* We have to check if new inputs were added */
    addUniqueInputs(minRow, maxRow);
    
    /* Detect undo/redo operation */
    // if (!movie->changelog->is_recording) {
        undoMinRow = minRow;
        undoMaxRow = maxRow;
        undoMinCol = 0;
        undoMaxCol = columnCount()-1;
        undoTimeoutSec = maxUndoTimeoutSec;
        undoStart = std::chrono::steady_clock::now();
        undoTimer->start(50);
    // }
}

void InputEditorModel::beginRemoveInputs(int minRow, int maxRow)
{
    beginRemoveRows(QModelIndex(), minRow, maxRow);
}

void InputEditorModel::endRemoveInputs(int minRow, int maxRow)
{
    endRemoveRows();

    /* Detect undo/redo operation */
    // if (!movie->changelog->is_recording) {
        undoMinRow = std::min(minRow, rowCount()-1);
        undoMaxRow = std::min(maxRow, rowCount()-1);
        undoMinCol = 0;
        undoMaxCol = 1; // Show removed rows by only highlight first cols
        undoTimeoutSec = maxUndoTimeoutSec;
        undoStart = std::chrono::steady_clock::now();
        undoTimer->start(50);
    // }
}

void InputEditorModel::update()
{
    static uint64_t last_framecount = 0;
    if (context->framecount != last_framecount) {
        emit dataChanged(index(context->framecount,0), index(context->framecount,columnCount()-1));
        emit dataChanged(index(last_framecount,0), index(last_framecount,columnCount()-1));
        last_framecount = context->framecount;
    }
}

void InputEditorModel::highlightUndo()
{
    /* Highlight last undo/redo operation */
    if (undoTimeoutSec > 0.0f) {
        const auto undoEnd = std::chrono::steady_clock::now();
        const std::chrono::duration<float> undoDiff = undoEnd - undoStart;
        undoTimeoutSec = std::min(undoTimeoutSec, maxUndoTimeoutSec - undoDiff.count());
        
        emit dataChanged(index(undoMinRow,undoMinCol), index(undoMaxRow,undoMaxCol));
    }
    else
        undoTimer->stop();
}

/* Register a savestate. If saved, frame contains the framecount of the
 * savestate slot. If loaded, frame contains 0.
 */
void InputEditorModel::registerSavestate(int slot, unsigned long long frame)
{
    /* Refresh the first column for previous and current state framecount */
    emit dataChanged(index(last_savestate,0), index(last_savestate,0));
    emit dataChanged(index(frame,0), index(frame,0));

    /* Update last savestate frame */
    if (frame == 0) {
        const SaveState& s = SaveStateList::get(slot);
        last_savestate = s.id;
    }
    else
        last_savestate = frame;
    
    /* Update greenzone between old and new root savestate */
    uint64_t oldRoot = SaveStateList::oldRootStateFramecount();
    uint64_t newRoot = SaveStateList::rootStateFramecount();

    if (!oldRoot || !newRoot)
        return;
        
    if (oldRoot < newRoot)
        emit dataChanged(index(oldRoot,0), index(newRoot,columnCount()-1));
    else
        emit dataChanged(index(newRoot,0), index(oldRoot,columnCount()-1));
}

void InputEditorModel::moveInputs(int oldIndex, int newIndex)
{
    SingleInput si = movie->editor->input_set[oldIndex];
    movie->editor->input_set.erase(movie->editor->input_set.begin() + oldIndex);
    movie->editor->input_set.insert(movie->editor->input_set.begin() + newIndex, si);
}

bool InputEditorModel::rewind(uint64_t framecount)
{
    return rewind(framecount, false);
}

bool InputEditorModel::rewind(uint64_t framecount, bool enforce_seek)
{
    /* If another rewind is already performing, don't do anything but let
     * the possibility to change the input (the movie will check it it's
     * allowed later in the process). This is checked by looking if the hotkey
     * queue is empty or not (TODO: a bit of a hack, find better).
     */
    if (!context->hotkey_pressed_queue.empty())
        return true;
    
    /* If already on the frame, nothing to do */
    if (framecount == context->framecount)
        return true;

    int state = 0;
    if (framecount < context->framecount) {
        state = SaveStateList::nearestState(framecount, movie);
        if (state == -1)
            /* No available savestate before the given framecount */
            return false;
    }
        
    /* Switch to playback if needed */
    int recording = context->config.sc.recording;
    if (recording == SharedConfig::RECORDING_WRITE) {
        context->hotkey_pressed_queue.push(HOTKEY_READWRITE);
    }

    /* Save current framecount because it will be replaced by state loading */
    uint64_t current_framecount = context->framecount;

    /* Load state */
    if (framecount < current_framecount) {
        context->hotkey_pressed_queue.push(HOTKEY_LOADSTATE1 + (state-1));
    }

    /* Fast-forward to frame if further than state/current framecount */
    uint64_t state_framecount = (framecount < current_framecount)?(SaveStateList::get(state).framecount):current_framecount;
    
    if (framecount > state_framecount) {
        /* Seek to either the modified frame or the current frame */
        if (!enforce_seek && context->config.editor_rewind_seek)
            context->seek_frame = current_framecount;
        else
            context->seek_frame = framecount;

        /* Freeze scroll until seek_frame is reached */
        freeze_scroll = true;

        if (context->config.editor_rewind_fastforward)
            context->hotkey_pressed_queue.push(HOTKEY_FASTFORWARD);

        if (!context->config.sc.running)
            context->hotkey_pressed_queue.push(HOTKEY_PLAYPAUSE);
    }
    else {
        /* Just pause */
        if (context->config.sc.running)
            context->hotkey_pressed_queue.push(HOTKEY_PLAYPAUSE);
    }
    
    /* To display a progress bar showing that a state is loading, we must
     * signal the window, because otherwise the next time it will update is
     * after the state has been loaded */
    emit stateLoaded();

    return true;
}

bool InputEditorModel::isScrollFreeze()
{
    return freeze_scroll;
}

void InputEditorModel::setScrollFreeze(bool state)
{
    freeze_scroll = state;
}

void InputEditorModel::setHoveredCell(const QModelIndex &i)
{
    QVector<int> roles(2, Qt::DisplayRole);
    roles[1] = Qt::ForegroundRole;
    
    const QModelIndex old = hoveredIndex;
    hoveredIndex = i;
    emit dataChanged(index(0,old.column()), index(rowCount(),old.column()), roles);
    emit dataChanged(index(0,hoveredIndex.column()), index(rowCount(),hoveredIndex.column()), QVector<int>(1, Qt::BackgroundRole));
    emit headerDataChanged(Qt::Horizontal, old.column(), old.column());
    emit headerDataChanged(Qt::Horizontal, hoveredIndex.column(), hoveredIndex.column());
}

void InputEditorModel::seekToFrame(unsigned long long frame)
{
    rewind(frame, true); // rewind and enforce seek to frame
}

void InputEditorModel::setAutoholdInput(int column, bool checked)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return;

    movie->editor->setAutohold(column-COLUMN_SPECIAL_SIZE, checked);

    emit dataChanged(index(0,column), index(rowCount(),column), QVector<int>(1, Qt::BackgroundRole));
    emit headerDataChanged(Qt::Horizontal, column, column);
}

bool InputEditorModel::isAutoholdInput(int column) const
{
    if (column < COLUMN_SPECIAL_SIZE)
        return false;

    return movie->editor->isAutohold(column-COLUMN_SPECIAL_SIZE);
}

void InputEditorModel::setAutofireInput(int column, bool checked)
{
    if (column < COLUMN_SPECIAL_SIZE)
        return;

    movie->editor->setAutofire(column-COLUMN_SPECIAL_SIZE, checked);

    emit dataChanged(index(0,column), index(rowCount(),column), QVector<int>(1, Qt::BackgroundRole));
    emit headerDataChanged(Qt::Horizontal, column, column);
}

bool InputEditorModel::isAutofireInput(int column) const
{
    return movie->editor->isAutofire(column-COLUMN_SPECIAL_SIZE);
}

void InputEditorModel::shiftMarkers(int startRow, int offset) {
    if (!context->config.editor_move_marker)
        return;

    // Create a temporary map to store updated markers
    std::map<int, std::string> updatedMarkers;

    for (const auto& marker : movie->editor->markers) {
        if (marker.first >= startRow) {
            updatedMarkers[marker.first + offset] = marker.second;
        } else {
            updatedMarkers[marker.first] = marker.second;
        }
    }

    // Replace the old markers with the updated ones
    movie->editor->markers = std::move(updatedMarkers);
}

void InputEditorModel::removeMarkersInRange(int startRow, int endRow) {
    if (!context->config.editor_move_marker)
        return;

    // Remove markers within the specified range
    auto it = movie->editor->markers.begin();
    while (it != movie->editor->markers.end()) {
        if (it->first >= startRow && it->first <= endRow) {
            it = movie->editor->markers.erase(it);
        } else {
            ++it;
        }
    }

    // Shift markers after the deleted range
    int offset = endRow - startRow + 1;
    std::map<int, std::string> updatedMarkers;
    for (const auto& marker : movie->editor->markers) {
        if (marker.first > endRow) {
            updatedMarkers[marker.first - offset] = marker.second;
        } else {
            updatedMarkers[marker.first] = marker.second;
        }
    }

    movie->editor->markers = std::move(updatedMarkers);
}
