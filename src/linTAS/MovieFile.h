/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include <vector>

class MovieFile {
public:
    /* List of error codes */
    enum Error {
        ENOMOVIE = -1, // No movie file at the specified path
        EBADARCHIVE = -2, // Could not extract movie file
        ENOINPUTS = -3, // Movie file does not contain the input file
        ENOCONFIG = -4, // Movie file does not contain the config file
    };

    /* Error string associated with an error code */
    static const char* errorString(int error_code);

    /* The list of inputs. We need this to be public because a movie may
     * check if another movie is a prefix
     */
    std::vector<AllInputs> input_list;

    /* Flag storing if the movie has been modified since last save.
     * Used for prompting a message when the game exits if the user wants
     * to save.
     */
    bool modifiedSinceLastSave;

    /* Flag storing if the movie has been modified since last autosave. */
    bool modifiedSinceLastAutoSave;

    /* Annotations to be saved inside the movie file */
    std::string annotations;

    MovieFile() {};

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

    /* Import the inputs only. Used when loading movies attached to savestates.
     * Returns 0 if no error, or a negative value if an error occured */
    int loadInputs(const std::string& moviefile);

    /* Write the inputs into a file and compress to the whole moviefile */
    int saveMovie();
    int saveMovie(const std::string& moviefile);

    /* Write only the n first frames of input into the movie file. Used for savestate movies */
    int saveMovie(const std::string& moviefile, unsigned long frame_nb);

    /* Get the number of frames of the current movie */
    unsigned long nbFrames();

    /* Get the frame count of the associated savestate if any */
    unsigned long savestateFramecount() const;

    /* Set inputs for a certain frame, and truncate if keep_inputs is false */
    int setInputs(const AllInputs& inputs, unsigned long pos, bool keep_inputs);

    /* Set inputs in the current frame, and truncate if keep_inputs is false */
    int setInputs(const AllInputs& inputs, bool keep_inputs);

    /* Load inputs from a certain frame */
    int getInputs(AllInputs& inputs, unsigned long pos) const;

    /* Load inputs from the current frame */
    int getInputs(AllInputs& inputs) const;

    /* Insert inputs before the requested pos */
    void insertInputsBefore(const AllInputs& inputs, unsigned long pos);

    /* Delete inputs at the requested pos */
    void deleteInputs(unsigned long pos);

    /* Truncate inputs to a frame number */
    void truncateInputs(unsigned long size);

    /* Helper function called when the movie has been modified */
    void wasModified();

    /* Close the moviefile */
    void close();

    /* Check if another movie starts with the same inputs as this movie */
    bool isPrefix(const MovieFile& movie);

    /* Write a single frame of inputs into the input stream */
    int writeFrame(std::ostream& input_stream, const AllInputs& inputs);

    /* Read a single frame of inputs from the line of inputs */
    int readFrame(std::string& line, AllInputs& inputs);

private:
    Context* context;

};

#endif
