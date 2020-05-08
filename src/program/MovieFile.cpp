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

MovieFile::MovieFile(Context* c) : modifiedSinceLastSave(false), modifiedSinceLastAutoSave(false), modifiedSinceLastStateLoad(false), context(c)
{
	rek.assign(R"(\|K([0-9a-f]*(?::[0-9a-f]+)*)\|)", std::regex::ECMAScript|std::regex::optimize);
	rem.assign(R"(\|M([\-0-9]+:[\-0-9]+:(?:[AR]:)?[\.1-5]{5})\|)", std::regex::ECMAScript|std::regex::optimize);
	rec.assign(R"(\|C((?:[\-0-9]+:){6}.{15})\|)", std::regex::ECMAScript|std::regex::optimize);
	ref.assign(R"(\|F(.{1,9})\|)", std::regex::ECMAScript|std::regex::optimize);
	ret.assign(R"(\|T([0-9]+:[0-9]+)\|)", std::regex::ECMAScript|std::regex::optimize);
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
	context->config.sc.initial_time_sec = config.value("initial_time_sec").toULongLong();
	context->config.sc.initial_time_nsec = config.value("initial_time_nsec").toULongLong();
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
	context->config.auto_restart = config.value("auto_restart").toBool();
	context->config.sc.variable_framerate = config.value("variable_framerate").toBool();

	config.beginGroup("mainthread_timetrack");
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME] = config.value("time").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY] = config.value("gettimeofday").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK] = config.value("clock").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME] = config.value("clock_gettime").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS] = config.value("sdl_getticks").toInt();
	context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER] = config.value("sdl_getperformancecounter").toInt();
	config.endGroup();

	int size = config.beginReadArray("input_names");
    if (size > 0)
        input_names.clear();
    for (int i = 0; i < size; ++i) {
        config.setArrayIndex(i);
        SingleInput si = config.value("input").value<SingleInput>();
        std::string name = config.value("name").toString().toStdString();
        input_names[si] = name;
    }
    config.endArray();

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

int MovieFile::saveMovie(const std::string& moviefile, uint64_t nb_frames)
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
	config.setValue("initial_time_sec", static_cast<unsigned long long>(context->config.sc.initial_time_sec));
	config.setValue("initial_time_nsec", static_cast<unsigned long long>(context->config.sc.initial_time_nsec));
	config.setValue("framerate_num", context->config.sc.framerate_num);
	config.setValue("framerate_den", context->config.sc.framerate_den);
	config.setValue("rerecord_count", context->rerecord_count);
	config.setValue("authors", context->authors.c_str());
	config.setValue("libtas_major_version", MAJORVERSION);
	config.setValue("libtas_minor_version", MINORVERSION);
	config.setValue("libtas_patch_version", PATCHVERSION);
	config.setValue("savestate_frame_count", static_cast<unsigned long long>(nb_frames));
	config.setValue("auto_restart", context->config.auto_restart);
	config.setValue("variable_framerate", context->config.sc.variable_framerate);

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

	config.remove("input_names");
    config.beginWriteArray("input_names");
    int i = 0;
    for (auto& in : input_names) {
        config.setArrayIndex(i++);
        config.setValue("input", QVariant::fromValue(in.first));
        config.setValue("name", in.second.c_str());
    }
    config.endArray();


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
		input_stream.put('K');
        input_stream << std::hex;
        for (int k=0; k<AllInputs::MAXKEYS; k++) {
            if (!inputs.keyboard[k]) break;
            input_stream << (k>0?":":"") << inputs.keyboard[k];
        }
    }

    /* Write mouse inputs */
    if (context->config.sc.mouse_support) {
        input_stream.put('|');
		input_stream.put('M');
        input_stream << std::dec;
        input_stream << inputs.pointer_x << ':' << inputs.pointer_y << ':';
		input_stream << ((inputs.pointer_mode == SingleInput::POINTER_MODE_RELATIVE)?"R:":"A:");
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B1))?'1':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B2))?'2':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B3))?'3':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B4))?'4':'.');
        input_stream.put((inputs.pointer_mask&(1<<SingleInput::POINTER_B5))?'5':'.');
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
		if (inputs.isDefaultController(joy))
			continue;
        input_stream.put('|');
		input_stream.put('C');
		input_stream.put('1'+joy);
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

	/* Write flag inputs */
	if (inputs.flags) {
		input_stream << '|';
		input_stream << 'F';
		if (inputs.flags & (1 << SingleInput::FLAG_RESTART)) input_stream.put('R');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER1_ADDED)) input_stream.put('1');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER2_ADDED)) input_stream.put('2');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER3_ADDED)) input_stream.put('3');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER4_ADDED)) input_stream.put('4');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER1_REMOVED)) input_stream.put('I');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER2_REMOVED)) input_stream.put('L');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER3_REMOVED)) input_stream.put('U');
		if (inputs.flags & (1 << SingleInput::FLAG_CONTROLLER4_REMOVED)) input_stream.put('O');
	}

	/* Write mouse inputs */
    if (context->config.sc.variable_framerate) {
        input_stream.put('|');
		input_stream.put('T');
        input_stream << std::dec;
        input_stream << inputs.framerate_num << ':' << inputs.framerate_den;
    }
	input_stream << '|' << std::endl;

    return 1;
}

int MovieFile::readFrame(const std::string& line, AllInputs& inputs)
{
    inputs.emptyInputs();

    std::istringstream input_string(line);
    char d;
	input_string >> d;

	/* Read keyboard inputs using regex first. If matched, read the rest of the
	 * inputs with regex as well. */

	std::smatch match;
    if (std::regex_search(line, match, rek) && match.size() > 1) {
		std::istringstream key_string(match.str(1));
		readKeyboardFrame(key_string, inputs);

		/* Read mouse inputs */
		if (std::regex_search(line, match, rem) && match.size() > 1) {
			std::istringstream mouse_string(match.str(1));
			readMouseFrame(mouse_string, inputs);
	    }

		/* Read controller inputs */
		std::sregex_iterator next(line.begin(), line.end(), rec);
		std::sregex_iterator end;
		if (next != end) {
			while (next != end) {
				std::smatch match = *next;
				std::istringstream controller_string(match.str(1));

				/* Extract joystick number */
				char j;
				controller_string >> j;

				/* Read joystick inputs */
				readControllerFrame(controller_string, inputs, j - '1');
				next++;
			}
		}

		/* Read flag inputs */
		if (std::regex_search(line, match, ref) && match.size() > 1) {
			std::istringstream flag_string(match.str(1));
			readFlagFrame(flag_string, inputs);
		}

		/* Read framerate inputs */
		if (std::regex_search(line, match, ret) && match.size() > 1) {
			std::istringstream framerate_string(match.str(1));
			readFramerateFrame(framerate_string, inputs);
		}

		return 1;
    }

	/* Following code is for old input format (1.3.5 and earlier) */

	/* Read keyboard inputs */
	if (context->config.sc.keyboard_support) {
		readKeyboardFrame(input_string, inputs);
    }

	/* Read mouse inputs */
    if (context->config.sc.mouse_support) {
		readMouseFrame(input_string, inputs);
    }

    /* Read controller inputs */
	for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
		readControllerFrame(input_string, inputs, joy);
    }

	/* Read flag inputs */
	readFlagFrame(input_string, inputs);

	/* Read framerate inputs */
	if (context->config.sc.variable_framerate) {
		readFramerateFrame(input_string, inputs);
	}

    return 1;
}

void MovieFile::readKeyboardFrame(std::istringstream& input_string, AllInputs& inputs)
{
	input_string >> std::hex;
	char d = input_string.peek();
	if (d == '|') {
		input_string.get();
		return;
	}
	for (int k=0; (k<AllInputs::MAXKEYS) && input_string; k++) {
		input_string >> inputs.keyboard[k] >> d;
		if (d == '|') {
			break;
		}
	}
}

void MovieFile::readMouseFrame(std::istringstream& input_string, AllInputs& inputs)
{
	char d;
	input_string >> std::dec;
	input_string >> inputs.pointer_x >> d >> inputs.pointer_y >> d;
	input_string >> d;
	if ((d == 'R') || (d == 'A')) {
		/* Read mouse mode */
		if (d == 'R') inputs.pointer_mode = SingleInput::POINTER_MODE_RELATIVE;
		else inputs.pointer_mode = SingleInput::POINTER_MODE_ABSOLUTE;
		input_string >> d;
		input_string >> d;
	}
	else {
		inputs.pointer_mode = SingleInput::POINTER_MODE_ABSOLUTE;
	}
	if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B1);
	input_string >> d;
	if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B2);
	input_string >> d;
	if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B3);
	input_string >> d;
	if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B4);
	input_string >> d;
	if (d != '.') inputs.pointer_mask |= (1 << SingleInput::POINTER_B5);
	input_string >> d;
}

void MovieFile::readControllerFrame(std::istringstream& input_string, AllInputs& inputs, int joy)
{
	char d;
	input_string >> std::dec;
	for (int axis=0; axis<AllInputs::MAXAXES; axis++) {
		input_string >> inputs.controller_axes[joy][axis] >> d;
	}
	for (int b=0; b<15; b++) {
		input_string >> d;
		if (d != '.') inputs.controller_buttons[joy] |= (1 << b);
	}
	input_string >> d;
}

void MovieFile::readFlagFrame(std::istringstream& input_string, AllInputs& inputs)
{
	char d;
	input_string >> d;
	while (input_string && (d != '|')) {
		switch (d) {
			case 'R': inputs.flags |= (1 << SingleInput::FLAG_RESTART); break;
			case '1': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER1_ADDED); break;
			case '2': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER2_ADDED); break;
			case '3': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER3_ADDED); break;
			case '4': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER4_ADDED); break;
			case 'I': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER1_REMOVED); break;
			case 'L': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER2_REMOVED); break;
			case 'U': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER3_REMOVED); break;
			case 'O': inputs.flags |= (1 << SingleInput::FLAG_CONTROLLER4_REMOVED); break;
		}
		input_string >> d;
	}
}

void MovieFile::readFramerateFrame(std::istringstream& input_string, AllInputs& inputs)
{
	char d;
	input_string >> std::dec;
	input_string >> inputs.framerate_num >> d >> inputs.framerate_den >> d;
}

uint64_t MovieFile::nbFrames()
{
	return input_list.size();
}

uint64_t MovieFile::savestateFramecount() const
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

int MovieFile::setInputs(const AllInputs& inputs, uint64_t pos, bool keep_inputs)
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

int MovieFile::getInputs(AllInputs& inputs, uint64_t pos) const
{
    if (pos >= input_list.size()) {
        inputs.emptyInputs();
        return -1;
    }

	inputs = input_list[pos];

    if ((pos + 1) == input_list.size()) {
        /* We are reading the last frame of the movie, notify the caller */
        return 1;
    }

    return 0;
}

void MovieFile::insertInputsBefore(const AllInputs& inputs, uint64_t pos)
{
	if (pos > input_list.size())
		return;

	input_list.insert(input_list.begin() + pos, inputs);
	wasModified();
}

void MovieFile::deleteInputs(uint64_t pos)
{
	if (pos >= input_list.size())
		return;

	input_list.erase(input_list.begin() + pos);
	wasModified();
}

void MovieFile::truncateInputs(uint64_t size)
{
	input_list.resize(size);
	wasModified();
}

void MovieFile::setLockedInputs(AllInputs& inputs)
{
	if (locked_inputs.empty())
		return;

	AllInputs movie_inputs;
	getInputs(movie_inputs);
	for (SingleInput si : locked_inputs) {
		int value = movie_inputs.getInput(si);
		inputs.setInput(si, value);
	}
}

void MovieFile::close()
{
	input_list.clear();
	locked_inputs.clear();
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
