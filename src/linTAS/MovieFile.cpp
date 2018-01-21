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
#include "../shared/version.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <libtar.h>
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_CREAT
#include <zlib.h>

static tartype_t gztype = { (openfunc_t) gzopen_wrapper, (closefunc_t) gzclose_wrapper,
	(readfunc_t) gzread_wrapper, (writefunc_t) gzwrite_wrapper};

MovieFile::MovieFile(Context* c) : modifiedSinceLastSave(false), context(c) {}

const char* MovieFile::errorString(int error_code) {
	switch (error_code) {
		case ENOMOVIE:
			return "Could not find the movie file";
		case EBADARCHIVE:
			return "The movie file could not be extracted";
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
	/* Check that the moviefile exists */
	if (access(moviefile.c_str(), F_OK) != 0)
		return ENOMOVIE;

	/* Empty the temp directory */
	std::string configfile = context->config.tempmoviedir + "/config.prefs";
	std::string inputfile = context->config.tempmoviedir + "/inputs";
	unlink(configfile.c_str());
	unlink(inputfile.c_str());

    /* Uncompress the movie file into out temp directory */
    TAR *tar;
    int ret = tar_open(&tar, moviefile.c_str(), &gztype, O_RDONLY, 0644, 0);
	if (ret == -1) return EBADARCHIVE;

    char* md = const_cast<char*>(context->config.tempmoviedir.c_str());
    ret = tar_extract_all(tar, md);
	if (ret == -1) return EBADARCHIVE;

    ret = tar_close(tar);
	if (ret == -1) return EBADARCHIVE;

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
    Fl_Preferences config_prefs(context->config.tempmoviedir.c_str(), "movie", "config");

	int val;

	#define GETWITHTYPE(prefs, key, member, type) \
		val = static_cast<int>(member); \
		prefs.get(key, val, val); \
		member = static_cast<type>(val)

	unsigned int movie_framecount = 0;
	GETWITHTYPE(config_prefs, "frame_count", movie_framecount, unsigned int);
	GETWITHTYPE(config_prefs, "keyboard_support", context->config.sc.keyboard_support, bool);
	GETWITHTYPE(config_prefs, "mouse_support", context->config.sc.mouse_support, bool);
	GETWITHTYPE(config_prefs, "nb_controllers", context->config.sc.nb_controllers, int);
	GETWITHTYPE(config_prefs, "initial_time_sec", context->config.sc.initial_time.tv_sec, time_t);
	GETWITHTYPE(config_prefs, "initial_time_nsec", context->config.sc.initial_time.tv_nsec, time_t);
	GETWITHTYPE(config_prefs, "framerate", context->config.sc.framerate, unsigned int);
	GETWITHTYPE(config_prefs, "rerecord_count", context->rerecord_count, unsigned int);

	/* Load the movie length and compute the movie end time using the initial time */
	struct timespec movie_length;
	GETWITHTYPE(config_prefs, "movie_length_sec", movie_length.tv_sec, time_t);
	GETWITHTYPE(config_prefs, "movie_length_nsec", movie_length.tv_nsec, time_t);

	context->movie_end_time.tv_sec = movie_length.tv_sec + context->config.sc.initial_time.tv_sec;
	context->movie_end_time.tv_nsec = movie_length.tv_nsec + context->config.sc.initial_time.tv_nsec;
	if (context->movie_end_time.tv_nsec >= 1000000000) {
		context->movie_end_time.tv_nsec -= 1000000000;
		context->movie_end_time.tv_sec++;
	}

	Fl_Preferences main_time_prefs(config_prefs, "mainthread_timetrack");

	main_time_prefs.get("time", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME], context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
	main_time_prefs.get("gettimeofday", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY], context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
	main_time_prefs.get("clock", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK], context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
	main_time_prefs.get("clock_gettime", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME], context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME]);
	main_time_prefs.get("sdl_getticks", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS], context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
	main_time_prefs.get("sdl_getperformancecounter", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER], context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);

	Fl_Preferences sec_time_prefs(config_prefs, "secondarythread_timetrack");

	sec_time_prefs.get("time", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_TIME], context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
	sec_time_prefs.get("gettimeofday", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY], context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
	sec_time_prefs.get("clock", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK], context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
	sec_time_prefs.get("clock_gettime", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME], context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME]);
	sec_time_prefs.get("sdl_getticks", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS], context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
	sec_time_prefs.get("sdl_getperformancecounter", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER], context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);

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

	if (movie_framecount != input_list.size()) {
		std::cerr << "Warning: movie framecount and movie config mismatch!" << std::endl;
		movie_framecount = input_list.size();
	}

    input_stream.close();
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

void MovieFile::saveMovie(const std::string& moviefile, unsigned int nb_frames)
{
    /* Format and write input frames into the input file */
    std::string input_file = context->config.tempmoviedir + "/inputs";
    std::ofstream input_stream(input_file, std::ofstream::trunc);

    for (auto it = input_list.begin(); it != input_list.begin() + nb_frames; ++it) {
        writeFrame(input_stream, *it);
    }
    input_stream.close();

    /* Save some parameters into the config file */
    Fl_Preferences config_prefs(context->config.tempmoviedir.c_str(), "movie", "config");
	config_prefs.set("game_name", context->gamename.c_str());
    config_prefs.set("frame_count", static_cast<int>(nb_frames));
    config_prefs.set("keyboard_support", static_cast<int>(context->config.sc.keyboard_support));
    config_prefs.set("mouse_support", static_cast<int>(context->config.sc.mouse_support));
    config_prefs.set("nb_controllers", context->config.sc.nb_controllers);
	config_prefs.set("initial_time_sec", static_cast<int>(context->config.sc.initial_time.tv_sec));
	config_prefs.set("initial_time_nsec", static_cast<int>(context->config.sc.initial_time.tv_nsec));
	config_prefs.set("framerate", static_cast<int>(context->config.sc.framerate));
	config_prefs.set("rerecord_count", static_cast<int>(context->rerecord_count));
	config_prefs.set("libtas_major_version", MAJORVERSION);
	config_prefs.set("libtas_minor_version", MINORVERSION);
	config_prefs.set("libtas_patch_version", PATCHVERSION);

	/* Compute and save movie length */
	time_t movie_length_sec = context->movie_end_time.tv_sec - context->config.sc.initial_time.tv_sec;
	time_t movie_length_nsec = context->movie_end_time.tv_nsec - context->config.sc.initial_time.tv_nsec;
	if (movie_length_nsec < 0) {
		movie_length_nsec += 1000000000;
		movie_length_sec--;
	}
	config_prefs.set("movie_length_sec", static_cast<int>(movie_length_sec));
	config_prefs.set("movie_length_nsec", static_cast<int>(movie_length_nsec));

	Fl_Preferences main_time_prefs(config_prefs, "mainthread_timetrack");

	main_time_prefs.set("time", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
	main_time_prefs.set("gettimeofday", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
	main_time_prefs.set("clock", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
	main_time_prefs.set("clock_gettime", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME]);
	main_time_prefs.set("sdl_getticks", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
	main_time_prefs.set("sdl_getperformancecounter", context->config.sc.main_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);

	Fl_Preferences sec_time_prefs(config_prefs, "secondarythread_timetrack");

	sec_time_prefs.set("time", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_TIME]);
	sec_time_prefs.set("gettimeofday", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_GETTIMEOFDAY]);
	sec_time_prefs.set("clock", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCK]);
	sec_time_prefs.set("clock_gettime", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_CLOCKGETTIME]);
	sec_time_prefs.set("sdl_getticks", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETTICKS]);
	sec_time_prefs.set("sdl_getperformancecounter", context->config.sc.sec_gettimes_threshold[SharedConfig::TIMETYPE_SDLGETPERFORMANCECOUNTER]);

    config_prefs.flush();

    /* Compress the files into the final movie file */
    TAR *tar;
    tar_open(&tar, moviefile.c_str(), &gztype, O_WRONLY | O_CREAT, 0644, 0);
    /* I would like to use tar_append_tree but it saves files with their path */
    //tar_append_tree(tar, md, save_dir);
    char* input_ptr = const_cast<char*>(input_file.c_str());
    char savename[7] = "inputs";
    tar_append_file(tar, input_ptr, savename);
    std::string config_file = context->config.tempmoviedir + "/config.prefs";
    char* config_ptr = const_cast<char*>(config_file.c_str());
    char savename2[13] = "config.prefs";
    tar_append_file(tar, config_ptr, savename2);

    tar_append_eof(tar);
    tar_close(tar);
}

void MovieFile::saveMovie(const std::string& moviefile)
{
	saveMovie(moviefile, input_list.size());
}

void MovieFile::saveMovie()
{
    saveMovie(context->config.moviefile);
	modifiedSinceLastSave = false;
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
        input_stream << std::dec;
        input_stream << inputs.pointer_x << ':' << inputs.pointer_y << ':';
        input_stream.put((inputs.pointer_mask&Button1Mask)?'1':'.');
        input_stream.put((inputs.pointer_mask&Button2Mask)?'2':'.');
        input_stream.put((inputs.pointer_mask&Button3Mask)?'3':'.');
        input_stream.put((inputs.pointer_mask&Button4Mask)?'4':'.');
        input_stream.put((inputs.pointer_mask&Button5Mask)?'5':'.');
    }

    /* Write controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
        input_stream.put('|');
        input_stream << std::dec;
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

    /* Read mouse inputs */
    if (context->config.sc.mouse_support) {
        input_string >> d;
        input_string >> std::dec;
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

    /* Read controller inputs */
    for (int joy=0; joy<context->config.sc.nb_controllers; joy++) {
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

unsigned int MovieFile::nbFramesConfig()
{
    /* Load the config file into the context struct */
    Fl_Preferences config_prefs(context->config.tempmoviedir.c_str(), "movie", "config");
    int frame_count;
    config_prefs.get("frame_count", frame_count, 0);

    return frame_count;
}

unsigned int MovieFile::nbFrames()
{
	return input_list.size();
}

unsigned int MovieFile::nbRerecords()
{
    /* Load the config file into the context struct */
    Fl_Preferences config_prefs(context->config.tempmoviedir.c_str(), "movie", "config");
    int rerecord_count;
    config_prefs.get("rerecord_count", rerecord_count, 0);

    return rerecord_count;
}

int MovieFile::setInputs(const AllInputs& inputs)
{
    /* Check that we are writing to the next frame */
    if (context->framecount == input_list.size()) {
        input_list.push_back(inputs);
		modifiedSinceLastSave = true;
        return 0;
    }
    else if (context->framecount < input_list.size()) {
        /* Writing to a frame that is before the last one. We resize the input
         * list accordingly and append the frame at the end.
         */
        input_list.resize(context->framecount);
        input_list.push_back(inputs);
		modifiedSinceLastSave = true;
        return 0;
    }
    else {
        std::cerr << "Writing to a frame " << context->framecount << "higher than the current list " << input_list.size() << std::endl;
        return 1;
    }
}

int MovieFile::getInputs(AllInputs& inputs)
{
    if (context->framecount >= input_list.size()) {
        inputs.emptyInputs();
        return 0;
    }

	inputs = input_list[context->framecount];

    if ((context->framecount + 1) == input_list.size()) {
        /* We are reading the last frame of the movie, notify the caller */
        return 1;
    }

    return 0;
}

void MovieFile::close()
{
    // if (context->config.sc.recording != SharedConfig::NO_RECORDING)
    //     saveMovie();
}

bool MovieFile::isPrefix(const MovieFile& movie)
{
    /* Not a prefix if the size is greater */
    if (movie.input_list.size() > input_list.size())
        return false;

    return std::equal(movie.input_list.begin(), movie.input_list.end(), input_list.begin());
}
