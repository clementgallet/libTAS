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

#include <QtGui/QBrush>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#include <QtGui/QFont>
#include <sstream>
#include <iostream>
#include <set>

#include "InputEditorModel.h"
#include "../SaveStateList.h"

InputEditorModel::InputEditorModel(Context* c, MovieFile* m, QObject *parent) : QAbstractTableModel(parent), context(c), movie(m) {}

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

    if (role == Qt::ForegroundRole) {
        if (row >= movie->inputs->nbFrames()) {
            return QGuiApplication::palette().text();
        }
        if (index.column() <= 1) {
            return QGuiApplication::palette().text();
        }

        QColor color = QGuiApplication::palette().text().color();

        /* Show inputs with transparancy when they are pending due to rewind */
        movie->inputs->input_event_queue.lock();
        for (auto it = movie->inputs->input_event_queue.begin(); it != movie->inputs->input_event_queue.end(); it++) {
            if (it->framecount != row)
                continue;
            const SingleInput si = movie->editor->input_set[index.column()-2];
            if (si == it->si) {
                /* For analog, use half-transparancy. Otherwise,
                 * use strong/weak transparancy of set/clear input */
                if (si.isAnalog()) {
                    color.setAlpha(128);
                }
                else {
                    if (it->value) {
                        color.setAlpha(192);
                    }
                    else {
                        color.setAlpha(64);
                    }
                }
                /* We don't return the brush immediatly, because users may change
                 * multiple times the same input. */
            }
        }
        movie->inputs->input_event_queue.unlock();
        
        return QBrush(color);
    }

    if (role == Qt::BackgroundRole) {
        /* Main color */
        QColor color = QGuiApplication::palette().window().color();
        int r, g, b;
        color.getRgb(&r, &g, &b, nullptr);
        
        /* Greenzone */
        uint64_t root_frame = SaveStateList::rootStateFramecount();
        bool greenzone = (row < context->framecount) && root_frame && (row >= root_frame);

        if (color.lightness() > 128) {
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
        
        /* Frame containing a savestate */
        int savestate_frame = SaveStateList::stateAtFrame(row);
        if (savestate_frame != -1)
            color = color.darker(105);

        /* Invalid portion */
        if (row < invalid_frame)
            color = color.darker(120);

        return QBrush(color);
    }

    if (role == Qt::DisplayRole) {
        if (row >= movie->inputs->nbFrames()) {
            return QVariant();
        }
        if (index.column() == 0) {
            int savestate_frame = SaveStateList::stateAtFrame(row);
            if (savestate_frame != -1) {
                if (savestate_frame == 10)
                    return QString("B");
                else
                    return savestate_frame;
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
        
        /* If the value is currently being modified, load the new value */
        movie->inputs->input_event_queue.lock();
        for (auto it = movie->inputs->input_event_queue.begin(); it != movie->inputs->input_event_queue.end(); it++) {
            if (it->framecount != row)
                continue;
            if (si == it->si) {
                if (si.isAnalog()) {
                    value = it->value;
                }
                else {
                    /* For non-analog values, always print the value, and the
                     * transparancy value will indicate if the value is being
                     * cleared or set. */
                    value = 1;
                }
            }
        }
        movie->inputs->input_event_queue.unlock();

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
            bool ret = rewind(row, true);
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

        /* Check if the data is different */
        AllInputs ai;
        movie->inputs->getInputs(ai, row);
        if (value.toInt() == ai.getInput(si))
            return false;

        /* Modifying the movie is only performed by the main thread */
        InputEvent ie;
        ie.framecount = row;
        ie.si = si;
        ie.value = value.toInt();
        movie->inputs->input_event_queue.push(ie);
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
        bool ret = rewind(row, true);
        if (!ret)
            return false;
    }

    /* Modifying the movie is only performed by the main thread */
    AllInputs ai;
    movie->inputs->getInputs(ai, row);
    InputEvent ie;
    ie.framecount = row;
    ie.si = si;
    ie.value = !ai.getInput(si);
    movie->inputs->input_event_queue.push(ie);
    emit dataChanged(index, index);
    return ie.value;
}

std::string InputEditorModel::inputLabel(int column)
{
    return movie->editor->input_set[column-2].description;
}

void InputEditorModel::renameLabel(int column, std::string label)
{
    /* Don't change label if it has only whitespaces */
    if (label.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
        return;
    
    movie->editor->input_set[column-2].description = label;
    emit dataChanged(index(0, column), index(rowCount(), column));
    emit inputSetChanged();
}

std::string InputEditorModel::inputDescription(int column)
{
    SingleInput si = movie->editor->input_set[column-2];

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
    emit dataChanged(index(row,0), index(row+paste_ais.size()-1,columnCount()));

    emit inputSetChanged();

    return paste_ais.size();
}

void InputEditorModel::pasteInputsInRange(int row, int count)
{
    /* Don't modify past inputs */
    if (row < static_cast<int>(context->framecount))
        return;

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

    size_t r = 0;
    for (int f = row; f < (row+count); f++) {
        movie->inputs->setInputs(paste_ais[r], f, true);
        addUniqueInputs(paste_ais[r]);
        r = (r+1)%paste_ais.size();
    }

    /* Update the movie framecount */
    movie->inputs->updateLength();

    /* Update the paste inputs view */
    emit dataChanged(index(row,0), index(row+count-1,columnCount()));

    emit inputSetChanged();
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
    emit dataChanged(index(0,column), index(rowCount()-1,column));
}


void InputEditorModel::clearInput(int row)
{
    movie->inputs->input_list[row].emptyInputs();
    emit dataChanged(index(row, 0), index(row, columnCount()));

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

void InputEditorModel::beginEditInputs(unsigned long long framecount)
{
}

void InputEditorModel::endEditInputs(unsigned long long framecount)
{
    emit dataChanged(index(framecount,0), index(framecount,columnCount()-1));

    /* We have to check if new inputs were added */
    addUniqueInputs(movie->inputs->input_list[framecount]);
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
        emit dataChanged(index(context->framecount,0), index(context->framecount,columnCount()-1));
        // emit dataChanged(index(0,0), index(rowCount(),columnCount()));
    }
}

void InputEditorModel::resetInputs()
{
    beginResetModel();
    invalid_frame = 0;
    last_savestate = 0;
    endResetModel();
    emit inputSetChanged();
}

void InputEditorModel::invalidateSavestates()
{
    /* Update invalid frame */
    uint64_t previous_invalid_frame = invalid_frame;
    invalid_frame = context->framecount;
    
    /* Update portion of the table */
    emit dataChanged(index(previous_invalid_frame,0), index(context->framecount,columnCount()-1));
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

bool InputEditorModel::rewind(uint64_t framecount, bool toggle)
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
        state = SaveStateList::nearestState(framecount);
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
        if (toggle && context->config.editor_rewind_seek)
            context->pause_frame = current_framecount;
        else
            context->pause_frame = framecount;

        /* Freeze scroll until pause_frame is reached */
        freeze_scroll = true;

        context->hotkey_pressed_queue.push(HOTKEY_FASTFORWARD);
        if (!context->config.sc.running)
            context->hotkey_pressed_queue.push(HOTKEY_PLAYPAUSE);
    }
    else {
        /* Just pause */
        if (context->config.sc.running)
            context->hotkey_pressed_queue.push(HOTKEY_PLAYPAUSE);
    }
    
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
