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

#ifndef LIBTAS_SAVEFILELIST_H_INCLUDED
#define LIBTAS_SAVEFILELIST_H_INCLUDED

#include <cstdio> // FILE
#include <string>
#include <forward_list>
#include <memory>

namespace libtas {

class SaveFile;

namespace SaveFileList {

std::forward_list<std::unique_ptr<SaveFile>>::const_iterator begin();
std::forward_list<std::unique_ptr<SaveFile>>::const_iterator end();

/* Is the file considered a savefile? */
bool isSaveFile(const char *file, const char *modes);
bool isSaveFile(const char *file, int oflag);
bool isSaveFile(const char *file);

/* Check if the savefile already exists and open it, or open a new savefile */
FILE *openSaveFile(const char *file, const char *modes);
int openSaveFile(const char *file, int oflag);

/* Try to close a savefile if it exists.
 * Returns 1 if not a savefile, 0 if savefile closed successfully and
 * -1 if an error occurred during closing (and errno set)
 */
int closeSaveFile(int fd);
int closeSaveFile(FILE *stream);

/* Determine if the removed file is a savefile.
 * Returns 1 if not a savefile, 0 if savefile removed successfully and
 * -1 if an error occurred during removal (and errno set)
 */
int removeSaveFile(const char *file);

/* Try to rename a savefile if it exists.
 * Returns 1 if not a savefile, 0 if savefile renamed successfully and
 * -1 if an error occurred during renaming (and errno set)
 */
int renameSaveFile(const char *oldfile, const char *newfile);

/* Get the savefile object */
const SaveFile* getSaveFile(const char *file);
const SaveFile* getSaveFile(int fd);

/* Get the file descriptor of a savefile, or 0 if not a savefile */
int getSaveFileFd(const char *file);

/* Get if savefile was removed */
bool isSaveFileRemoved(const char *file);

/* Get the n-th save file inside directory `dir`. Returns nullptr if not present */
const SaveFile* getSaveFileInsideDir(std::string dir, int n);

}

}

#endif
