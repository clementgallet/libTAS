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

#include <QSettings>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_CREAT
#include <X11/X.h> // ButtonXMask
#include <errno.h>
#include <unistd.h>

#include "MovieFile.h"
#include "utils.h"
#include "../shared/version.h"

MovieFile::MovieFile(Context* c) : modifiedSinceLastSave(false), modifiedSinceLastAutoSave(false), modifiedSinceLastStateLoad(false), context(c) {}

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

int MovieFile::extractMovie(const std::string& moviefile)
{
	if (moviefile.empty())
		return ENOMOVIE;

	/* Check that the moviefile exists */
	if (access(moviefile.c_str(), F_OK) != 0)
		return ENOMOVIE;

	/* Empty the temp directory */
	std::string configfile = context->config.tempmoviedir + "/config.ini";
	std::string inputfile = context->config.tempmoviedir + "/inputs";
	std::string annotationsfile = context->config.tempmoviedir + "/annotations.txt";
	unlink(configfile.c_str());
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
	QString configfile = context->config.tempmoviedir.c_str();
	configfile += "/config.ini";

	QSettings config(configfile, QSettings::IniFormat);
	config.setFallbacksEnabled(false);

	context->config.sc.movie_framecount = config.value("frame_count").toULongLong();
	context->config.sc.keyboard_support = config.value("keyboard_support").toBool();
	context->config.sc.mouse_support = config.value("mouse_support").toBool();

	context->config.sc.nb_controllers = config.value("nb_controllers").toInt();
	context->config.sc.initial_time.tv_sec = config.value("initial_time_sec").toInt();
	context->config.sc.initial_time.tv_nsec = config.value("initial_time_nsec").toInt();
	context->config.sc.framerate_num = config.value("framerate_num").toUInt();
	context->config.sc.framerate_den = config.value("framerate_den").toUInt();
	/* Compatibility with older movie format */
	if (!context->config.sc.framerate_num) {
		context->config.sc.framerate_num = config.value("framerate").toUInt();
		context->config.sc.framerate_den = 1;
	}
	context->rerecord_count = config.value("rerecord_count").toUInt();
	context->authors = config.value("authors").toString().toStdString();
	context->md5_movie = config.value("md5").toString().toStdString();

	config.beginGroup("mainthread_timetrack");
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME] = config.value("time").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY] = config.value("gettimeofday").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK] = config.value("clock").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME] = config.value("clock_gettime").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS] = config.value("sdl_getticks").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER] = config.value("sdl_getperformancecounter").toInt();
	config.endGroup();

	config.beginGroup("secondarythread_timetrack");
	context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_TIME] = config.value("time").toInt();
	context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY] = config.value("gettimeofday").toInt();
	context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK] = config.value("clock").toInt();
	context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME] = config.value("clock_gettime").toInt();
	context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS] = config.value("sdl_getticks").toInt();
	context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER] = config.value("sdl_getperformancecounter").toInt();
	config.endGroup();

    /* Open the input file and parse each line to fill our input list */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ifstream input_stream(input_file);
    std::string line;

    input_list.clear();

    while (std::getline(input_stream, line)) {
        if (!line.empty() && (line[0] == '|')) {
            AllInputs ai;
            readFrame(line, ai);
            input_list.push_back(ai);
        }
    }

	if (context->config.sc.movie_framecount != input_list.size()) {
		std::cerr << "Warning: movie framecount and movie config mismatch!" << std::endl;
		context->config.sc.movie_framecount = input_list.size();
	}

    input_stream.close();

	/* Load annotations if available */
	std::string annotations_file = context->config.tempmoviedir + "/annotations.txt";
    std::ifstream annotations_stream(annotations_file);
	if (annotations_stream) {
		annotations = std::string((std::istreambuf_iterator<char>(annotations_stream)),
	                 std::istreambuf_iterator<char>());
	}
	else {
		annotations = "";
	}

	return 0;
}

int MovieFile::loadMovie()
{
    return loadMovie(context->config.moviefile);
}

int MovieFile::loadInputs(const std::string& moviefile)
{
	/* Extract the moviefile in the temp directory */
	int ret = extractMovie(moviefile);
	if (ret < 0)
		return ret;

    /* Open the input file and parse each line to fill our input list */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ifstream input_stream(input_file);
    std::string line;

    input_list.clear();

    while (std::getline(input_stream, line)) {
        if (!line.empty() && (line[0] == '|')) {
            AllInputs ai;
            readFrame(line, ai);
            input_list.push_back(ai);
        }
    }

    input_stream.close();
	return 0;
}

int MovieFile::saveMovie(const std::string& moviefile, unsigned long nb_frames)
{
	/* Skip empty moviefiles, if user tested the annotations without specifying a movie */
	if (moviefile.empty())
		return ENOMOVIE;

    /* Format and write input frames into the input file */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ofstream input_stream(input_file, std::ofstream::trunc);

    for (auto it = input_list.begin(); it != input_list.end(); ++it) {
        writeFrame(input_stream, *it);
    }
    input_stream.close();

    /* Save some parameters into the config file */
	QString configfile = context->config.tempmoviedir.c_str();
	configfile += "/config.ini";

	QSettings config(configfile, QSettings::IniFormat);
	config.setFallbacksEnabled(false);

	config.setValue("game_name", context->gamename.c_str());
	config.setValue("frame_count", static_cast<unsigned long long>(input_list.size()));
	config.setValue("keyboard_support", context->config.sc.keyboard_support);
	config.setValue("mouse_support", context->config.sc.mouse_support);
	config.setValue("nb_controllers", context->config.sc.nb_controllers);
	config.setValue("initial_time_sec", static_cast<int>(context->config.sc.initial_time.tv_sec));
	config.setValue("initial_time_nsec", static_cast<int>(context->config.sc.initial_time.tv_nsec));
	config.setValue("framerate_num", context->config.sc.framerate_num);
	config.setValue("framerate_den", context->config.sc.framerate_den);
	config.setValue("rerecord_count", context->rerecord_count);
	config.setValue("authors", context->authors.c_str());
	config.setValue("libtas_major_version", MAJORVERSION);
	config.setValue("libtas_minor_version", MINORVERSION);
	config.setValue("libtas_patch_version", PATCHVERSION);
	config.setValue("savestate_frame_count", static_cast<unsigned long long>(nb_frames));

	/* Store the md5 that was extracted from the movie, or store the game
	 * binary movie if not. */
	if (!context->md5_movie.empty())
		config.setValue("md5", context->md5_movie.c_str());
	else if (!context->md5_game.empty())
		config.setValue("md5", context->md5_game.c_str());

	config.beginGroup("mainthread_timetrack");
	config.setValue("time", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
	config.setValue("gettimeofday", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
	config.setValue("clock", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
	config.setValue("clock_gettime", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME]);
	config.setValue("sdl_getticks", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
	config.setValue("sdl_getperformancecounter", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);
	config.endGroup();

	config.beginGroup("secondarythread_timetrack");
	config.setValue("time", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
	config.setValue("gettimeofday", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
	config.setValue("clock", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
	config.setValue("clock_gettime", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME]);
	config.setValue("sdl_getticks", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
	config.setValue("sdl_getperformancecounter", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);
	config.endGroup();

    config.sync();

	/* Save annotations */
	std::string annotations_file = context->config.tempmoviedir + "/annotations.txt";
    std::ofstream annotations_stream(annotations_file);
	annotations_stream << annotations;
	annotations_stream.close();

	/* Build the tar command */
	std::ostringstream oss;
	oss << "tar -czUf \"";
	oss << moviefile;
	oss << "\" -C ";
	oss << context->config.tempmoviedir;
	oss << " inputs config.ini annotations.txt";

	/* Execute the tar command */
	// std::cout << oss.str() << std::endl;
	int ret = system(oss.str().c_str());

	if (!WIFEXITED(ret) || (WEXITSTATUS(ret) != 0))
		return EBADARCHIVE;

	return 0;
}

int MovieFile::saveMovie(const std::string& moviefile)
{
	return saveMovie(moviefile, input_list.size());
}

int MovieFile::saveMovie()
{
	modifiedSinceLastSave = false;
	return saveMovie(context->config.moviefile);
}

int MovieFile::writeFrame(std::ostream& input_stream, const AllInputs& inputs)
{
    /* Write keyboard inputs */
    if (context->config.sc.keyboard_support) {
        input_stream.put('|');
        input_stream << std::hex;
        for (int k=0; k<AllInputs::MAXKEYS; k++) {
            if (!inputs.keyboard[k]) break;
            input_stream << (k>0?":":"") << inputs.keyboard[k];
        }
    }

    /* Write mouse inputs */
    if (context->config.sc.mouse_support) {
        input_stream.put('|');
        input_stream << std::dec;
        input_stream << inputs.pointer_x << ':' << inputs.pointer_y << ':';
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B1))?'1':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B2))?'2':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B3))?'3':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B4))?'4':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B5))?'5':'.');
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
        input_stream.put('|');
        input_stream << std::dec;
        for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
            input_stream << inputs.controller_axes[joy][axis] << ':';
        }
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_A))?'A':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_B))?'B':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_X))?'X':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_Y))?'Y':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_BACK))?'b':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_GUIDE))?'g':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_START))?'s':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_LEFTSTICK))?'(':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_RIGHTSTICK))?')':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_LEFTSHOULDER))?'[':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_RIGHTSHOULDER))?']':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_UP))?'u':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_DOWN))?'d':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_LEFT))?'l':'.');
        input_stream.put((inputs.controller_buttons[joy]&(1<<SingleInput::BUTTON_DPAD_RIGHT))?'r':'.');
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

    /* Read mouse inputs */
    if (context->config.sc.mouse_support) {
        input_string >> d;
        input_string >> std::dec;
        input_string >> inputs.pointer_x >> d >> inputs.pointer_y >> d;
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B1);
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B2);
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B3);
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B4);
        input_string >> d;
        if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B5);
    }

    /* Read controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
        input_string >> d;
        input_string >> std::dec;
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

unsigned long MovieFile::nbFrames()
{
	return input_list.size();
}

unsigned long MovieFile::savestateFramecount() const
{
	/* Load the config file into the context struct */
	QString configfile = context->config.tempmoviedir.c_str();
	configfile += "/config.ini";

	QSettings config(configfile, QSettings::IniFormat);
	config.setFallbacksEnabled(false);

	return config.value("savestate_frame_count").toULongLong();
}

int MovieFile::setInputs(const AllInputs& inputs, bool keep_inputs)
{
	return setInputs(inputs, context->framecount, keep_inputs);
}

int MovieFile::setInputs(const AllInputs& inputs, unsigned long pos, bool keep_inputs)
{
    /* Check that we are writing to the next frame */
    if (pos == input_list.size()) {
        input_list.push_back(inputs);
		wasModified();
        return 0;
    }
    else if (pos < input_list.size()) {
        /* Writing to a frame that is before the last one. if keep_inputs is
		 * false, we resize the input list accordingly and append the frame at
		 * the end.
         */
		if (keep_inputs) {
			input_list[pos] = inputs;
		}
		else {
	        input_list.resize(pos);
	        input_list.push_back(inputs);
		}
		wasModified();
        return 0;
    }
    else {
        std::cerr << "Writing to a frame " << pos << " higher than the current list " << input_list.size() << std::endl;
        return 1;
    }
}

int MovieFile::getInputs(AllInputs& inputs) const
{
	return getInputs(inputs, context->framecount);
}

int MovieFile::getInputs(AllInputs& inputs, unsigned long pos) const
{
    if (pos >= input_list.size()) {
        inputs.emptyInputs();
        return 0;
    }

	inputs = input_list[pos];

    if ((pos + 1) == input_list.size()) {
        /* We are reading the last frame of the movie, notify the caller */
        return 1;
    }

    return 0;
}

void MovieFile::insertInputsBefore(const AllInputs& inputs, unsigned long pos)
{
	if (pos > input_list.size())
		return;

	input_list.insert(input_list.begin() + pos, inputs);
	wasModified();
}

void MovieFile::deleteInputs(unsigned long pos)
{
	if (pos >= input_list.size())
		return;

	input_list.erase(input_list.begin() + pos);
	wasModified();
}

void MovieFile::truncateInputs(unsigned long size)
{
	input_list.resize(size);
	wasModified();
}

void MovieFile::close()
{
	input_list.clear();
}

bool MovieFile::isPrefix(const MovieFile& movie, unsigned int frame)
{
    /* Not a prefix if the size is greater */
    if (frame > input_list.size())
        return false;

    return std::equal(movie.input_list.begin(), movie.input_list.begin() + frame, input_list.begin());
}

bool MovieFile::isPrefix(const MovieFile& movie)
{
	/* Recover the frame of the savestate */
	unsigned int fc = movie.savestateFramecount();
	return isPrefix(movie, fc);
}

void MovieFile::wasModified()
{
	modifiedSinceLastSave = true;
	modifiedSinceLastAutoSave = true;
	modifiedSinceLastStateLoad = true;
}
