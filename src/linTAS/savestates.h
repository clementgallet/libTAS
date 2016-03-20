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

#ifndef SAVESTATES_H_INCLUDED
#define SAVESTATES_H_INCLUDED

#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/uio.h>

/* Store a section of the game memory */
struct StateSection {
    /* All information gather from a single line of /proc/pid/maps */
    unsigned long long int addr;
    unsigned long long int endaddr;
    int readflag;
    int writeflag;
    int execflag;
    unsigned long long int offset;
    char device[8];
    unsigned long long int inode;
    char* filename;

    /* The actual memory inside this section */
    char* mem;
};

/* Store the full game memory */
struct State {
    /* Meta data */
    long frame_count;

    /* Memory sections */
    int n_sections;
    unsigned long long total_size;
    struct StateSection* sections;
};


void attachToGame(pid_t game_pid);
void detachToGame(pid_t game_pid);
int 
read_mapping (FILE *mapfile, 
          unsigned long long int *addr, 
          unsigned long long int *endaddr, 
          char *permissions, 
          unsigned long long int *offset, 
          char *device, 
          unsigned long long int *inode, 
          char *filename);

void saveState(pid_t game_pid, struct State* state);
void loadState(pid_t game_pid, struct State* state);
void deallocState(struct State* state);

#endif

