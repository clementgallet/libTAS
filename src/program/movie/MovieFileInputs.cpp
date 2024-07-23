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
    clear();
}

void MovieFileInputs::setChangeLog(MovieFileChangeLog* mcl)
{
    movie_changelog = mcl;
}

void MovieFileInputs::clear()
{
    modifiedSinceLastSave = false;
    modifiedSinceLastAutoSave = false;
    modifiedSinceLastStateLoad = false;
    input_list.clear();
}

void MovieFileInputs::load()
{
    /* Clear structures */
    input_list.clear();
    
    /* Open the input file and parse each line to fill our input list */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ifstream input_stream(input_file);
    std::string line;

    while (std::getline(input_stream, line)) {
        if (!line.empty() && (line[0] == '|')) {
            AllInputs ai;
            int ret = readFrame(line, ai);
            if (ret >= 0)
                input_list.push_back(std::move(ai));
        }
    }

    input_stream.close();

    return;
}

void MovieFileInputs::save()
{
    /* Format and write input frames into the input file */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ofstream input_stream(input_file, std::ofstream::trunc);

    for (auto it = input_list.begin(); it != input_list.end(); ++it) {
        writeFrame(input_stream, *it);
    }
    input_stream.close();
}

int MovieFileInputs::writeFrame(std::ostream& input_stream, const AllInputs& inputs)
{
    /* Write only events if present */
    if (!inputs.events.empty()) {
        for (const auto& event : inputs.events) {
            input_stream.put('|');
            input_stream.put('E');
            input_stream << std::dec;
            input_stream << event.type << ':' << event.which << ':' << event.value;
        }
        input_stream << '|' << std::endl;
        return 1;
    }
    
    /* Write keyboard inputs */
    input_stream.put('|');
    input_stream.put('K');
    input_stream << std::hex;
    for (int k=0; k<AllInputs::MAXKEYS; k++) {
        if (!inputs.keyboard[k]) break;
        input_stream << (k>0?":":"") << inputs.keyboard[k];
    }

    /* Write mouse inputs */
    if (context->config.sc.mouse_support && inputs.pointer) {
        input_stream.put('|');
        input_stream.put('M');
        input_stream << std::dec;
        input_stream << inputs.pointer->x << ':' << inputs.pointer->y << ':';
        input_stream << ((inputs.pointer->mode == SingleInput::POINTER_MODE_RELATIVE)?"R:":"A:");
        input_stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B1))?'1':'.');
        input_stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B2))?'2':'.');
        input_stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B3))?'3':'.');
        input_stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B4))?'4':'.');
        input_stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B5))?'5':'.');
        input_stream << ":" << inputs.pointer->wheel;
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
        if (inputs.isDefaultController(joy))
            continue;
        /* Previous test ensures that there is an allocated ControllerInputs object */
        input_stream.put('|');
        input_stream.put('C');
        input_stream.put('1'+joy);
        input_stream << std::dec;
        for (int axis=0; axis<ControllerInputs::MAXAXES; axis++) {
            input_stream << inputs.controllers[joy]->axes[axis] << ':';
        }
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_A))?'A':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_B))?'B':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_X))?'X':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_Y))?'Y':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_BACK))?'b':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_GUIDE))?'g':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_START))?'s':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_LEFTSTICK))?'(':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_RIGHTSTICK))?')':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_LEFTSHOULDER))?'[':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_RIGHTSHOULDER))?']':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_UP))?'u':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_DOWN))?'d':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_LEFT))?'l':'.');
        input_stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_RIGHT))?'r':'.');
    }

    if (inputs.misc) {
        /* Write flag inputs */
        if (inputs.misc->flags) {
            input_stream << '|';
            input_stream << 'F';
            if (inputs.misc->flags & (1 << SingleInput::FLAG_RESTART)) input_stream.put('R');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED)) input_stream.put('1');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER2_ADDED_REMOVED)) input_stream.put('2');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER3_ADDED_REMOVED)) input_stream.put('3');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER4_ADDED_REMOVED)) input_stream.put('4');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_FOCUS_UNFOCUS)) input_stream.put('F');
        }
        
        /* Write framerate inputs */
        /* Only store framerate if different from initial framerate */
        if ((inputs.misc->framerate_num && (inputs.misc->framerate_num != framerate_num)) ||
            (inputs.misc->framerate_den && (inputs.misc->framerate_den != framerate_den))) {
            input_stream.put('|');
            input_stream.put('T');
            input_stream << std::dec;
            if (inputs.misc->framerate_num)
            input_stream << inputs.misc->framerate_num;
            else
            input_stream << framerate_num;
            input_stream << ':';
            if (inputs.misc->framerate_den)
            input_stream << inputs.misc->framerate_den;
            else
            input_stream << framerate_den;
        }
        
        /* Write realtime inputs */
        if (inputs.misc->realtime_sec) {
            input_stream.put('|');
            input_stream.put('D');
            input_stream << std::dec;
            input_stream << inputs.misc->realtime_sec << ':' << inputs.misc->realtime_nsec;
        }
    }

    input_stream << '|' << std::endl;

    return 1;
}

int MovieFileInputs::readFrame(const std::string& line, AllInputs& inputs)
{
    inputs.clear();
    int ret = 0;

    std::istringstream input_string(line);
    char d;
    input_string >> d;
    if (d != '|')
        return -1;

    d = input_string.peek();
    while (d != std::char_traits<char>::eof()) {
        switch (d) {
            case 'E':
                input_string >> d;
                ret = readEventFrame(input_string, inputs);
                break;
            case 'K':
                input_string >> d;
                ret = readKeyboardFrame(input_string, inputs);
                break;
            case 'M':
                input_string >> d;
                ret = readMouseFrame(input_string, inputs);
                break;
            case 'C':
                input_string >> d;
                input_string >> d;
                ret = readControllerFrame(input_string, inputs, d - '1');
                break;
            case 'F':
                input_string >> d;
                ret = readFlagFrame(input_string, inputs);
                break;
            case 'T':
                input_string >> d;
                ret = readFramerateFrame(input_string, inputs);
                break;
            case 'D':
                input_string >> d;
                ret = readRealtimeFrame(input_string, inputs);
                break;
            default:
                /* Following code is for old input format (1.3.5 and earlier) */
                ret = readKeyboardFrame(input_string, inputs);
                if (ret < 0)
                    return ret;

                if (context->config.sc.mouse_support) {
                    ret = readMouseFrame(input_string, inputs);
                    if (ret < 0)
                        return ret;
                }

                for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
                    ret = readControllerFrame(input_string, inputs, joy);
                    if (ret < 0)
                        return ret;
                }

                ret = readFlagFrame(input_string, inputs);
                if (ret < 0)
                    return ret;

                if (context->config.sc.variable_framerate) {
                    ret = readFramerateFrame(input_string, inputs);
                    if (ret < 0)
                        return ret;
                }
                break;
        }
        if (ret < 0)
            return ret;
        d = input_string.peek();
    }

    /* If we imported events, we need to fill the remaining state to the state
     * at the end of event processing. */
    inputs.processEvents();

    return 0;
}

int MovieFileInputs::readEventFrame(std::istringstream& input_string, AllInputs& inputs)
{
    char d;
    InputEvent ie;
    input_string >> std::dec;
    input_string >> ie.type >> d;
    if (d != ':') return -1;
    input_string >> ie.which >> d;
    if (d != ':') return -1;
    input_string >> ie.value >> d;

    inputs.events.push_back(ie);
    return 0;
}

int MovieFileInputs::readKeyboardFrame(std::istringstream& input_string, AllInputs& inputs)
{
    input_string >> std::hex;
    char d = input_string.peek();
    if (d == '|') {
        input_string.get();
        return 0;
    }
    for (int k=0; (k<AllInputs::MAXKEYS) && input_string; k++) {
        input_string >> inputs.keyboard[k] >> d;
        if (d == '|') {
            break;
        }
        if (d != ':') return -1;
    }
    return 0;
}

int MovieFileInputs::readMouseFrame(std::istringstream& input_string, AllInputs& inputs)
{
    if (!inputs.pointer)
        inputs.pointer.reset(new MouseInputs{});

    char d;
    input_string >> std::dec;
    input_string >> inputs.pointer->x >> d;
    if (d != ':') return -1;
    input_string >> inputs.pointer->y >> d;
    if (d != ':') return -1;
    input_string >> d;
    if ((d == 'R') || (d == 'A')) {
        /* Read mouse mode */
        if (d == 'R') inputs.pointer->mode = SingleInput::POINTER_MODE_RELATIVE;
        else inputs.pointer->mode = SingleInput::POINTER_MODE_ABSOLUTE;
        input_string >> d;
        input_string >> d;
    }
    else {
        inputs.pointer->mode = SingleInput::POINTER_MODE_ABSOLUTE;
    }
    inputs.pointer->mask = 0;
    if (d != '.') inputs.pointer->mask |= (1 << SingleInput::POINTER_B1);
    input_string >> d;
    if (d != '.') inputs.pointer->mask |= (1 << SingleInput::POINTER_B2);
    input_string >> d;
    if (d != '.') inputs.pointer->mask |= (1 << SingleInput::POINTER_B3);
    input_string >> d;
    if (d != '.') inputs.pointer->mask |= (1 << SingleInput::POINTER_B4);
    input_string >> d;
    if (d != '.') inputs.pointer->mask |= (1 << SingleInput::POINTER_B5);

    input_string >> d;

    /* Check for optional wheel value */
    if (d == ':')
        input_string >> inputs.pointer->wheel >> d;
    return 0;
}

int MovieFileInputs::readControllerFrame(std::istringstream& input_string, AllInputs& inputs, int joy)
{
    if ((joy < 0) || (joy > AllInputs::MAXJOYS))
    return -1;

    char d;
    input_string >> std::dec;
    if (!inputs.controllers[joy])
        inputs.controllers[joy].reset(new ControllerInputs{});
    for (int axis=0; axis<ControllerInputs::MAXAXES; axis++) {
        input_string >> inputs.controllers[joy]->axes[axis] >> d;
        if (d != ':') return -1;
    }
    for (int b=0; b<15; b++) {
        input_string >> d;
        if (d != '.') inputs.controllers[joy]->buttons |= (1 << b);
    }
    input_string >> d;
    return 0;
}

int MovieFileInputs::readFlagFrame(std::istringstream& input_string, AllInputs& inputs)
{
    if (!inputs.misc)
        inputs.misc.reset(new MiscInputs{});
    
    char d;
    input_string >> d;
    while (input_string && (d != '|')) {
        switch (d) {
            case 'R': inputs.misc->flags |= (1 << SingleInput::FLAG_RESTART); break;
            case '1': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED); break;
            case '2': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER2_ADDED_REMOVED); break;
            case '3': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER3_ADDED_REMOVED); break;
            case '4': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER4_ADDED_REMOVED); break;
            case 'F': inputs.misc->flags |= (1 << SingleInput::FLAG_FOCUS_UNFOCUS); break;
            /* Old unused flags */
            case 'I': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED); break;
            case 'L': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER2_ADDED_REMOVED); break;
            case 'U': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER3_ADDED_REMOVED); break;
            case 'O': inputs.misc->flags |= (1 << SingleInput::FLAG_CONTROLLER4_ADDED_REMOVED); break;
        }
        input_string >> d;
    }
    return 0;
}

int MovieFileInputs::readFramerateFrame(std::istringstream& input_string, AllInputs& inputs)
{
    if (!inputs.misc)
        inputs.misc.reset(new MiscInputs{});
    char d;
    input_string >> std::dec;
    input_string >> inputs.misc->framerate_num >> d >> inputs.misc->framerate_den >> d;
    return 0;
}

int MovieFileInputs::readRealtimeFrame(std::istringstream& input_string, AllInputs& inputs)
{
    if (!inputs.misc)
        inputs.misc.reset(new MiscInputs{});
    char d;
    input_string >> std::dec;
    input_string >> inputs.misc->realtime_sec >> d >> inputs.misc->realtime_nsec >> d;
    return 0;
}

uint64_t MovieFileInputs::nbFrames()
{
    std::unique_lock<std::mutex> lock(input_list_mutex);
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
    std::unique_lock<std::mutex> lock(input_list_mutex);

    if (pos < context->framecount)
        return -1;
        
    /* Check that we are writing to the next frame */
    if (pos == input_list.size()) {
        movie_changelog->registerInsertFrame(pos, inputs);
        input_list.push_back(inputs);
        wasModified();
        return 0;
    }
    else if (pos < input_list.size()) {
        /* Writing to a frame that is before the last one. if keep_inputs is
         * false, we resize the input list accordingly and append the frame at
         * the end.
         */
        if (keep_inputs) {
            movie_changelog->registerEditFrame(pos, inputs);
            input_list[pos] = inputs;
        }
        else {
            movie_changelog->registerRemoveFrames(pos+1, input_list.size()-1);
            movie_changelog->registerEditFrame(pos, inputs);
            input_list.resize(pos);
            input_list.push_back(inputs);
        }
        wasModified();
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
        pos = input_list.size();
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
    std::unique_lock<std::mutex> lock(input_list_mutex);

    if (maxFrame >= input_list.size())
        return;
    
    movie_changelog->registerClearFrames(minFrame, maxFrame);

    for (int i = minFrame; i <= maxFrame; i++)
        input_list[i].clear();
    wasModified();
}

void MovieFileInputs::paintInput(SingleInput si, int value, int minFrame, int maxFrame)
{
    movie_changelog->registerPaint(minFrame, maxFrame, si, value);
    for (int i = minFrame; i <= maxFrame; i++)
        queueInput(i, si, value, false);
}

void MovieFileInputs::editInputs(const std::vector<AllInputs>& inputs, uint64_t pos)
{
    return editInputs(inputs, pos, inputs.size());
}

void MovieFileInputs::editInputs(const std::vector<AllInputs>& inputs, uint64_t pos, int count)
{
    std::unique_lock<std::mutex> lock(input_list_mutex);

    if ((pos + count) > input_list.size())
        return;

    movie_changelog->registerEditFrames(pos, pos+count-1, inputs);
    std::copy(inputs.begin(), inputs.begin() + count, input_list.begin() + pos);
    wasModified();
}

void MovieFileInputs::insertInputsBefore(uint64_t pos, int count)
{
    std::unique_lock<std::mutex> lock(input_list_mutex);

    if (pos > input_list.size())
        return;

    AllInputs ai;
    ai.clear();

    movie_changelog->registerInsertFrames(pos, count);
    input_list.insert(input_list.begin() + pos, count, ai);
    wasModified();
}

void MovieFileInputs::insertInputsBefore(const std::vector<AllInputs>& inputs, uint64_t pos)
{
    std::unique_lock<std::mutex> lock(input_list_mutex);

    if (pos > input_list.size())
        return;

    movie_changelog->registerInsertFrames(pos, inputs);
    input_list.insert(input_list.begin() + pos, inputs.begin(), inputs.end());
    wasModified();
}

void MovieFileInputs::deleteInputs(uint64_t pos, int count)
{
    std::unique_lock<std::mutex> lock(input_list_mutex);

    if ((pos + count) > input_list.size())
        return;

    movie_changelog->registerRemoveFrames(pos, pos+count-1);
    if ((pos + count) == input_list.size())
        input_list.resize(pos);
    else
        input_list.erase(input_list.begin() + pos, input_list.begin() + pos + count);
    wasModified();
}

void MovieFileInputs::extractInputs(std::set<SingleInput> &set)
{
    std::unique_lock<std::mutex> lock(input_list_mutex);

    for (const AllInputs &ai : input_list) {
        ai.extractInputs(set);
    }
}

void MovieFileInputs::copyTo(MovieFileInputs* movie_inputs) const
{
    movie_inputs->input_list.resize(input_list.size());
    std::copy(input_list.begin(), input_list.end(), movie_inputs->input_list.begin());
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

uint64_t MovieFileInputs::processPendingInputs()
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
        else
            ai.setInput(ie.si, ie.value);
        wasModified();
        return ie.framecount;
    }
    return UINT64_MAX;
}

uint64_t MovieFileInputs::size()
{
    return input_list.size();
}
