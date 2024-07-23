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

#include "InputSerialization.h"

#include "Context.h"
#include "../shared/inputs/AllInputs.h"
#include "../shared/inputs/ControllerInputs.h"
#include "../shared/inputs/MiscInputs.h"
#include "../shared/inputs/MouseInputs.h"

#include <sstream>

/* Initial framerate values */
static unsigned int framerate_num, framerate_den;
static Context* context;

void InputSerialization::setContext(Context* c)
{
    context = c;
}

void InputSerialization::setFramerate(unsigned int num, unsigned int den)
{
    framerate_num = num;
    framerate_den = den;
}

void InputSerialization::writeInputs(std::ostream& stream, const std::vector<AllInputs>& input_list)
{
    for (auto it = input_list.begin(); it != input_list.end(); ++it) {
        writeFrame(stream, *it);
    }
}

void InputSerialization::readInputs(std::istream& stream, std::vector<AllInputs>& input_list)
{
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && (line[0] == '|')) {
            AllInputs ai;
            int ret = readFrame(line, ai);
            if (ret >= 0)
                input_list.push_back(std::move(ai));
            else
                return;
        }
    }
}

int InputSerialization::writeFrame(std::ostream& stream, const AllInputs& inputs)
{
    /* Write only events if present */
    if (!inputs.events.empty()) {
        for (const auto& event : inputs.events) {
            stream.put('|');
            stream.put('E');
            stream << std::dec;
            stream << event.type << ':' << event.which << ':' << event.value;
        }
        stream << '|' << std::endl;
        return 1;
    }
    
    /* Write keyboard inputs */
    stream.put('|');
    stream.put('K');
    stream << std::hex;
    for (int k=0; k<AllInputs::MAXKEYS; k++) {
        if (!inputs.keyboard[k]) break;
        stream << (k>0?":":"") << inputs.keyboard[k];
    }

    /* Write mouse inputs */
    if (context->config.sc.mouse_support && inputs.pointer) {
        stream.put('|');
        stream.put('M');
        stream << std::dec;
        stream << inputs.pointer->x << ':' << inputs.pointer->y << ':';
        stream << ((inputs.pointer->mode == SingleInput::POINTER_MODE_RELATIVE)?"R:":"A:");
        stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B1))?'1':'.');
        stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B2))?'2':'.');
        stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B3))?'3':'.');
        stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B4))?'4':'.');
        stream.put((inputs.pointer->mask&(1<<SingleInput::POINTER_B5))?'5':'.');
        stream << ":" << inputs.pointer->wheel;
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
        if (inputs.isDefaultController(joy))
            continue;
        /* Previous test ensures that there is an allocated ControllerInputs object */
        stream.put('|');
        stream.put('C');
        stream.put('1'+joy);
        stream << std::dec;
        for (int axis=0; axis<ControllerInputs::MAXAXES; axis++) {
            stream << inputs.controllers[joy]->axes[axis] << ':';
        }
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_A))?'A':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_B))?'B':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_X))?'X':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_Y))?'Y':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_BACK))?'b':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_GUIDE))?'g':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_START))?'s':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_LEFTSTICK))?'(':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_RIGHTSTICK))?')':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_LEFTSHOULDER))?'[':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_RIGHTSHOULDER))?']':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_UP))?'u':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_DOWN))?'d':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_LEFT))?'l':'.');
        stream.put((inputs.controllers[joy]->buttons&(1<<SingleInput::BUTTON_DPAD_RIGHT))?'r':'.');
    }

    if (inputs.misc) {
        /* Write flag inputs */
        if (inputs.misc->flags) {
            stream << '|';
            stream << 'F';
            if (inputs.misc->flags & (1 << SingleInput::FLAG_RESTART)) stream.put('R');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED)) stream.put('1');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER2_ADDED_REMOVED)) stream.put('2');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER3_ADDED_REMOVED)) stream.put('3');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_CONTROLLER4_ADDED_REMOVED)) stream.put('4');
            if (inputs.misc->flags & (1 << SingleInput::FLAG_FOCUS_UNFOCUS)) stream.put('F');
        }
        
        /* Write framerate inputs */
        /* Only store framerate if different from initial framerate */
        if ((inputs.misc->framerate_num && (inputs.misc->framerate_num != framerate_num)) ||
            (inputs.misc->framerate_den && (inputs.misc->framerate_den != framerate_den))) {
            stream.put('|');
            stream.put('T');
            stream << std::dec;
            if (inputs.misc->framerate_num)
            stream << inputs.misc->framerate_num;
            else
            stream << framerate_num;
            stream << ':';
            if (inputs.misc->framerate_den)
            stream << inputs.misc->framerate_den;
            else
            stream << framerate_den;
        }
        
        /* Write realtime inputs */
        if (inputs.misc->realtime_sec) {
            stream.put('|');
            stream.put('D');
            stream << std::dec;
            stream << inputs.misc->realtime_sec << ':' << inputs.misc->realtime_nsec;
        }
    }

    stream << '|' << std::endl;

    return 1;
}

int InputSerialization::readFrame(const std::string& line, AllInputs& inputs)
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

int InputSerialization::readEventFrame(std::istringstream& input_string, AllInputs& inputs)
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

int InputSerialization::readKeyboardFrame(std::istringstream& input_string, AllInputs& inputs)
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

int InputSerialization::readMouseFrame(std::istringstream& input_string, AllInputs& inputs)
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

int InputSerialization::readControllerFrame(std::istringstream& input_string, AllInputs& inputs, int joy)
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

int InputSerialization::readFlagFrame(std::istringstream& input_string, AllInputs& inputs)
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

int InputSerialization::readFramerateFrame(std::istringstream& input_string, AllInputs& inputs)
{
    if (!inputs.misc)
        inputs.misc.reset(new MiscInputs{});
    char d;
    input_string >> std::dec;
    input_string >> inputs.misc->framerate_num >> d >> inputs.misc->framerate_den >> d;
    return 0;
}

int InputSerialization::readRealtimeFrame(std::istringstream& input_string, AllInputs& inputs)
{
    if (!inputs.misc)
        inputs.misc.reset(new MiscInputs{});
    char d;
    input_string >> std::dec;
    input_string >> inputs.misc->realtime_sec >> d >> inputs.misc->realtime_nsec >> d;
    return 0;
}
