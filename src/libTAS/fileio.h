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

#ifndef LIBTAS_FILEIO_H_INCLUDED
#define LIBTAS_FILEIO_H_INCLUDED

#ifdef LIBTAS_ENABLE_FILEIO_HOOKING

#include "../external/SDL.h"
#include "global.h"
#include <cstdio>

/* RWops Types */
#define SDL_RWOPS_UNKNOWN   0   /* Unknown stream type */
#define SDL_RWOPS_WINFILE   1   /* Win32 file */
#define SDL_RWOPS_STDFILE   2   /* Stdio file */
#define SDL_RWOPS_JNIFILE   3   /* Android asset */
#define SDL_RWOPS_MEMORY    4   /* Memory stream */
#define SDL_RWOPS_MEMORY_RO 5   /* Read-Only memory stream */

/**
 * This is the read/write operation structure -- very basic.
 */
typedef struct SDL_RWops
{
    /**
     *  Return the size of the file in this rwops, or -1 if unknown
     */
    Sint64 (*size) (struct SDL_RWops * context);

    /**
     *  Seek to \c offset relative to \c whence, one of stdio's whence values:
     *  RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
     *
     *  \return the final offset in the data stream, or -1 on error.
     */
    Sint64 (*seek) (struct SDL_RWops * context, Sint64 offset,
                             int whence);

    /**
     *  Read up to \c maxnum objects each of size \c size from the data
     *  stream to the area pointed at by \c ptr.
     *
     *  \return the number of objects read, or 0 at error or end of file.
     */
    size_t (*read) (struct SDL_RWops * context, void *ptr,
                             size_t size, size_t maxnum);

    /**
     *  Write exactly \c num objects each of size \c size from the area
     *  pointed at by \c ptr to data stream.
     *
     *  \return the number of objects written, or 0 at error or end of file.
     */
    size_t (*write) (struct SDL_RWops * context, const void *ptr,
                              size_t size, size_t num);

    /**
     *  Close and free an allocated SDL_RWops structure.
     *
     *  \return 0 if successful or -1 on write error when flushing data.
     */
    int (*close) (struct SDL_RWops * context);

    Uint32 type;
    union
    {
        struct
        {
            SDL_bool autoclose;
            FILE *fp;
        } stdio;
        struct
        {
            Uint8 *base;
            Uint8 *here;
            Uint8 *stop;
        } mem;
        struct
        {
            void *data1;
            void *data2;
        } unknown;
    } hidden;

} SDL_RWops;


/**
 *  \name RWFrom functions
 *
 *  Functions to create SDL_RWops structures from various data streams.
 */
/* @{ */

OVERRIDE SDL_RWops *SDL_RWFromFile(const char *file, const char *mode);
OVERRIDE SDL_RWops *SDL_RWFromFP(FILE * fp, SDL_bool autoclose);


/* Open a file and create a new stream for it. */
OVERRIDE FILE *fopen (const char *filename, const char *modes);
OVERRIDE FILE *fopen64 (const char *filename, const char *modes);

/* Write formatted output to STREAM. */
OVERRIDE int fprintf (FILE *stream, const char *format, ...);

/* Write formatted output to S from argument list ARG. */
OVERRIDE int vfprintf (FILE *s, const char *format, va_list arg);

/* Write a character to STREAM. */
OVERRIDE int fputc (int c, FILE *stream);
OVERRIDE int putc (int c, FILE *stream);

/* Write chunks of generic data to STREAM. */
OVERRIDE size_t fwrite (const void *ptr, size_t size,
		      size_t n, FILE *s);


/* Open FILE and return a new file descriptor for it, or -1 on error.
   OFLAG determines the type of access used.  If O_CREAT or O_TMPFILE is set
   in OFLAG, the third argument is taken as a `mode_t', the mode of the
   created file. */
OVERRIDE int open (const char *file, int oflag, ...);
OVERRIDE int open64 (const char *__file, int __oflag, ...);

/* Similar to `open' but a relative path name is interpreted relative to
   the directory for which FD is a descriptor.

   NOTE: some other `openat' implementation support additional functionality
   through this interface, especially using the O_XATTR flag.  This is not
   yet supported here. */
OVERRIDE int openat (int fd, const char *file, int oflag, ...);
OVERRIDE int openat64 (int fd, const char *file, int oflag, ...);

/* Create and open FILE, with mode MODE.  This takes an `int' MODE
   argument because that is what `mode_t' will be widened to. */
OVERRIDE int creat (const char *file, mode_t mode);
OVERRIDE int creat64 (const char *file, mode_t mode);

/* Write N bytes of BUF to FD.  Return the number written, or -1. */
OVERRIDE ssize_t write (int fd, const void *buf, size_t n);

/* Write N bytes of BUF to FD at the given position OFFSET without
   changing the file pointer.  Return the number written, or -1. */
OVERRIDE ssize_t pwrite (int fd, const void *buf, size_t n,
		       __off_t offset);

/* Write N bytes of BUF to FD at the given position OFFSET without
   changing the file pointer.  Return the number written, or -1.  */
OVERRIDE ssize_t pwrite64 (int fd, const void *buf, size_t n,
			 __off64_t offset);

#endif

void link_sdlfileio(void);
void link_stdiofileio(void);
void link_posixfileio(void);

#endif

