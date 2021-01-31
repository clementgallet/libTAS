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

#include <QBrush>
#include <QClipboard>
#include <QGuiApplication>
#include <QPalette>
#include <QFont>
#include <QThread>
#include <sstream>
#include <iostream>
#include <set>

#include "InputEditorModel.h"
#include "../SaveStateList.h"

InputEditorModel::InputEditorModel(Context* c, MovieFile* m, QObject *parent) : QAbstractTableModel(parent), context(c), movie(m)
{
    savestate_frames.fill(-1);
}

int InputEditorModel::rowCount(const QModelIndex & /*parent*/) const
{
    return movie->inputs->nbFrames() + 1;
}

int InputEditorModel::columnCount(const QModelIndex & /*parent*/) const
{
    return movie->editor->input_set.size() + 2;
}

Qt::ItemFlags InputEditorModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    if (index.column() < 2)
        return QAbstractItemModel::flags(index);

    /* Don't toggle past inputs before root savestate */
    uint64_t root_frame = SaveStateList::rootStateFramecount();
    if (!root_frame) {
        if (index.row() < static_cast<int>(context->framecount))
            return QAbstractItemModel::flags(index);
    }
    else {
        if (static_cast<uint64_t>(index.row()) < root_frame)
            return QAbstractItemModel::flags(index);
    }

    const SingleInput si = movie->editor->input_set[index.column()-2];

    /* Don't edit locked input */
    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        return QAbstractItemModel::flags(index);

    if (si.isAnalog())
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractItemModel::flags(index);
}

QVariant InputEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0)
                return QString(tr(""));
            if (section == 1)
                return QString(tr("Frame"));
            if (static_cast<unsigned int>(section-2) < movie->editor->input_set.size())
                return QString(movie->editor->input_set[section-2].description.c_str());
        }
    }
    return QVariant();
}

QVariant InputEditorModel::data(const QModelIndex &index, int role) const
{
    unsigned int row = index.row();
    
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role == Qt::FontRole) {
        QFont font;
        if ((index.column() == 0) && (row == last_savestate)) {
            font.setBold(true);
        }
        return font;
    }

    if (role == Qt::BackgroundRole) {
        /* Main color */
        QColor color = QGuiApplication::palette().window().color();
        int r, g, b;
        color.getRgb(&r, &g, &b, nullptr);
        
        if (color.lightness() > 128) {
            /* Light theme */
            if (row == context->framecount)
                color.setRgb(r - 0x30, g - 0x10, b);
            else if (row < context->framecount) {
                if (movie->editor->isDraw(row))
                    color.setRgb(r - 0x30, g, b - 0x30);
                else
                    color.setRgb(r, g - 0x30, b - 0x30);
            }
            else {
                if (movie->editor->isDraw(row))
                    color.setRgb(r, g, b - 0x18);
                else
                    color.setRgb(r, g - 0x18, b - 0x18);
            }
        }
        else {
            /* Dark theme */
            if (row == context->framecount)
                color.setRgb(r, g + 0x10, b + 0x20);
            else if (row < context->framecount) {
                if (movie->editor->isDraw(row))
                    color.setRgb(r, g + 0x18, b);
                else
                    color.setRgb(r + 0x18, g, b);
            }
            else {
                if (movie->editor->isDraw(row))
                    color.setRgb(r + 0x08, g + 0x08, b);
                else
                    color.setRgb(r + 0x08, g, b);
            }
        }

        /* Frame column */
        if (index.column() <= 1) {
            color = color.lighter(105);
        }
        else {
            /* Check for locked input */
            if (!movie->editor->locked_inputs.empty()) {
                const SingleInput si = movie->editor->input_set[index.column()-2];
                if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end()) {
                    color = color.darker(150);
                }
            }
        }
        
        /* Greenzone */
        if (row < context->framecount) {
            uint64_t root_frame = SaveStateList::rootStateFramecount();
            if (!root_frame && row >= root_frame)
                color = color.darker(105);            
        }

        /* Frame containing a savestate */
        for (unsigned int i=0; i<savestate_frames.size(); i++) {
            if (savestate_frames[i] == row) {
                color = color.darker(105);
                break;
            }
        }

        return QBrush(color);
    }

    if (role == Qt::DisplayRole) {
        if (row >= movie->inputs->nbFrames()) {
            return QVariant();
        }
        if (index.column() == 0) {
            for (unsigned int i=0; i<savestate_frames.size(); i++) {
                if (savestate_frames[i] == row) {
                    return i;
                }
            }
            return QString("");
        }
        if (index.column() == 1) {
            return row;
        }

        const AllInputs ai = movie->inputs->input_list[row];
        const SingleInput si = movie->editor->input_set[index.column()-2];

        /* Get the value of the single input in movie inputs */
        int value = ai.getInput(si);

        if (si.isAnalog()) {
            return QString().setNum(value);
        }

        if (value) {
            return QString(si.description.c_str());
        }
        else {
            return QString("");
        }
    }

    if (role == Qt::EditRole) {
        if (row >= movie->inputs->nbFrames()) {
            return QVariant();
        }
        if (index.column() < 2) {
            return QVariant();
        }

        const SingleInput si = movie->editor->input_set[index.column()-2];

        /* Don't edit locked input */
        if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
            return QVariant();

        const AllInputs ai = movie->inputs->input_list[row];

        /* Get the value of the single input in movie inputs */
        int value = ai.getInput(si);

        if (si.isAnalog()) {
            return QVariant(value);
        }
    }

    return QVariant();
}

bool InputEditorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        unsigned int row = index.row();

        if (index.column() < 2)
            return false;

        /* Rewind to past frame is needed */
        if (row < context->framecount) {
            bool ret = rewind(row);
            if (!ret)
                return false;
        }

        const SingleInput si = movie->editor->input_set[index.column()-2];

        /* Don't edit locked input */
        if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
            return false;

        /* Add a row if necessary */
        if (row == movie->inputs->nbFrames()) {
            insertRows(movie->inputs->nbFrames(), 1, QModelIndex());
        }

        AllInputs &ai = movie->inputs->input_list[row];

        int ivalue = value.toInt();

        ai.setInput(si, ivalue);
        movie->inputs->wasModified();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}


void InputEditorModel::buildInputSet()
{
    std::set<SingleInput> new_input_set;

    /* Gather all unique inputs from the movie */
    for (const AllInputs &ai : movie->inputs->input_list) {
        ai.extractInputs(new_input_set);
    }

    /* Remove inputs already on the list */
    for (SingleInput si : movie->editor->input_set) {
        new_input_set.erase(si);
    }

    /* Add the new inputs if any */
    for (SingleInput si : new_input_set) {

        /* Gather input description */
        for (SingleInput ti : context->config.km.input_list) {
            if (si == ti) {
                si.description = ti.description;
                break;
            }
        }

        /* Insert input */
        movie->editor->input_set.push_back(si);
    }
}

bool InputEditorModel::toggleInput(const QModelIndex &index)
{
    /* Don't toggle savestate / frame count */
    if (index.column() < 2)
        return false;

    unsigned int row = index.row();

    /* Don't toggle past inputs before root savestate */
    uint64_t root_frame = SaveStateList::rootStateFramecount();
    if (!root_frame) {
        if (row < context->framecount)
            return false;
    }
    else {
        if (row < root_frame)
            return false;        
    }

    SingleInput si = movie->editor->input_set[index.column()-2];

    /* Don't toggle locked input */
    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        return false;

    /* Add a row if necessary */
    if (row == movie->inputs->nbFrames()) {
        insertRows(movie->inputs->nbFrames(), 1, QModelIndex());
    }
    
    /* Rewind to past frame is needed */
    if (row < context->framecount) {
        bool ret = rewind(row);
        if (!ret)
            return false;
    }

    AllInputs &ai = movie->inputs->input_list[row];

    int value = ai.toggleInput(si);
    movie->inputs->wasModified();

    emit dataChanged(index, index);

    movie->inputs->modifiedSinceLastSave = true;
    movie->inputs->modifiedSinceLastAutoSave = true;
    return value;
}

std::string InputEditorModel::inputLabel(int column)
{
    return movie->editor->input_set[column-2].description;
}

void InputEditorModel::renameLabel(int column, std::string label)
{
    movie->editor->input_set[column-2].description = label;
    emit dataChanged(createIndex(0, column), createIndex(rowCount(), column));
    emit inputSetChanged();
}

std::string InputEditorModel::inputDescription(int column)
{
    SingleInput si = movie->editor->input_set[column-2];

    /* Gather input description */
    for (SingleInput ti : context->config.km.input_list) {
        if (si == ti) {
            return ti.description;
        }
    }

    return "";
}

bool InputEditorModel::isInputAnalog(int column)
{
    if (column < 2)
        return false;

    const SingleInput si = movie->editor->input_set[column-2];
    return si.isAnalog();
}

bool InputEditorModel::insertRows(int row, int count, const QModelIndex &parent)
{
    /* Don't insert past inputs */
    if (row < static_cast<int>(context->framecount))
        return false;

    beginInsertRows(parent, row, row+count-1);

    AllInputs ai;
    ai.emptyInputs();

    for (int i=0; i<count; i++) {
        movie->inputs->insertInputsBefore(ai, row);
    }

    endInsertRows();

    /* Update the movie framecount. Should it be done here ?? */
    movie->inputs->updateLength();

    return true;
}

bool InputEditorModel::removeRows(int row, int count, const QModelIndex &parent)
{
    /* Don't delete past inputs */
    if (row < static_cast<int>(context->framecount))
        return false;

    beginRemoveRows(parent, row, row+count-1);

    for (int i=0; i<count; i++) {
        movie->inputs->deleteInputs(row);
    }

    endRemoveRows();

    /* Update the movie framecount */
    movie->inputs->updateLength();

    return true;
}

void InputEditorModel::clearClipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->clear();
}

void InputEditorModel::copyInputs(int row, int count)
{
    std::ostringstream inputString;

    /* Translate inputs into a string */
    for (int r=row; r < row+count; r++) {
        movie->inputs->writeFrame(inputString, movie->inputs->input_list[r]);
    }

    /* Append text from the existing clipboard text */
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString clipText = clipboard->text();
    clipText.append(inputString.str().c_str());
    clipboard->setText(clipText);
}

int InputEditorModel::pasteInputs(int row)
{
    /* Don't modify past inputs */
    if (row < static_cast<int>(context->framecount))
        return 0;

    QClipboard *clipboard = QGuiApplication::clipboard();
    std::istringstream inputString(clipboard->text().toStdString());

    std::vector<AllInputs> paste_ais;
    std::string line;
    while (std::getline(inputString, line)) {
        if (!line.empty() && (line[0] == '|')) {
            AllInputs ai;
            movie->inputs->readFrame(line, ai);
            paste_ais.push_back(ai);
        }
    }

    /* Check if we need to insert frames */
    int insertedFrames = row + paste_ais.size() - movie->inputs->nbFrames();

    if (insertedFrames > 0) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + insertedFrames - 1);
    }

    for (size_t r = 0; r < paste_ais.size(); r++) {
        movie->inputs->setInputs(paste_ais[r], row + r, true);
        addUniqueInputs(paste_ais[r]);
    }

    if (insertedFrames > 0) {
        endInsertRows();
    }

    /* Update the movie framecount */
    movie->inputs->updateLength();

    /* Update the paste inputs view */
    emit dataChanged(createIndex(row,0), createIndex(row+paste_ais.size()-1,columnCount()));

    emit inputSetChanged();

    return paste_ais.size();
}

int InputEditorModel::pasteInsertInputs(int row)
{
    /* Don't modify past inputs */
    if (row < static_cast<int>(context->framecount))
        return 0;

    QClipboard *clipboard = QGuiApplication::clipboard();
    std::istringstream inputString(clipboard->text().toStdString());

    std::vector<AllInputs> paste_ais;
    std::string line;
    while (std::getline(inputString, line)) {
        if (!line.empty() && (line[0] == '|')) {
            AllInputs ai;
            movie->inputs->readFrame(line, ai);
            paste_ais.push_back(ai);
        }
    }

    beginInsertRows(QModelIndex(), row, row + paste_ais.size() - 1);

    for (size_t r = 0; r < paste_ais.size(); r++) {
        movie->inputs->insertInputsBefore(paste_ais[r], row + r);
        addUniqueInputs(paste_ais[r]);
    }

    endInsertRows();

    /* Update the movie framecount */
    movie->inputs->updateLength();

    emit inputSetChanged();

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

            /* Gather input description */
            for (SingleInput ti : context->config.km.input_list) {
                if (si == ti) {
                    si.description = ti.description;
                    break;
                }
            }

            beginInsertColumns(QModelIndex(), columnCount(), columnCount());
            movie->editor->input_set.push_back(si);
            endInsertColumns();
        }
    }
    emit inputSetChanged();
}

void InputEditorModel::clearUniqueInput(int column)
{
    SingleInput si = movie->editor->input_set[column-2];

    /* Don't clear locked input */
    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        return;

    for (unsigned int f = context->framecount; f < movie->inputs->nbFrames(); f++) {
        movie->inputs->input_list[f].setInput(si, 0);
    }

    movie->inputs->wasModified();
}

void InputEditorModel::removeUniqueInput(int column)
{
    SingleInput si = movie->editor->input_set[column-2];

    /* Check if the input is set in past frames */
    for (unsigned int f = 0; f < context->framecount; f++) {
        if (movie->inputs->input_list[f].getInput(si))
            return;
    }

    /* Clear remaining frames */
    for (unsigned int f = context->framecount; f < movie->inputs->nbFrames(); f++) {
        movie->inputs->input_list[f].setInput(si, 0);
    }

    movie->inputs->wasModified();

    /* Remove clear locked state */
    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        movie->editor->locked_inputs.erase(si);

    /* Remove the column */
    beginRemoveColumns(QModelIndex(), column, column);
    movie->editor->input_set.erase(movie->editor->input_set.begin() + (column-2));
    endRemoveColumns();
}

bool InputEditorModel::isLockedUniqueInput(int column)
{
    if (column < 2)
        return false;

    SingleInput si = movie->editor->input_set[column-2];

    if (movie->editor->locked_inputs.find(si) != movie->editor->locked_inputs.end())
        return true;

    return false;
}


void InputEditorModel::lockUniqueInput(int column, bool locked)
{
    if (column < 2)
        return;

    SingleInput si = movie->editor->input_set[column-2];

    if (locked) {
        movie->editor->locked_inputs.insert(si);
    }
    else {
        movie->editor->locked_inputs.erase(si);
    }

    /* Update the input column */
    emit dataChanged(createIndex(0,column), createIndex(rowCount()-1,column));
}


void InputEditorModel::clearInput(int row)
{
    movie->inputs->input_list[row].emptyInputs();
    emit dataChanged(createIndex(row, 0), createIndex(row, columnCount()));

    movie->inputs->wasModified();
}

void InputEditorModel::beginModifyInputs()
{
    beginResetModel();
}

void InputEditorModel::endModifyInputs()
{
    buildInputSet();
    endResetModel();
    emit inputSetChanged();
}

void InputEditorModel::beginAddInputs()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
}

void InputEditorModel::endAddInputs()
{
    endInsertRows();

    /* We have to check if new inputs were added */
    addUniqueInputs(movie->inputs->input_list[movie->inputs->nbFrames()-1]);
}

void InputEditorModel::beginEditInputs()
{
}

void InputEditorModel::endEditInputs()
{
    emit dataChanged(createIndex(context->framecount,0), createIndex(context->framecount,columnCount()));

    /* We have to check if new inputs were added */
    addUniqueInputs(movie->inputs->input_list[context->framecount]);
}

void InputEditorModel::update()
{
    if (context->framecount == 1) {
        beginResetModel();
        buildInputSet();
        endResetModel();
        emit inputSetChanged();
    }
    else {
        emit dataChanged(createIndex(context->framecount,0), createIndex(context->framecount,columnCount()));
        // emit dataChanged(createIndex(0,0), createIndex(rowCount(),columnCount()));
    }
}

void InputEditorModel::resetInputs()
{
    beginResetModel();
    // input_set.clear();
    savestate_frames.fill(-1);
    endResetModel();
    emit inputSetChanged();
}

/* Register a savestate. If saved, frame contains the framecount of the
 * savestate slot. It loaded, frame contains 0.
 */
void InputEditorModel::registerSavestate(int slot, unsigned long long frame)
{
    if (frame > 0)
        savestate_frames[slot] = frame;
    unsigned long long old_savestate = last_savestate;
    last_savestate = savestate_frames[slot];
    emit dataChanged(createIndex(old_savestate,0), createIndex(old_savestate,0));
    emit dataChanged(createIndex(last_savestate,0), createIndex(last_savestate,0));
    
    /* Update greenzone between old and new root savestate */
    uint64_t oldRoot = SaveStateList::oldRootStateFramecount();
    uint64_t newRoot = SaveStateList::rootStateFramecount();

    if (!oldRoot || !newRoot)
        return;
        
    if (oldRoot < newRoot)
        emit dataChanged(createIndex(oldRoot,0), createIndex(newRoot,columnCount()));
    else
        emit dataChanged(createIndex(newRoot,0), createIndex(oldRoot,columnCount()));
}

void InputEditorModel::moveInputs(int oldIndex, int newIndex)
{
    SingleInput si = movie->editor->input_set[oldIndex];
    movie->editor->input_set.erase(movie->editor->input_set.begin() + oldIndex);
    movie->editor->input_set.insert(movie->editor->input_set.begin() + newIndex, si);
}

bool InputEditorModel::rewind(uint64_t framecount)
{
    if (framecount >= context->framecount)
        return false;
        
    int state = SaveStateList::nearestState(framecount);
    if (state == -1)
        return false;
        
    /* Switch to playback if needed */
    int recording = context->config.sc.recording;
    if (recording == SharedConfig::RECORDING_WRITE) {
        context->hotkey_pressed_queue.push(HOTKEY_READWRITE);
    }

    /* Load state */
    context->hotkey_pressed_queue.push(HOTKEY_LOADSTATE1 + (state-1));

    /* Fast-forward to frame if further than state framecount */
    uint64_t state_framecount = SaveStateList::get(state).framecount;
    if (framecount > state_framecount) {
        context->pause_frame = framecount;

        context->hotkey_pressed_queue.push(HOTKEY_FASTFORWARD);
        if (!context->config.sc.running)
            context->hotkey_pressed_queue.push(HOTKEY_PLAYPAUSE);
    }
    
    /* Switch back */
    if (recording == SharedConfig::RECORDING_WRITE) {
        context->hotkey_pressed_queue.push(HOTKEY_READWRITE);
    }
    
    return true;
}
