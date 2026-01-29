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

#include "AutoSave.h"
#include "utils.h"
#include "movie/MovieFile.h"
#include "Context.h"

#include <iostream>
#include <dirent.h> // scandir
#include <unistd.h> // unlink

static time_t last_time_saved = time(nullptr);
static int nb_frame_advance = 0;

void AutoSave::update(Context* context, MovieFile& movie)
{
	/* Check if autosave is enabled */
	if (!context->config.autosave)
		return;

	/* Check if the movie was modified */
	if (!movie.inputs->modifiedSinceLastAutoSave)
		return;

	/* Update the frame counter and check if we must auto-save */
	if ((++nb_frame_advance > context->config.autosave_frames) &&
		(difftime(time(nullptr), last_time_saved) > context->config.autosave_delay_sec))
	{
		nb_frame_advance = 0;
		time(&last_time_saved);

		/* Build the autosave filename */
		std::filesystem::path moviename = context->config.moviefile.stem();

		/* We remove old saves here while we have the correct movie name */
		removeOldSaves(context, moviename.string());

		moviename = context->config.tempmoviedir / moviename;

		char buf[32];
		strftime(buf, 32, "_%Y%m%d-%H%M%S.ltm", localtime(&last_time_saved));

		moviename += buf;

		std::cout << "Autosave movie to " << moviename << std::endl;

		/* Save the movie */
		movie.saveMovie(moviename);

		movie.inputs->modifiedSinceLastAutoSave = false;
	}
}

void AutoSave::removeOldSaves(Context* context, const std::string& moviename)
{
	/* Scanning the directory of savefiles. We can't pass a filter function as
	 * lambda because only non-capturing lambdas can be converted to function
	 * pointers. We must filter when iterating. */
     
    /* We use a set to order files alphabetically */
    std::set<std::filesystem::path> sorted_files;

    for (auto &entry : std::filesystem::directory_iterator(context->config.tempmoviedir))
        sorted_files.insert(entry.path());

    int matches = 0;
    for (auto filename_it = sorted_files.rbegin(); filename_it != sorted_files.rend(); filename_it++) {
        std::string tempfilename = filename_it->filename().string();
        if ((tempfilename.size() == (moviename.size() + 20)) &&
            (0 == tempfilename.compare(0, moviename.size(), moviename))) {
            /* We found a matching autosave */
            matches++;
            if (matches > context->config.autosave_count) {
                /* Removing the autosave */
                std::cout << "Remove autosave movie " << *filename_it << std::endl;
                std::filesystem::remove(*filename_it);
            }
        }
        else {
            /* Little optimisation. If we already had a match, then we won't
             * have any more matches, so we can stop here */
            if (matches > 0)
                break;
        }
    }
}
