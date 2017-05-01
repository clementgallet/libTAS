/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "MovieFile.h"
#include <sstream>
#include <iomanip>
#include <iostream>

void MovieFile::open(const char* filename, Context* c)
{
    context = c;
    switch(context->recording) {
        case Context::RECORDING_WRITE:
            movie_stream.open(filename, std::fstream::in | std::fstream::out | std::fstream::trunc);
            writeHeader();
            break;
        case Context::RECORDING_READ_WRITE:
            movie_stream.open(filename, std::fstream::in | std::fstream::out);
            readHeader();
            break;
        case Context::RECORDING_READ_ONLY:
            movie_stream.open(filename, std::fstream::in);
            readHeader();
            break;
    }
}

void MovieFile::writeHeader()
{
}

void MovieFile::readHeader()
{
}

int MovieFile::writeFrame(unsigned long frame, AllInputs inputs)
{
    /* Write keyboard inputs */
    if (context->config.sc.keyboard_support) {
        movie_stream.put('|');
        movie_stream << std::hex;
        for (int k=0; k<AllInputs::MAXKEYS; k++) {
            if (inputs.keyboard[k] == XK_VoidSymbol) break;
            movie_stream << (k>0?":":"") << inputs.keyboard[k];
        }
    }

    /* Write mouse inputs */
    if (context->config.sc.mouse_support) {
        movie_stream.put('|');
        movie_stream << std::hex;
        movie_stream << inputs.pointer_x << ':' << inputs.pointer_y << ':';
        movie_stream.put((inputs.pointer_mask&Button1Mask)?'1':'.');
        movie_stream.put((inputs.pointer_mask&Button2Mask)?'2':'.');
        movie_stream.put((inputs.pointer_mask&Button3Mask)?'3':'.');
        movie_stream.put((inputs.pointer_mask&Button4Mask)?'4':'.');
        movie_stream.put((inputs.pointer_mask&Button5Mask)?'5':'.');
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.numControllers; joy++) {
        movie_stream.put('|');
        movie_stream << std::hex;
        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            movie_stream << inputs.controller_axes[joy][axis] << ':';
        }
        movie_stream.put((inputs.controller_buttons[joy]&(1<<0))?'A':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<1))?'B':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<2))?'X':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<3))?'Y':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<4))?'b':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<5))?'g':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<6))?'s':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<7))?'(':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<8))?')':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<9))?'[':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<10))?']':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<11))?'u':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<12))?'d':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<13))?'l':'.');
        movie_stream.put((inputs.controller_buttons[joy]&(1<<14))?'r':'.');
    }

    movie_stream << '|' << std::endl;

    return 1;
}

int MovieFile::readFrame(unsigned long frame, AllInputs& inputs)
{
    inputs.emptyInputs();

    std::string line;
    std::getline(movie_stream, line);
    if (!movie_stream)
        return 0;
    std::istringstream input_string(line);
    char d;
    input_string.get(d);

    /* Find the first line starting with '|' */
    while (d != '|') {
        std::getline(movie_stream, line);
        if (!movie_stream)
            return 0;
        input_string.str(line);
        input_string.get(d);
    }

    input_string.unget();

    /* Read keyboard inputs */
    if (context->config.sc.keyboard_support) {
        input_string >> d;
        input_string >> std::hex;
        /* Check if there is no key pressed */
        input_string.get(d);
        if (d != '|') {
            input_string.unget();
            for (int k=0; k<AllInputs::MAXKEYS; k++) {
                input_string >> inputs.keyboard[k] >> d;
                if (d == '|') {
                    break;
                }
            }
        }
        input_string.unget();
    }

    /* Write mouse inputs in text */
    if (context->config.sc.mouse_support) {
        input_string >> d;
        input_string >> std::hex;
        input_string >> inputs.pointer_x >> d >> inputs.pointer_y >> d;
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= Button1Mask;
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= Button2Mask;
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= Button3Mask;
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= Button4Mask;
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= Button5Mask;
    }

    for (int joy=0; joy<context->config.sc.numControllers; joy++) {
        input_string >> d;
        input_string >> std::hex;
        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            input_string >> inputs.controller_axes[joy][axis] >> d;
        }
        for (int b=0; b<15; b++) {
            input_string >> d;
            if (d != '.') inputs.controller_buttons[joy] |= (1 << b);
        }
    }

    return 1;
}

void MovieFile::truncate()
{
}

void MovieFile::close()
{
    movie_stream.close();
}
