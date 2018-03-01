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

#include "AutoSave.h"
#include "utils.h"
#include <iostream>

static const double time_threshold_sec = 600;
static const unsigned int frame_advance_threshold = 1000;

static time_t last_time_saved = time(nullptr);
static unsigned int nb_frame_advance = 0;

void AutoSave::update(Context* context, MovieFile& movie)
{
	/* Update the frame counter and check if we must auto-save */
	if ((++nb_frame_advance > frame_advance_threshold) &&
		(difftime(time(nullptr), last_time_saved) > time_threshold_sec))
	{
		nb_frame_advance = 0;
		time(&last_time_saved);

		/* Build the autosave filename */
		size_t sep = context->config.moviefile.find_last_of("/");
		std::string moviename = context->config.tempmoviedir + "/";
		if (sep != std::string::npos)
			moviename += context->config.moviefile.substr(sep + 1);
		else
			moviename += context->config.moviefile;

		/* Remove the extension if any */
		if (moviename.compare(moviename.size() - 4, 4, ".ltm") == 0) {
			moviename.resize(moviename.size() - 4);
		}

		char buf[32];
		strftime(buf, 32, "_%Y%m%d-%H%M%S.ltm", localtime(&last_time_saved));

		moviename += buf;

		std::cout << "Autosave movie to " << moviename << std::endl;

		/* Save the movie */
		movie.saveMovie(moviename);
	}
}
