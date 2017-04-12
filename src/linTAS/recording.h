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

#ifndef RECORDING_H_INCLUDED
#define RECORDING_H_INCLUDED

#include <stdio.h>
#include <unistd.h>
#include "../shared/AllInputs.h"
#include "../shared/tasflags.h"

#define HEADER_SIZE 256

FILE* openRecording(const char* filename, TasFlags::RecStatus recording);
void writeHeader(FILE* fp);
void readHeader(FILE* fp);
int writeFrame(FILE* fp, unsigned long frame, struct AllInputs inputs);
int readFrame(FILE* fp, unsigned long frame, struct AllInputs* inputs);
void truncateRecording(FILE* fp);
void closeRecording(FILE* fp);

#endif
