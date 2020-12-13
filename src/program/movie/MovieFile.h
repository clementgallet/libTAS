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

#ifndef LIBTAS_MOVIEFILE_H_INCLUDED
#define LIBTAS_MOVIEFILE_H_INCLUDED

#include "../../shared/AllInputs.h"
#include "../Context.h"
#include "MovieFileAnnotations.h"
#include "MovieFileEditor.h"
#include "MovieFileHeader.h"
#include "MovieFileInputs.h"

#include <string>

class MovieFile {
public:
    
    MovieFileHeader* header;
    MovieFileInputs* inputs;
    MovieFileAnnotations* annotations;
    MovieFileEditor* editor;

    /* List of error codes */
    enum Error {
        ENOMOVIE = -1, // No movie file at the specified path
        EBADARCHIVE = -2, // Could not extract movie file
        ENOINPUTS = -3, // Movie file does not contain the input file
        ENOCONFIG = -4, // Movie file does not contain the config file
    };

    /* Error string associated with an error code */
    static const char* errorString(int error_code);

    /* Prepare a movie file from the context */
    MovieFile(Context* c);

    /* Extract a moviefile into the temp directory
     * Returns 0 if no error, or a negative value if an error occured */
    int extractMovie();
    int extractMovie(const std::string& moviefile);

    /* Import the inputs into a list, and all the parameters.
     * Returns 0 if no error, or a negative value if an error occured */
    int loadMovie();
    int loadMovie(const std::string& moviefile);

    /* Import only relevent data used for savestate movies.
     * Returns 0 if no error, or a negative value if an error occured */
    int loadSavestateMovie(const std::string& moviefile);

    /* Write the inputs into a file and compress to the whole moviefile */
    int saveMovie();
    int saveMovie(const std::string& moviefile);

    /* Write only the n first frames of input into the movie file. Used for savestate movies */
    int saveMovie(const std::string& moviefile, uint64_t frame_nb);

    /* Copy movie to another one */
    void copyTo(MovieFile& movie) const;

    /* Copy locked inputs from the current inputs to the inputs in argument */
    void setLockedInputs(AllInputs& inputs);

    bool isPrefix(const MovieFile& movie) const;

    /* Close the moviefile */
    void close();

private:
    Context* context;    

};

#endif
