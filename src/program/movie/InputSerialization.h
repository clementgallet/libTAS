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

#ifndef LIBTAS_INPUTSERIALIZATION_H_INCLUDED
#define LIBTAS_INPUTSERIALIZATION_H_INCLUDED

#include "../shared/inputs/AllInputs.h"

#include <fstream>
#include <string>
#include <vector>

struct Context;

namespace InputSerialization {

void setContext(Context* c);

/* Set framerate initial values */
void setFramerate(unsigned int num, unsigned int den);

/* Write a list of inputs into the stream */
void writeInputs(std::ostream& stream, const std::vector<AllInputs>& input_list);

/* Read a list of inputs from a stream */
void readInputs(std::istream& stream, std::vector<AllInputs>& input_list);

/* Write a single frame of inputs into the input stream */
int writeFrame(std::ostream& input_stream, const AllInputs& inputs);

/* Read a single frame of inputs from the line of inputs */
int readFrame(const std::string& line, AllInputs& inputs);

/* Read one event input string */
int readEventFrame(std::istringstream& input_string, AllInputs& inputs);

/* Read the keyboard input string */
int readKeyboardFrame(std::istringstream& input_string, AllInputs& inputs);

/* Read the mouse input string */
int readMouseFrame(std::istringstream& input_string, AllInputs& inputs);

/* Read one controller input string */
int readControllerFrame(std::istringstream& input_string, AllInputs& inputs, int joy);

/* Read the flag input string */
int readFlagFrame(std::istringstream& input_string, AllInputs& inputs);

/* Read the framerate input string */
int readFramerateFrame(std::istringstream& input_string, AllInputs& inputs);

/* Read the realtime input string */
int readRealtimeFrame(std::istringstream& input_string, AllInputs& inputs);

};

#endif
