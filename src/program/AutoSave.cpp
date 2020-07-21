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

#include "AutoSave.h"
#include "utils.h"
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
	if (!movie.modifiedSinceLastAutoSave)
		return;

	/* Update the frame counter and check if we must auto-save */
	if ((++nb_frame_advance > context->config.autosave_frames) &&
		(difftime(time(nullptr), last_time_saved) > context->config.autosave_delay_sec))
	{
		nb_frame_advance = 0;
		time(&last_time_saved);

		/* Build the autosave filename */
		std::string moviename = fileFromPath(context->config.moviefile);

		/* Remove the extension if any */
		if (moviename.compare(moviename.size() - 4, 4, ".ltm") == 0) {
			moviename.resize(moviename.size() - 4);
		}

		/* We remove old saves here while we have the correct movie name */
		removeOldSaves(context, moviename.c_str());

		moviename = context->config.tempmoviedir + "/" + moviename;

		char buf[32];
		strftime(buf, 32, "_%Y%m%d-%H%M%S.ltm", localtime(&last_time_saved));

		moviename += buf;

		std::cout << "Autosave movie to " << moviename << std::endl;

		/* Save the movie */
		movie.saveMovie(moviename);

		movie.modifiedSinceLastAutoSave = false;
	}
}

void AutoSave::removeOldSaves(Context* context, const char* moviename)
{
	struct dirent **savefiles;

	/* Scanning the directory of savefiles. We can't pass a filter function as
	 * lambda because only non-capturing lambdas can be converted to function
	 * pointers. We must filter when iterating. */
	int nfiles = scandir(context->config.tempmoviedir.c_str(), &savefiles, nullptr, alphasort);

	if (nfiles < 0) {
		std::cerr << "Could not scan directory " << context->config.tempmoviedir << std::endl;
		return;
	}

	int matches = 0;
	for (int i = nfiles-1; i >= 0; i--)
    {
        struct dirent *file = savefiles[i];
		if ((strlen(file->d_name) == (strlen(moviename) + 20)) &&
			(strncmp(file->d_name, moviename, strlen(moviename)) == 0)) {
			/* We found a matching autosave */
			matches++;
			if (matches > context->config.autosave_count) {
				/* Removing the autosave */
				std::string autosave = context->config.tempmoviedir;
				autosave += "/";
				autosave += file->d_name;
				std::cout << "Remove autosave movie " << autosave << std::endl;
				unlink(autosave.c_str());
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
