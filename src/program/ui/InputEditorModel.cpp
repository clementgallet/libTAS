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

#include <QBrush>
#include <QClipboard>
#include <QGuiApplication>
#include <QFont>
#include <sstream>

#include <set>

#include "InputEditorModel.h"

InputEditorModel::InputEditorModel(Context* c, MovieFile* m, QObject *parent) : QAbstractTableModel(parent), context(c), movie(m)
{
    savestate_frames.fill(-1);
}

int InputEditorModel::rowCount(const QModelIndex & /*parent*/) const
{
    return movie->nbFrames() + 1;
}

int InputEditorModel::columnCount(const QModelIndex & /*parent*/) const
{
    return input_set.size() + 2;
}

Qt::ItemFlags InputEditorModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    if (index.column() < 2)
        return QAbstractItemModel::flags(index);

    if (index.row() < static_cast<int>(context->framecount))
        return QAbstractItemModel::flags(index);

    const SingleInput si = input_set[index.column()-2];

    /* Don't edit locked input */
    if (movie->locked_inputs.find(si) != movie->locked_inputs.end())
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
            return QString(input_set[section-2].description.c_str());
        }
    }
    return QVariant();
}

QVariant InputEditorModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role == Qt::FontRole) {
        QFont font;
        if ((index.column() == 0) && (index.row() == last_savestate)) {
            font.setBold(true);
        }
        return font;
    }

    if (role == Qt::BackgroundRole) {
        /* Return white-ish for future inputs */
        // if (index.row() > static_cast<int>(context->framecount))
        //     return QBrush(QColor(0xff, 0xfe, 0xee));

        /* Return white-ish for savestate column */
        // if (index.column() == 0)
        //     return QBrush(QColor(0xff, 0xfe, 0xee));

        QColor color;

        /* Main color */
        if (index.row() == static_cast<int>(context->framecount))
            color.setRgb(0xb5, 0xe7, 0xf7);
        else if (index.row() < static_cast<int>(context->framecount))
            color.setRgb(0xd2, 0xf9, 0xd3);
        else
            color.setRgb(0xfe, 0xfe, 0xe8);

        /* Frame column */
        if (index.column() <= 1) {
            color = color.lighter(105);
        }
        else {
            /* Check for locked input */
            if (!movie->locked_inputs.empty()) {
                const SingleInput si = input_set[index.column()-2];
                if (movie->locked_inputs.find(si) != movie->locked_inputs.end()) {
                    color = color.darker(150);
                }
            }
        }

        /* Frame containing a savestate */
        for (unsigned int i=0; i<savestate_frames.size(); i++) {
            if (savestate_frames[i] == index.row()) {
                color = color.darker(105);
                break;
            }
        }

        return QBrush(color);
    }

    if (role == Qt::DisplayRole) {
        if (index.row() >= movie->input_list.size()) {
            return QVariant();
        }
        if (index.column() == 0) {
            for (unsigned int i=0; i<savestate_frames.size(); i++) {
                if (savestate_frames[i] == index.row()) {
                    return i;
                }
            }
            return QString("");
        }
        if (index.column() == 1) {
            return index.row();
        }

        const AllInputs ai = movie->input_list[index.row()];
        const SingleInput si = input_set[index.column()-2];

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
        if (index.row() >= movie->input_list.size()) {
            return QVariant();
        }
        if (index.column() < 2) {
            return QVariant();
        }

        const SingleInput si = input_set[index.column()-2];

        /* Don't edit locked input */
        if (movie->locked_inputs.find(si) != movie->locked_inputs.end())
            return QVariant();

        const AllInputs ai = movie->input_list[index.row()];

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

        if (index.column() < 2)
            return false;

        if (index.row() < static_cast<int>(context->framecount))
            return false;

        const SingleInput si = input_set[index.column()-2];

        /* Don't edit locked input */
        if (movie->locked_inputs.find(si) != movie->locked_inputs.end())
            return false;

        /* Add a row if necessary */
        if (index.row() == movie->input_list.size()) {
            insertRows(movie->input_list.size(), 1, QModelIndex());
        }

        AllInputs &ai = movie->input_list[index.row()];

        int ivalue = value.toInt();

        ai.setInput(si, ivalue);
        movie->wasModified();
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}


void InputEditorModel::buildInputSet()
{
    std::set<SingleInput> new_input_set;

    /* Gather all unique inputs from the movie */
    for (const AllInputs &ai : movie->input_list) {
        ai.extractInputs(new_input_set);
    }

    input_set.clear();
    for (SingleInput si : new_input_set) {

        /* Gather input name in the movie if there is one */
        auto it = movie->input_names.find(si);
        if (it != movie->input_names.end()) {
            si.description = it->second;
        }
        else {
            /* Gather input description */
            for (SingleInput ti : context->config.km.input_list) {
                if (si == ti) {
                    si.description = ti.description;
                    break;
                }
            }
        }


        /* Insert input */
        input_set.push_back(si);
    }

}

bool InputEditorModel::toggleInput(const QModelIndex &index)
{
    /* Don't toggle savestate / frame count */
    if (index.column() < 2)
        return false;

    /* Don't toggle past inputs */
    if (index.row() < static_cast<int>(context->framecount))
        return false;

    SingleInput si = input_set[index.column()-2];

    /* Don't toggle locked input */
    if (movie->locked_inputs.find(si) != movie->locked_inputs.end())
        return false;

    /* Add a row if necessary */
    if (index.row() == movie->input_list.size()) {
        insertRows(movie->input_list.size(), 1, QModelIndex());
    }

    AllInputs &ai = movie->input_list[index.row()];

    int value = ai.toggleInput(si);
    movie->wasModified();

    emit dataChanged(index, index);

    movie->modifiedSinceLastSave = true;
    movie->modifiedSinceLastAutoSave = true;
    return value;
}

std::string InputEditorModel::inputLabel(int column)
{
    return input_set[column-2].description;
}

void InputEditorModel::renameLabel(int column, std::string label)
{
    input_set[column-2].description = label;
    movie->input_names[input_set[column-2]] = label;
    emit dataChanged(createIndex(0, column), createIndex(rowCount(), column));
    emit inputSetChanged();
}


std::string InputEditorModel::inputDescription(int column)
{
    SingleInput si = input_set[column-2];

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

    const SingleInput si = input_set[column-2];
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
        movie->insertInputsBefore(ai, row);
    }

    endInsertRows();

    /* Update the movie framecount. Should it be done here ?? */
    context->config.sc.movie_framecount = movie->nbFrames();
    context->config.sc_modified = true;
    emit frameCountChanged();

    return true;
}

bool InputEditorModel::removeRows(int row, int count, const QModelIndex &parent)
{
    /* Don't delete past inputs */
    if (row < static_cast<int>(context->framecount))
        return false;

    beginRemoveRows(parent, row, row+count-1);

    for (int i=0; i<count; i++) {
        movie->deleteInputs(row);
    }

    endRemoveRows();

    /* Update the movie framecount. Should it be done here ?? */
    context->config.sc.movie_framecount = movie->nbFrames();
    context->config.sc_modified = true;
    emit frameCountChanged();

    return true;
}

void InputEditorModel::copyInputs(int row, int count)
{
    std::ostringstream inputString;

    /* Translate inputs into a string */
    for (int r=row; r < row+count; r++) {
        movie->writeFrame(inputString, movie->input_list[r]);
    }

    // QString qInputs(inputString)
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(inputString.str().c_str());
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
            movie->readFrame(line, ai);
            paste_ais.push_back(ai);
        }
    }

    /* Check if we need to insert frames */
    int insertedFrames = row + paste_ais.size() - movie->input_list.size();

    if (insertedFrames > 0) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + insertedFrames - 1);
    }

    for (size_t r = 0; r < paste_ais.size(); r++) {
        movie->setInputs(paste_ais[r], row + r, true);
        addUniqueInputs(paste_ais[r]);
    }

    if (insertedFrames > 0) {
        endInsertRows();
    }

    /* Update the movie framecount. Should it be done here ?? */
    context->config.sc.movie_framecount = movie->nbFrames();
    context->config.sc_modified = true;
    emit frameCountChanged();

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
            movie->readFrame(line, ai);
            paste_ais.push_back(ai);
        }
    }

    beginInsertRows(QModelIndex(), row, row + paste_ais.size() - 1);

    for (size_t r = 0; r < paste_ais.size(); r++) {
        movie->insertInputsBefore(paste_ais[r], row + r);
        addUniqueInputs(paste_ais[r]);
    }

    endInsertRows();

    /* Update the movie framecount. Should it be done here ?? */
    context->config.sc.movie_framecount = movie->nbFrames();
    context->config.sc_modified = true;
    emit frameCountChanged();

    emit inputSetChanged();

    return paste_ais.size();
}


void InputEditorModel::addUniqueInput(const SingleInput &si)
{
    /* Check if input is already present */
    for (SingleInput ti : input_set) {
        if (si == ti) {
            return;
        }
    }

    beginInsertColumns(QModelIndex(), columnCount(), columnCount());
    input_set.push_back(si);
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
        for (SingleInput ti : input_set) {
            if (si == ti) {
                new_input = false;
                break;
            }
        }

        /* Insert input if new */
        if (new_input) {

            /* Gather input name in the movie if there is one */
            auto it = movie->input_names.find(si);
            if (it != movie->input_names.end()) {
                si.description = it->second;
            }
            else {
                /* Gather input description */
                for (SingleInput ti : context->config.km.input_list) {
                    if (si == ti) {
                        si.description = ti.description;
                        break;
                    }
                }
            }

            addUniqueInput(si);
        }
    }
    emit inputSetChanged();
}

void InputEditorModel::clearUniqueInput(int column)
{
    SingleInput si = input_set[column-2];

    /* Don't clear locked input */
    if (movie->locked_inputs.find(si) != movie->locked_inputs.end())
        return;

    for (int f = static_cast<int>(context->framecount); f < movie->input_list.size(); f++) {
        movie->input_list[f].setInput(si, 0);
    }

    movie->wasModified();
}

bool InputEditorModel::isLockedUniqueInput(int column)
{
    if (column < 2)
        return false;

    SingleInput si = input_set[column-2];

    if (movie->locked_inputs.find(si) != movie->locked_inputs.end())
        return true;

    return false;
}


void InputEditorModel::lockUniqueInput(int column, bool locked)
{
    if (column < 2)
        return;

    SingleInput si = input_set[column-2];

    if (locked) {
        movie->locked_inputs.insert(si);
    }
    else {
        movie->locked_inputs.erase(si);
    }

    /* Update the input column */
    emit dataChanged(createIndex(0,column), createIndex(rowCount()-1,column));
}


void InputEditorModel::clearInput(int row)
{
    movie->input_list[row].emptyInputs();
    emit dataChanged(createIndex(row, 0), createIndex(row, columnCount()));

    movie->wasModified();
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
    addUniqueInputs(movie->input_list[movie->nbFrames()-1]);
}

void InputEditorModel::beginEditInputs()
{
}

void InputEditorModel::endEditInputs()
{
    emit dataChanged(createIndex(context->framecount,0), createIndex(context->framecount,columnCount()));

    /* We have to check if new inputs were added */
    addUniqueInputs(movie->input_list[context->framecount]);
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
    input_set.clear();
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
}
