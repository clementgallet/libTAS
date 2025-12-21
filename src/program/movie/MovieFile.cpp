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
    changelog = new MovieFileChangeLog(c);
    
    inputs->setChangeLog(changelog);
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
    changelog->clear();
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

    /* Load editor before inputs, because `editor->load()` imports the input
     * editor columns and inputs->load() imports actual inputs, then it adds
     * more columns for inputs inside the movie that don't have a column yet.
     * Then it resets the input editor view */
    editor->load();
    header->load();
    inputs->load();
    annotations->load();

    /* Copy framerate values to inputs */
    inputs->setFramerate(header->framerate_num, header->framerate_den, header->variable_framerate);
    inputs->length_sec = header->length_sec;
    inputs->length_nsec = header->length_nsec;

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
    inputs->length_sec = header->length_sec;
    inputs->length_nsec = header->length_nsec;

    return 0;
}

int MovieFile::saveMovie(const std::string& moviefile, uint64_t nb_frames)
{
    /* Skip empty moviefiles, if user tested the annotations without specifying a movie */
    if (moviefile.empty())
        return ENOMOVIE;

    inputs->save();
    header->variable_framerate = inputs->variable_framerate;
    header->length_sec = inputs->length_sec;
    header->length_nsec = inputs->length_nsec;
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

    std::cout << "Movie saved" << std::endl;

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

void MovieFile::copyFrom(const MovieFile& movie)
{
    /* This will only be used for savestate movies, we only care to copy relevant data */
    editor->input_set = movie.editor->input_set;
    editor->nondraw_frames = movie.editor->nondraw_frames;
    header->framerate_num = movie.header->framerate_num;
    header->framerate_den = movie.header->framerate_den;
    header->savestate_framecount = context->framecount;
    inputs->copyFrom(movie.inputs);
}

void MovieFile::setLockedInputs(AllInputs& inp)
{
    const AllInputs& movie_inputs = inputs->getInputs();
    editor->setLockedInputs(inp, movie_inputs);
}

bool MovieFile::isPrefix(const MovieFile& movie) const
{
    return inputs->isEqual(movie.inputs, 0, movie.header->savestate_framecount);
}

bool MovieFile::isEqual(const MovieFile& movie, unsigned int first_frame, unsigned int last_frame) const
{
    return inputs->isEqual(movie.inputs, first_frame, last_frame);
}

void MovieFile::close()
{
    inputs->close();
    editor->close();
}

void MovieFile::applyAutoHoldFire()
{
    for (size_t i = 0; i < editor->autohold.size(); i++) {
        if (editor->autohold[i] > 0) {
            SingleInput si = editor->input_set[i];
            int value = 1;
            
            /* When autohold an analog value, we take the previous value */
            if (si.isAnalog() && (context->framecount > 0)) {
                const AllInputs& old_ai = inputs->getInputs(context->framecount - 1);
                value = old_ai.getInput(si);
            }

            if (editor->autohold[i] >= 2) // Auto-fire
                value = (context->framecount % 2) == (editor->autohold[i] % 2);

            inputs->paintInput(si, value, context->framecount, context->framecount);
        }
    }
    inputs->processPendingActions();
}
