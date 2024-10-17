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

#include "MovieFileInputs.h"
#include "MovieFileChangeLog.h"
#include "InputSerialization.h"

#include "utils.h"
#include "Context.h"
#include "../shared/version.h"
#include "../shared/inputs/AllInputs.h"
#include "../shared/inputs/ControllerInputs.h"
#include "../shared/inputs/MiscInputs.h"
#include "../shared/inputs/MouseInputs.h"

#include <QtCore/QSettings>
#include <iostream>
#include <sstream>
#include <algorithm>

MovieFileInputs::MovieFileInputs(Context* c) : context(c)
{
    InputSerialization::setContext(c);
    modifiedSinceLastSave = false;
    modifiedSinceLastAutoSave = false;
    modifiedSinceLastStateLoad = false;
}

void MovieFileInputs::setChangeLog(MovieFileChangeLog* mcl)
{
    movie_changelog = mcl;
}

void MovieFileInputs::setFramerate(unsigned int num, unsigned int den)
{
    framerate_num = num;
    framerate_den = den;
    InputSerialization::setFramerate(framerate_num, framerate_den);
}

void MovieFileInputs::clear()
{
    modifiedSinceLastSave = false;
    modifiedSinceLastAutoSave = false;
    modifiedSinceLastStateLoad = false;
    emit inputsToBeReset();
    input_list.clear();
    emit inputsReset();
    movie_changelog->clear();
}

void MovieFileInputs::load()
{
    emit inputsToBeReset();

    /* Clear structures */
    input_list.clear();
    
    /* Open the input file and parse each line to fill our input list */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ifstream input_stream(input_file);
    
    InputSerialization::readInputs(input_stream, input_list);

    input_stream.close();

    movie_changelog->clear();
    emit inputsReset();
    return;
}

void MovieFileInputs::save()
{
    /* Format and write input frames into the input file */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ofstream input_stream(input_file, std::ofstream::trunc);

    InputSerialization::writeInputs(input_stream, input_list);

    input_stream.close();
}

uint64_t MovieFileInputs::nbFrames()
{
    return input_list.size();
}

int MovieFileInputs::setInputs(const AllInputs& inputs, uint64_t pos)
{
    return setInputs(inputs, pos, true);
}

int MovieFileInputs::setInputs(const AllInputs& inputs)
{
    return setInputs(inputs, context->framecount, true);    
}

int MovieFileInputs::setInputs(const AllInputs& inputs, bool keep_inputs)
{
    return setInputs(inputs, context->framecount, keep_inputs);
}

int MovieFileInputs::setInputs(const AllInputs& inputs, uint64_t pos, bool keep_inputs)
{
    if (pos < context->framecount)
        return -1;
        
    /* Check that we are writing to the next frame */
    if (pos == input_list.size()) {
        movie_changelog->registerInsertFrame(pos, inputs);
        return 0;
    }
    else if (pos < input_list.size()) {
        /* Writing to a frame that is before the last one. if keep_inputs is
         * false, we resize the input list accordingly and append the frame at
         * the end.
         */
        if (keep_inputs) {
            movie_changelog->registerEditFrame(pos, inputs);
        }
        else {
            movie_changelog->registerRemoveFrames(pos+1, input_list.size()-1);
            movie_changelog->registerEditFrame(pos, inputs);
        }
        return 0;
    }
    else {
        std::cerr << "Writing to a frame " << pos << " higher than the current list " << input_list.size() << std::endl;
        return 1;
    }
}

const AllInputs& MovieFileInputs::getInputs()
{
    return getInputs(context->framecount);
}

const AllInputs& MovieFileInputs::getInputs(uint64_t pos)
{
    // std::unique_lock<std::mutex> lock(input_list_mutex);

    if (pos >= input_list.size()) {
        pos = input_list.size() - 1;
    }

    /* Special case for zero framerate */
    if (input_list[pos].misc) {
        if (!input_list[pos].misc->framerate_num)
            input_list[pos].misc->framerate_num = framerate_num;
        if (!input_list[pos].misc->framerate_den)
            input_list[pos].misc->framerate_den = framerate_den;
    }

    return input_list[pos];
}

void MovieFileInputs::clearInputs(int minFrame, int maxFrame)
{
    if (maxFrame >= input_list.size())
        return;
    
    movie_changelog->registerClearFrames(minFrame, maxFrame);
}

void MovieFileInputs::paintInput(SingleInput si, int value, int minFrame, int maxFrame)
{
    movie_changelog->registerPaint(minFrame, maxFrame, si, value);
}

void MovieFileInputs::paintInput(SingleInput si, std::vector<int>& values, int minFrame)
{
    movie_changelog->registerPaint(minFrame, si, values);
}

void MovieFileInputs::editInputs(const std::vector<AllInputs>& inputs, uint64_t pos)
{
    return editInputs(inputs, pos, inputs.size());
}

void MovieFileInputs::editInputs(const std::vector<AllInputs>& inputs, uint64_t pos, int count)
{
    if ((pos + count) > input_list.size())
        return;

    movie_changelog->registerEditFrames(pos, pos+count-1, inputs);
}

void MovieFileInputs::insertInputsBefore(uint64_t pos, int count)
{
    if (pos > input_list.size())
        return;
        
    movie_changelog->registerInsertFrames(pos, count);
}

void MovieFileInputs::insertInputsBefore(const std::vector<AllInputs>& inputs, uint64_t pos)
{
    if (pos > input_list.size())
        return;

    movie_changelog->registerInsertFrames(pos, inputs);
}

void MovieFileInputs::deleteInputs(uint64_t pos, int count)
{
    if ((pos + count) > input_list.size())
        return;

    movie_changelog->registerRemoveFrames(pos, pos+count-1);
}

void MovieFileInputs::extractInputs(std::set<SingleInput> &set)
{
    std::unique_lock<std::mutex> lock(input_list_mutex);

    for (const AllInputs &ai : input_list) {
        ai.extractInputs(set);
    }
}

void MovieFileInputs::copyFrom(const MovieFileInputs* movie_inputs)
{
    emit inputsToBeReset();
    input_list.resize(movie_inputs->input_list.size());
    std::copy(movie_inputs->input_list.begin(), movie_inputs->input_list.end(), input_list.begin());
    emit inputsReset();
    movie_changelog->clear();
}

void MovieFileInputs::close()
{
    input_list.clear();
}

bool MovieFileInputs::isEqual(const MovieFileInputs* movie, unsigned int start_frame, unsigned int end_frame) const
{
    /* Not equal if a size is greater */
    if (end_frame > input_list.size())
        return false;

    if (end_frame > movie->input_list.size())
        return false;

    return std::equal(movie->input_list.begin() + start_frame, movie->input_list.begin() + end_frame, input_list.begin() + start_frame);
}

void MovieFileInputs::wasModified()
{
    modifiedSinceLastSave = true;
    modifiedSinceLastAutoSave = true;
    modifiedSinceLastStateLoad = true;
}

void MovieFileInputs::queueInput(uint64_t pos, SingleInput si, int value, bool isEvent)
{
    InputPending ie;
    ie.framecount = pos;
    ie.si = si;
    ie.value = value;
    ie.isEvent = isEvent;
    input_queue.push(ie);
}

void MovieFileInputs::processPendingInputs()
{
    /* Process input events */
    while (!input_queue.empty()) {
        InputPending ie;
        input_queue.pop(ie);
        
        /* Check for setting inputs before current framecount */
        if (ie.framecount < context->framecount)
            continue;
        
        std::unique_lock<std::mutex> lock(input_list_mutex);

        if (ie.framecount >= input_list.size())
            continue;

        AllInputs& ai = input_list[ie.framecount];
        if ((ie.si.type == SingleInput::IT_NONE) && ie.isEvent)
            ai.clear();
        else if (ie.isEvent) {
            ai.events.push_back({ie.si.type, ie.si.which, ie.value});
            ai.processEvents(); // TODO: Unoptimal to call it everytime
        }
        else {
            emit inputsToBeEdited(ie.framecount, ie.framecount);
            ai.setInput(ie.si, ie.value);
            emit inputsEdited(ie.framecount, ie.framecount);
        }
        wasModified();
    }
}

uint64_t MovieFileInputs::size()
{
    return input_list.size();
}
