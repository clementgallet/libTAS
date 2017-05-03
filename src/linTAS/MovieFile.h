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

    void open(Context* c);
    void importMovie();
    void importMovie(std::string& moviefile);
    void exportMovie();
    void writeHeader();
    void readHeader();
    int nbFrames(std::string& moviefile);
    int writeFrame(std::ofstream& input_stream, const AllInputs& inputs);
    int readFrame(std::string& line, AllInputs& inputs);
    void setFrame(const AllInputs& inputs);
    int getFrame(AllInputs& inputs);
    void truncate();
    void close();

private:
    Context* context;
    std::string movie_dir;
    std::list<AllInputs> input_list;
    std::list<AllInputs>::iterator input_it;
};

#endif
