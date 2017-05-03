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
#include "utils.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <libtar.h>
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_CREAT
#include <zlib.h>

void MovieFile::open(Context* c)
{
    context = c;

    movie_dir = getenv("HOME");
    movie_dir += "/.libtas";
    if (create_dir(movie_dir))
        return;
    movie_dir += "/movie";
    if (create_dir(movie_dir))
        return;

    switch(context->recording) {
        case Context::RECORDING_WRITE:
            writeHeader();
            input_list.clear();
            break;
        case Context::RECORDING_READ_WRITE:
        case Context::RECORDING_READ_ONLY:
            importMovie();
            readHeader();
            break;
    }
}

tartype_t gztype = { (openfunc_t) gzopen_wrapper, (closefunc_t) gzclose_wrapper,
	(readfunc_t) gzread_wrapper, (writefunc_t) gzwrite_wrapper};

void MovieFile::importMovie(std::string& moviefile)
{
    TAR *tar;
    tar_open(&tar, moviefile.c_str(), &gztype, O_RDONLY, 0644, 0);
    char* md = const_cast<char*>(movie_dir.c_str());
    tar_extract_all(tar, md);
    tar_close(tar);

    std::string input_file = movie_dir + "/inputs";
    std::ifstream input_stream(input_file);
    std::string line;

    input_list.clear();

    int f = 0;
    while (std::getline(input_stream, line)) {
        if (!line.empty() && (line[0] == '|')) {
            AllInputs ai;
            readFrame(line, ai);
            input_list.push_back(ai);
        }
    }

    input_it = input_list.begin();
    input_stream.close();
}

void MovieFile::importMovie()
{
    importMovie(context->config.moviefile);
}

void MovieFile::exportMovie()
{
    std::string input_file = movie_dir + "/inputs";
    std::ofstream input_stream(input_file, std::ofstream::trunc);

    for (auto const& ai : input_list) {
        writeFrame(input_stream, ai);
    }
    input_stream.close();

    TAR *tar;
    tar_open(&tar, context->config.moviefile.c_str(), &gztype, O_WRONLY | O_CREAT, 0644, 0);
    char* md = const_cast<char*>(movie_dir.c_str());
    /* I would like to use tar_append_tree but it saves files with their path */
    //tar_append_tree(tar, md, save_dir);
    char* input_ptr = const_cast<char*>(input_file.c_str());
    char savename[7] = "inputs";
    tar_append_file(tar, input_ptr, savename);

    tar_append_eof(tar);
    tar_close(tar);
}


void MovieFile::writeHeader()
{
}

void MovieFile::readHeader()
{
}

int MovieFile::writeFrame(std::ofstream& input_stream, const AllInputs& inputs)
{
    /* Write keyboard inputs */
    if (context->config.sc.keyboard_support) {
        input_stream.put('|');
        input_stream << std::hex;
        for (int k=0; k<AllInputs::MAXKEYS; k++) {
            if (inputs.keyboard[k] == XK_VoidSymbol) break;
            input_stream << (k>0?":":"") << inputs.keyboard[k];
        }
    }

    /* Write mouse inputs */
    if (context->config.sc.mouse_support) {
        input_stream.put('|');
        input_stream << std::hex;
        input_stream << inputs.pointer_x << ':' << inputs.pointer_y << ':';
        input_stream.put((inputs.pointer_mask&Button1Mask)?'1':'.');
        input_stream.put((inputs.pointer_mask&Button2Mask)?'2':'.');
        input_stream.put((inputs.pointer_mask&Button3Mask)?'3':'.');
        input_stream.put((inputs.pointer_mask&Button4Mask)?'4':'.');
        input_stream.put((inputs.pointer_mask&Button5Mask)?'5':'.');
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.numControllers; joy++) {
        input_stream.put('|');
        input_stream << std::hex;
        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            input_stream << inputs.controller_axes[joy][axis] << ':';
        }
        input_stream.put((inputs.controller_buttons[joy]&(1<<0))?'A':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<1))?'B':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<2))?'X':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<3))?'Y':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<4))?'b':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<5))?'g':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<6))?'s':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<7))?'(':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<8))?')':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<9))?'[':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<10))?']':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<11))?'u':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<12))?'d':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<13))?'l':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<14))?'r':'.');
    }

    input_stream << '|' << std::endl;

    return 1;
}

int MovieFile::readFrame(std::string& line, AllInputs& inputs)
{
    inputs.emptyInputs();

    std::istringstream input_string(line);
    char d;

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

int MovieFile::nbFrames(std::string& moviefile)
{
    /* Check if movie exists, otherwise return 0 */
    struct stat sb;
    if (stat(moviefile.c_str(), &sb) == -1)
        return 0;

    /* Extract file into our temp directory */
    movie_dir = getenv("HOME");
    movie_dir += "/.libtas";
    if (create_dir(movie_dir))
        return 0;
    movie_dir += "/movie";
    if (create_dir(movie_dir))
        return 0;

    importMovie(moviefile);

    return input_list.size();
}

void MovieFile::setFrame(const AllInputs& inputs)
{
    input_list.push_back(inputs);
}

int MovieFile::getFrame(AllInputs& inputs)
{
    if (input_it == input_list.end())
        return 0;
    inputs = *input_it;
    input_it++;
    return 1;
}

void MovieFile::truncate()
{
}

void MovieFile::close()
{
    exportMovie();
}
