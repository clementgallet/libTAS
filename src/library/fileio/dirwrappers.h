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

#ifndef LIBTAS_DIR_H_INCLUDED
#define LIBTAS_DIR_H_INCLUDED

#include "../hook.h"
#include <dirent.h>

#ifndef __DARWIN_SUF_UNIX03
#define __DARWIN_SUF_UNIX03 ""
#endif

#ifndef __DARWIN_SUF_64_BIT_INO_T
#define __DARWIN_SUF_64_BIT_INO_T ""
#endif

#define __DARWIN_ALIAS_STR(x) x ## __DARWIN_SUF_UNIX03
#define __DARWIN_ALIAS_I_STR(x) x ## __DARWIN_SUF_64_BIT_INO_T ## __DARWIN_SUF_UNIX03
#define __DARWIN_INODE64_STR(x) x ## __DARWIN_SUF_64_BIT_INO_T

namespace libtas {

/* Open a directory stream on NAME.
   Return a DIR stream on the directory, or NULL if it could not be opened.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
OVERRIDE DIR *__DARWIN_ALIAS_I_STR(opendir) (const char *__name);

/* Same as opendir, but open the stream on the file descriptor FD.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
OVERRIDE DIR *__DARWIN_ALIAS_I_STR(fdopendir) (int __fd);

/* Close the directory stream DIRP.
   Return 0 if successful, -1 if not.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
OVERRIDE int __DARWIN_ALIAS_STR(closedir) (DIR *__dirp);

/* Read a directory entry from DIRP.  Return a pointer to a `struct
   dirent' describing the entry, or NULL for EOF or error.  The
   storage returned may be overwritten by a later readdir call on the
   same DIR stream.

   If the Large File Support API is selected we have to use the
   appropriate interface.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
OVERRIDE struct dirent *__DARWIN_INODE64_STR(readdir) (DIR *__dirp);

#ifdef __unix__
OVERRIDE struct dirent64 *readdir64 (DIR *__dirp);
#endif

OVERRIDE int __DARWIN_INODE64_STR(readdir_r) (DIR * dirp, struct dirent *entry, struct dirent **result);

#ifdef __unix__
OVERRIDE int readdir64_r (DIR * dirp, struct dirent64 *entry, struct dirent64 **result);
#endif

}

#endif
