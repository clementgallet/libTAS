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

#ifndef LINTAS_MOVIEFILE_H_INCLUDED
#define LINTAS_MOVIEFILE_H_INCLUDED

//#include <stdio.h>
//#include <unistd.h>
#include "../shared/AllInputs.h"
#include "Context.h"
#include <fstream>
#include <string>
#include <list>

class MovieFile {
public:
    /* Prepare a movie file from the context */
    void open(Context* c);

    /* Extract a moviefile and import the inputs into a list */
    void loadMovie();
    void loadMovie(std::string& moviefile);

    /* Write the inputs into a file and compress to the whole moviefile */
    void saveMovie();

    /* Get the number of frames from a moviefile (needs to load it) */
    int nbFrames(const Context& c, const std::string& moviefile);

    /* Set inputs in the current frame */
    int setInputs(const AllInputs& inputs);

    /* Load inputs from the current frame */
    int getInputs(AllInputs& inputs);

    /* Save and close the moviefile */
    void close();

private:
    /* Write a single frame of inputs into the input stream */
    int writeFrame(std::ofstream& input_stream, const AllInputs& inputs);

    /* Read a single frame of inputs from the line of inputs */
    int readFrame(std::string& line, AllInputs& inputs);

    Context* context;

    /* The list of inputs */
    std::list<AllInputs> input_list;

    /* An iterator to the current frame in the input list */
    std::list<AllInputs>::iterator input_it;

    /* The index of the iterator */
    unsigned int it_index;
};

#endif
