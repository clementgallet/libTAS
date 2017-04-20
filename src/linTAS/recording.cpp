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

#include "recording.h"

FILE* openRecording(const char* filename, Context::RecStatus recording)
{
    FILE* fp;

    switch(recording) {
        case Context::RECORDING_WRITE:
            fp = fopen(filename, "wb");
            writeHeader(fp);
            break;
        case Context::RECORDING_READ_WRITE:
            fp = fopen(filename, "r+b");
            readHeader(fp);
            break;
        case Context::RECORDING_READ_ONLY:
            fp = fopen(filename, "rb");
            readHeader(fp);
            break;
    }
    return fp;
}

void writeHeader(FILE* fp)
{
    int i;
    //fseek(fp, 0, SEEK_SET);

    /* TODO: Placeholder for now. Will fill information later */
    for (i=0; i<HEADER_SIZE; i++)
        fputc(-1, fp);
}

void readHeader(FILE* fp)
{
    /* TODO: Placeholder for now. Will fill information later */
    fseek(fp, HEADER_SIZE, SEEK_SET);
}


int writeFrame(FILE* fp, unsigned long frame, struct AllInputs inputs)
{
    fwrite(inputs.keyboard, sizeof(KeySym), AllInputs::MAXKEYS, fp);
    fwrite(&inputs.pointer_x, sizeof(int), 1, fp);
    fwrite(&inputs.pointer_y, sizeof(int), 1, fp);
    fwrite(&inputs.pointer_mask, sizeof(unsigned int), 1, fp);
    fwrite(inputs.controller_axes, sizeof(short), AllInputs::MAXJOYS*AllInputs::MAXAXES, fp);
    fwrite(inputs.controller_buttons, sizeof(unsigned short), AllInputs::MAXJOYS, fp);
    return 1;
}

int readFrame(FILE* fp, unsigned long frame, struct AllInputs* inputs)
{
    size_t size = fread(inputs->keyboard, sizeof(KeySym), AllInputs::MAXKEYS, fp);
    size += fread(&inputs->pointer_x, sizeof(int), 1, fp);
    size += fread(&inputs->pointer_y, sizeof(int), 1, fp);
    size += fread(&inputs->pointer_mask, sizeof(unsigned int), 1, fp);
    size += fread(inputs->controller_axes, sizeof(short), AllInputs::MAXJOYS*AllInputs::MAXAXES, fp);
    size += fread(inputs->controller_buttons, sizeof(unsigned short), AllInputs::MAXJOYS, fp);
    //if (size != sizeof(struct AllInputs)) {
    //    printf("Did not read all (%zu elements), end of file?\n", size);
    //    return 0;
    //}
    return 1;
}

void truncateRecording(FILE* fp)
{
    long current_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) != current_pos) {
        /* We have to truncate the file */

        fseek(fp, current_pos, SEEK_SET);

        /* We are mixing ANSI C functions (fseek, ftell) with POSIX functions (ftruncate)
         * that do not work on the same layer. So it is safer to flush any operations
         * before truncate the file.
         */
        fflush(fp);

        if (ftruncate(fileno(fp), current_pos) != 0)
            fprintf(stderr, "Cound not truncate recording file\n");
    }

}

void closeRecording(FILE* fp)
{
    /* TODO: Write some stuff in the header */

    fclose(fp);
}
