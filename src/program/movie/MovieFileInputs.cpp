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
#include "IMovieAction.h"
#include "MovieActionEditFrames.h"
#include "MovieActionInsertFrames.h"
#include "MovieActionPaint.h"
#include "MovieActionRemoveFrames.h"

#include "utils.h"
#include "Context.h"
#include "../shared/version.h"
#include "../shared/inputs/AllInputs.h"
#include "../shared/inputs/ControllerInputs.h"
#include "../shared/inputs/MiscInputs.h"
#include "../shared/inputs/MouseInputs.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>

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

void MovieFileInputs::setFramerate(unsigned int num, unsigned int den, bool variable)
{
    framerate_num = num;
    framerate_den = den;
    variable_framerate = variable;
    InputSerialization::setFramerate(framerate_num, framerate_den);
}

void MovieFileInputs::clear()
{
    /* For new movies, will be overwritten when loading a moviefile */
    setFramerate(context->config.sc.initial_framerate_num, context->config.sc.initial_framerate_den, false);

    modifiedSinceLastSave = false;
    modifiedSinceLastAutoSave = false;
    modifiedSinceLastStateLoad = false;
    emit inputsToBeReset();
    input_list.clear();
    movie_changelog->clear();
    emit inputsReset();
}

void MovieFileInputs::load()
{
    emit inputsToBeReset();

    modifiedSinceLastSave = false;
    modifiedSinceLastAutoSave = false;
    modifiedSinceLastStateLoad = false;

    /* Clear structures */
    input_list.clear();
    
    /* Open the input file and parse each line to fill our input list */
    std::filesystem::path input_file = context->config.tempmoviedir / "inputs";
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
    std::filesystem::path input_file = context->config.tempmoviedir / "inputs";
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
    /* Check that we are writing to the next frame */
    if (pos == input_list.size()) {
        action_queue.push(new MovieActionInsertFrames(pos, inputs, this));
        return 0;
    }
    else if (pos < input_list.size()) {
        /* Writing to a frame that is before the last one. if keep_inputs is
         * false, we resize the input list accordingly and append the frame at
         * the end.
         */
        if (keep_inputs) {
            action_queue.push(new MovieActionEditFrames(pos, inputs, this));
        }
        else {
            action_queue.push(new MovieActionRemoveFrames(pos+1, input_list.size()-1, this));
            action_queue.push(new MovieActionEditFrames(pos, inputs, this));
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
    std::unique_lock<std::mutex> lock(input_list_mutex);

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

const AllInputs& MovieFileInputs::getInputsUnprotected(uint64_t pos)
{
    return input_list[pos];
}

void MovieFileInputs::clearInputs(int minFrame, int maxFrame)
{
    action_queue.push(new MovieActionEditFrames(minFrame, maxFrame, this));
}

void MovieFileInputs::paintInput(SingleInput si, int value, int minFrame, int maxFrame)
{
    action_queue.push(new MovieActionPaint(minFrame, maxFrame, si, value, this));
}

void MovieFileInputs::paintInput(SingleInput si, std::vector<int>& values, int minFrame)
{
    action_queue.push(new MovieActionPaint(minFrame, si, values, this));
}

void MovieFileInputs::editInputs(const std::vector<AllInputs>& inputs, uint64_t pos)
{
    if (!inputs.empty())
        action_queue.push(new MovieActionEditFrames(pos, inputs, this));
}

void MovieFileInputs::editInputs(const std::vector<AllInputs>& inputs, uint64_t pos, int count)
{
    if ((count > 0) && (inputs.size() >= count))
        action_queue.push(new MovieActionEditFrames(pos, pos+count-1, inputs, this));
}

void MovieFileInputs::insertInputsBefore(uint64_t pos, int count)
{
    if (count > 0)
        action_queue.push(new MovieActionInsertFrames(pos, count, this));
}

void MovieFileInputs::insertInputsBefore(const std::vector<AllInputs>& inputs, uint64_t pos)
{
    if (!inputs.empty())
        action_queue.push(new MovieActionInsertFrames(pos, inputs, this));
}

void MovieFileInputs::deleteInputs(uint64_t pos, int count)
{
    if (count > 0)
        action_queue.push(new MovieActionRemoveFrames(pos, pos+count-1, this));
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
    std::unique_lock<std::mutex> lock(input_list_mutex);

    emit inputsToBeReset();
    input_list.resize(movie_inputs->input_list.size());
    std::copy(movie_inputs->input_list.begin(), movie_inputs->input_list.end(), input_list.begin());
    movie_changelog->clear();
    emit inputsReset();
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

    /* We don't need to update movie length and send it to the game when not recording.
     * This can save a bit of time during fast-forward. */
    if (context->config.sc.recording != SharedConfig::NO_RECORDING)
        updateLength();
}

void MovieFileInputs::processPendingActions()
{
    /* Process input events */
    while (!action_queue.empty()) {
        IMovieAction* action;
        action_queue.pop(action);
        
        /* We must store old inputs here instead of during the action creation,
         * because old inputs may change depending on previous pending actions */
        action->storeOldInputs();
        
        movie_changelog->push(action);
        emit movie_changelog->updateChangeLog();
    }
}

uint64_t MovieFileInputs::size()
{
    return input_list.size();
}

void MovieFileInputs::updateLength()
{
    context->config.sc_modified = true;
    context->config.sc.movie_framecount = nbFrames();

    if (!variable_framerate) {
        /* Compute movie length from framecount */
        length_sec = (uint64_t)(context->config.sc.movie_framecount) * context->config.sc.initial_framerate_den / context->config.sc.initial_framerate_num;
        length_nsec = ((1000000000ull * (uint64_t)context->config.sc.movie_framecount * context->config.sc.initial_framerate_den) / context->config.sc.initial_framerate_num) % 1000000000ull;
        return;
    }

    /* Compute movie length by summing each frame length when variable fps */
    length_sec = 0;
    length_nsec = 0;

    /* Current framerate */
    uint32_t cur_framerate_num = framerate_num;
    uint32_t cur_framerate_den = framerate_den;

    /* Store one frame of time increment for the current framerate */
    int64_t increment_tv_sec = cur_framerate_den / cur_framerate_num;
    int64_t increment_tv_nsec = 1000000000LL * (int64_t)(cur_framerate_den % cur_framerate_num) / cur_framerate_num;
    int64_t fractional_increment = 1000000000LL * (int64_t)(cur_framerate_den % cur_framerate_num) % cur_framerate_num;
    int64_t fractional_part = 0;
    
    for (const AllInputs &ai : input_list) {

        uint32_t new_framerate_num = framerate_num;
        uint32_t new_framerate_den = framerate_den;

        if (ai.misc) {
            if (ai.misc->framerate_den)
                new_framerate_den = ai.misc->framerate_den;
            if (ai.misc->framerate_num)
                new_framerate_num = ai.misc->framerate_num;
        }

        /* Framerate was modified, update time increments */
        if (new_framerate_num != cur_framerate_num || new_framerate_den != cur_framerate_den) {
            cur_framerate_num = new_framerate_num;
            cur_framerate_den = new_framerate_den;

            increment_tv_sec = cur_framerate_den / cur_framerate_num;
            increment_tv_nsec = 1000000000LL * (int64_t)(cur_framerate_den % cur_framerate_num) / cur_framerate_num;
            fractional_increment = 1000000000LL * (int64_t)(cur_framerate_den % cur_framerate_num) % cur_framerate_num;
            fractional_part = 0;
        }

        /* Increment the current length */
        length_sec += increment_tv_sec;
        length_nsec += increment_tv_nsec;
        fractional_part += fractional_increment;
        while (fractional_part >= cur_framerate_num)
        {
            length_nsec++;
            fractional_part -= cur_framerate_num;
        }

        /* Sanitize current length */
        if (length_nsec >= 1000000000LL) {
            length_sec += length_nsec / 1000000000LL;
            length_nsec = length_nsec % 1000000000LL;
        }
    }
}
