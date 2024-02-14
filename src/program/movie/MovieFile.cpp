/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "../shared/inputs/AllInputs.h"
#include "Context.h"

#include <sstream>
#include <iostream>
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_CREAT
#include <errno.h>
#include <unistd.h>

MovieFile::MovieFile(Context* c) : context(c)
{
    header = new MovieFileHeader(c);
    inputs = new MovieFileInputs(c);
    annotations = new MovieFileAnnotations(c);
    editor = new MovieFileEditor(c);
}

const char* MovieFile::errorString(int error_code) {
    static std::string err;

    switch (error_code) {
        case ENOMOVIE:
            return "Could not find the movie file";
        case EBADARCHIVE:
            err = "The movie file could not be extracted or build: ";
            err += strerror(errno);
            return err.c_str();
        case ENOINPUTS:
            return "The movie file does not contain the inputs file";
        case ENOCONFIG:
            return "The movie file does not contain the config file";
        default:
            return "Unknown error";
    }
}

void MovieFile::clear()
{
    header->clear();
    inputs->clear();
    annotations->clear();
    editor->clear();
}

int MovieFile::extractMovie(const std::string& moviefile)
{
    if (moviefile.empty())
        return ENOMOVIE;

    /* Check that the moviefile exists */
    if (access(moviefile.c_str(), F_OK) != 0)
        return ENOMOVIE;

    /* Empty the temp directory */
    std::string configfile = context->config.tempmoviedir + "/config.ini";
    std::string editorfile = context->config.tempmoviedir + "/editor.ini";
    std::string inputfile = context->config.tempmoviedir + "/inputs";
    std::string annotationsfile = context->config.tempmoviedir + "/annotations.txt";
    unlink(configfile.c_str());
    unlink(editorfile.c_str());
    unlink(inputfile.c_str());
    unlink(annotationsfile.c_str());

    /* Build the tar command */
    std::ostringstream oss;
    /* Piping gzip -> tar to avoid gzip warnings on old movie files */
    oss << "gzip -dq < \"";
    oss << moviefile;
    oss << "\" | tar -xUf - -C ";
    oss << context->config.tempmoviedir;

    /* Execute the tar command */
    // std::cout << oss.str() << std::endl;
    int ret = system(oss.str().c_str());

    if (!WIFEXITED(ret) || (WEXITSTATUS(ret) != 0))
        return EBADARCHIVE;

    /* Check the presence of the inputs and config files */
    if (access(configfile.c_str(), F_OK) != 0)
        return ENOCONFIG;
    if (access(inputfile.c_str(), F_OK) != 0)
        return ENOINPUTS;

    return 0;
}

int MovieFile::extractMovie()
{
    return extractMovie(context->config.moviefile);
}

int MovieFile::loadMovie(const std::string& moviefile)
{
    /* Extract the moviefile in the temp directory */
    int ret = extractMovie(moviefile);
    if (ret < 0)
        return ret;

    /* Load the config file into the context struct */
    header->load();
    inputs->load();
    annotations->load();
    editor->load();

    /* Copy framerate values to inputs */
    inputs->framerate_num = header->framerate_num;
    inputs->framerate_den = header->framerate_den;

    if (context->config.sc.movie_framecount != inputs->nbFrames()) {
        std::cerr << "Warning: movie framecount and movie config mismatch!" << std::endl;
        context->config.sc.movie_framecount = inputs->nbFrames();
    }

    return 0;
}

int MovieFile::loadMovie()
{
    return loadMovie(context->config.moviefile);
}

int MovieFile::loadSavestateMovie(const std::string& moviefile)
{
    /* Extract the moviefile in the temp directory */
    int ret = extractMovie(moviefile);
    if (ret < 0)
        return ret;

    inputs->load();
    editor->load();
    header->loadSavestate();

    return 0;
}

int MovieFile::saveMovie(const std::string& moviefile, uint64_t nb_frames)
{
    /* Skip empty moviefiles, if user tested the annotations without specifying a movie */
    if (moviefile.empty())
        return ENOMOVIE;

    inputs->save();
    header->save(inputs->nbFrames(), nb_frames);
    annotations->save();
    editor->save();

    /* Build the tar command */
    std::ostringstream oss;
    oss << "tar -czUf \"";
    oss << moviefile;
    oss << "\" -C ";
    oss << context->config.tempmoviedir;
    oss << " inputs config.ini editor.ini annotations.txt";

    /* Execute the tar command */
    // std::cout << oss.str() << std::endl;
    int ret = system(oss.str().c_str());

    if (!WIFEXITED(ret) || (WEXITSTATUS(ret) != 0))
        return EBADARCHIVE;

    return 0;
}

int MovieFile::saveMovie(const std::string& moviefile)
{
    return saveMovie(moviefile, inputs->nbFrames());
}

int MovieFile::saveMovie()
{
    inputs->modifiedSinceLastSave = false;
    return saveMovie(context->config.moviefile);
}

int MovieFile::saveBackupMovie()
{
    std::string backupfile = context->config.tempmoviedir + "/backup.ltm";
    std::cout << "No movie recording was selected. A backup movie was saved to " << backupfile << std::endl;
    return saveMovie(backupfile);
}

void MovieFile::copyTo(MovieFile& movie) const
{
    /* This will only be used for savestate movies, we only care to copy relevant data */
    movie.editor->input_set = editor->input_set;
    movie.editor->nondraw_frames = editor->nondraw_frames;
    movie.header->framerate_num = header->framerate_num;
    movie.header->framerate_den = header->framerate_den;
    movie.header->savestate_framecount = context->framecount;
    inputs->copyTo(movie.inputs);
}

void MovieFile::setLockedInputs(AllInputs& inp)
{
    const AllInputs& movie_inputs = inputs->getInputs();
    editor->setLockedInputs(inp, movie_inputs);
}

bool MovieFile::isPrefix(const MovieFile& movie) const
{
    return inputs->isPrefix(movie.inputs, movie.header->savestate_framecount);
}

void MovieFile::close()
{
    inputs->close();
    editor->close();
}

void MovieFile::updateLength()
{
    if (context->config.sc.movie_framecount != inputs->nbFrames()) {
        context->config.sc.movie_framecount = inputs->nbFrames();

        /* Unvalidate movie length when variable framerate */
        if (context->config.sc.variable_framerate) {
            header->length_sec = -1;
            header->length_nsec = -1;
        }
        else {
            /* Compute movie length from framecount */
            header->length_sec = (uint64_t)(context->config.sc.movie_framecount) * context->config.sc.initial_framerate_den / context->config.sc.initial_framerate_num;
            header->length_nsec = ((1000000000ull * (uint64_t)context->config.sc.movie_framecount * context->config.sc.initial_framerate_den) / context->config.sc.initial_framerate_num) % 1000000000ull;
        }

        context->config.sc_modified = true;
    }
}
