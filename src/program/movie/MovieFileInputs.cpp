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

#include <QtCore/QSettings>
#include <iostream>
#include <sstream>

#include "MovieFileInputs.h"
#include "../utils.h"
#include "../../shared/version.h"

MovieFileInputs::MovieFileInputs(Context* c) : context(c)
{
    rek.assign(R"(\|K([0-9a-f]*(?::[0-9a-f]+)*)\|)", std::regex::ECMAScript|std::regex::optimize);
    rem.assign(R"(\|M([\-0-9]+:[\-0-9]+:(?:[AR]:)?[\.1-5]{5})\|)", std::regex::ECMAScript|std::regex::optimize);
    rec.assign(R"(\|C([1-4](?:[\-0-9]+:){6}.{15})\|)", std::regex::ECMAScript|std::regex::optimize);
    ref.assign(R"(\|F(.{1,9})\|)", std::regex::ECMAScript|std::regex::optimize);
    ret.assign(R"(\|T([0-9]+:[0-9]+)\|)", std::regex::ECMAScript|std::regex::optimize);
    
    clear();
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
            readFrame(line, ai);
            input_list.push_back(ai);
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
    /* Write keyboard inputs */
    input_stream.put('|');
    input_stream.put('K');
    input_stream << std::hex;
    for (int k=0; k<AllInputs::MAXKEYS; k++) {
        if (!inputs.keyboard[k]) break;
        input_stream << (k>0?":":"") << inputs.keyboard[k];
    }

    /* Write mouse inputs */
    if (context->config.sc.mouse_support) {
        input_stream.put('|');
        input_stream.put('M');
        input_stream << std::dec;
        input_stream << inputs.pointer_x << ':' << inputs.pointer_y << ':';
        input_stream << ((inputs.pointer_mode == SingleInput::POINTER_MODE_RELATIVE)?"R:":"A:");
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B1))?'1':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B2))?'2':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B3))?'3':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B4))?'4':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B5))?'5':'.');
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
        if (inputs.isDefaultController(joy))
            continue;
        input_stream.put('|');
        input_stream.put('C');
        input_stream.put('1'+joy);
        input_stream << std::dec;
        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            input_stream << inputs.controller_axes[joy][axis] << ':';
        }
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_A))?'A':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_B))?'B':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_X))?'X':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_Y))?'Y':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_BACK))?'b':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_GUIDE))?'g':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_START))?'s':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_LEFTSTICK))?'(':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_RIGHTSTICK))?')':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_LEFTSHOULDER))?'[':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_RIGHTSHOULDER))?']':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_UP))?'u':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_DOWN))?'d':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_LEFT))?'l':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_RIGHT))?'r':'.');
    }

    /* Write flag inputs */
    if (inputs.flags) {
        input_stream << '|';
        input_stream << 'F';
        if (inputs.flags & (1 << SingleInput::FLAG_RESTART)) input_stream.put('R');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER1_ADDED)) input_stream.put('1');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER2_ADDED)) input_stream.put('2');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER3_ADDED)) input_stream.put('3');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER4_ADDED)) input_stream.put('4');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER1_REMOVED)) input_stream.put('I');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER2_REMOVED)) input_stream.put('L');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER3_REMOVED)) input_stream.put('U');
        if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER4_REMOVED)) input_stream.put('O');
    }

    /* Write mouse inputs */
    if (context->config.sc.variable_framerate) {
        /* Zero framerate is default framerate */
        if (inputs.framerate_num) {
            /* Only store framerate if different from initial framerate */
            if ((inputs.framerate_num != framerate_num) || (inputs.framerate_den != framerate_den)) {
                input_stream.put('|');
                input_stream.put('T');
                input_stream << std::dec;
                input_stream << inputs.framerate_num << ':' << inputs.framerate_den;
            }
        }
    }
    input_stream << '|' << std::endl;

    return 1;
}

int MovieFileInputs::readFrame(const std::string& line, AllInputs& inputs)
{
    inputs.emptyInputs();

    std::istringstream input_string(line);
    char d;
    input_string >> d;

    /* Read keyboard inputs using regex first. If matched, read the rest of the
     * inputs with regex as well. */

    std::smatch match;
    if (std::regex_search(line, match, rek) && match.size() > 1) {
        std::istringstream key_string(match.str(1));
        readKeyboardFrame(key_string, inputs);

        /* Read mouse inputs */
        if (std::regex_search(line, match, rem) && match.size() > 1) {
            std::istringstream mouse_string(match.str(1));
            readMouseFrame(mouse_string, inputs);
        }

        /* Read controller inputs */
        std::sregex_iterator next(line.begin(), line.end(), rec);
        std::sregex_iterator end;
        if (next != end) {
            while (next != end) {
                std::smatch match = *next;
                std::istringstream controller_string(match.str(1));

                /* Extract joystick number */
                char j;
                controller_string >> j;

                /* Read joystick inputs */
                readControllerFrame(controller_string, inputs, j - '1');
                next++;
            }
        }

        /* Read flag inputs */
        if (std::regex_search(line, match, ref) && match.size() > 1) {
            std::istringstream flag_string(match.str(1));
            readFlagFrame(flag_string, inputs);
        }

        /* Read framerate inputs */
        if (std::regex_search(line, match, ret) && match.size() > 1) {
            std::istringstream framerate_string(match.str(1));
            readFramerateFrame(framerate_string, inputs);
        }
        else {
            /* Write initial framerate values */
            // inputs.framerate_num = framerate_num;
            // inputs.framerate_den = framerate_den;
        }

        return 1;
    }

    /* Following code is for old input format (1.3.5 and earlier) */

    /* Read keyboard inputs */
    readKeyboardFrame(input_string, inputs);

    /* Read mouse inputs */
    if (context->config.sc.mouse_support) {
        readMouseFrame(input_string, inputs);
    }

    /* Read controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
        readControllerFrame(input_string, inputs, joy);
    }

    /* Read flag inputs */
    readFlagFrame(input_string, inputs);

    /* Read framerate inputs */
    if (context->config.sc.variable_framerate) {
        readFramerateFrame(input_string, inputs);
    }

    return 1;
}

void MovieFileInputs::readKeyboardFrame(std::istringstream& input_string, AllInputs& inputs)
{
    input_string >> std::hex;
    char d = input_string.peek();
    if (d == '|') {
        input_string.get();
        return;
    }
    for (int k=0; (k<AllInputs::MAXKEYS) && input_string; k++) {
        input_string >> inputs.keyboard[k] >> d;
        if (d == '|') {
            break;
        }
    }
}

void MovieFileInputs::readMouseFrame(std::istringstream& input_string, AllInputs& inputs)
{
    char d;
    input_string >> std::dec;
    input_string >> inputs.pointer_x >> d >> inputs.pointer_y >> d;
    input_string >> d;
    if ((d == 'R') || (d == 'A')) {
        /* Read mouse mode */
        if (d == 'R') inputs.pointer_mode = SingleInput::POINTER_MODE_RELATIVE;
        else inputs.pointer_mode = SingleInput::POINTER_MODE_ABSOLUTE;
        input_string >> d;
        input_string >> d;
    }
    else {
        inputs.pointer_mode = SingleInput::POINTER_MODE_ABSOLUTE;
    }
    if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B1);
    input_string >> d;
    if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B2);
    input_string >> d;
    if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B3);
    input_string >> d;
    if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B4);
    input_string >> d;
    if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B5);
    input_string >> d;
}

void MovieFileInputs::readControllerFrame(std::istringstream& input_string, AllInputs& inputs, int joy)
{
    char d;
    input_string >> std::dec;
    for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
        input_string >> inputs.controller_axes[joy][axis] >> d;
    }
    for (int b=0; b<15; b++) {
        input_string >> d;
        if (d != '.') inputs.controller_buttons[joy] |= (1 << b);
    }
    input_string >> d;
}

void MovieFileInputs::readFlagFrame(std::istringstream& input_string, AllInputs& inputs)
{
    char d;
    input_string >> d;
    while (input_string && (d != '|')) {
        switch (d) {
            case 'R': inputs.flags |= (1 << SingleInput::FLAG_RESTART); break;
            case '1': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER1_ADDED); break;
            case '2': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER2_ADDED); break;
            case '3': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER3_ADDED); break;
            case '4': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER4_ADDED); break;
            case 'I': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER1_REMOVED); break;
            case 'L': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER2_REMOVED); break;
            case 'U': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER3_REMOVED); break;
            case 'O': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER4_REMOVED); break;
        }
        input_string >> d;
    }
}

void MovieFileInputs::readFramerateFrame(std::istringstream& input_string, AllInputs& inputs)
{
    char d;
    input_string >> std::dec;
    input_string >> inputs.framerate_num >> d >> inputs.framerate_den >> d;
}

uint64_t MovieFileInputs::nbFrames() const
{
    return input_list.size();
}

void MovieFileInputs::updateLength() const
{
    if (context->config.sc.movie_framecount != nbFrames()) {
        context->config.sc.movie_framecount = nbFrames();

        /* Unvalidate movie length when variable framerate */
        if (context->config.sc.variable_framerate) {
            context->movie_time_sec = -1;
            context->movie_time_nsec = -1;
        }
        else {
            /* Compute movie length from framecount */
            context->movie_time_sec = (uint64_t)(context->config.sc.movie_framecount) * context->config.sc.framerate_den / context->config.sc.framerate_num;
            context->movie_time_nsec = ((1000000000ull * (uint64_t)context->config.sc.movie_framecount * context->config.sc.framerate_den) / context->config.sc.framerate_num) % 1000000000ull;
        }

        context->config.sc_modified = true;
    }
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
            input_list[pos] = inputs;
        }
        else {
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

int MovieFileInputs::getInputs(AllInputs& inputs) const
{
    return getInputs(inputs, context->framecount);
}

int MovieFileInputs::getInputs(AllInputs& inputs, uint64_t pos) const
{
    if (pos >= input_list.size()) {
        inputs.emptyInputs();
        if (pos == input_list.size())
            return -2;
        return -1;
    }

    inputs = input_list[pos];

    /* Special case for zero framerate */
    if (!inputs.framerate_num) {
        inputs.framerate_num = framerate_num;
        inputs.framerate_den = framerate_den;
    }

    if ((pos + 1) == input_list.size()) {
        /* We are reading the last frame of the movie, notify the caller */
        return 1;
    }

    return 0;
}

void MovieFileInputs::insertInputsBefore(const AllInputs& inputs, uint64_t pos)
{
    if (pos > input_list.size())
        return;

    input_list.insert(input_list.begin() + pos, inputs);
    wasModified();
}

void MovieFileInputs::deleteInputs(uint64_t pos)
{
    if (pos >= input_list.size())
        return;

    input_list.erase(input_list.begin() + pos);
    wasModified();
}

// void MovieFileInputs::truncateInputs(uint64_t size)
// {
//     input_list.resize(size);
//     wasModified();
// }

void MovieFileInputs::close()
{
    input_list.clear();
}

bool MovieFileInputs::isPrefix(const MovieFileInputs* movie, unsigned int frame) const
{
    /* Not a prefix if the size is greater */
    if (frame > input_list.size())
        return false;

    return std::equal(movie->input_list.begin(), movie->input_list.begin() + frame, input_list.begin());
}

void MovieFileInputs::wasModified()
{
    modifiedSinceLastSave = true;
    modifiedSinceLastAutoSave = true;
    modifiedSinceLastStateLoad = true;
}

uint64_t MovieFileInputs::processEvent()
{
    /* Process input events */
    while (!input_event_queue.empty()) {
        InputEvent ie;
        input_event_queue.pop(ie);
        
        /* Check for setting inputs before current framecount */
        if (ie.framecount < context->framecount)
            continue;
            
        AllInputs ai;
        int ret = getInputs(ai, ie.framecount);
        if (ret >= 0) {
            ai.setInput(ie.si, ie.value);
            setInputs(ai, ie.framecount, true);
            return ie.framecount;
        }
    }
    return UINT64_MAX;
}
