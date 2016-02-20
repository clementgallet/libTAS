#include "recording.h"

FILE* openRecording(const char* filename, int recording)
{
    FILE* fp;

    if (recording) {
        fp = fopen(filename, "wb");
        writeHeader(fp);
    }
    else {
        fp = fopen(filename, "r+b");
        readHeader(fp);
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
    int start_position = HEADER_SIZE + frame * sizeof(struct AllInputs);
    long int current_position = ftell(fp);

    if (start_position != current_position) {
        printf("Did not write, bad position\n");
        return 0;
    }

    fwrite(inputs.keyboard, sizeof(char), 32, fp);
    return 1;
}

int readFrame(FILE* fp, unsigned long frame, struct AllInputs* inputs)
{
    int start_position = HEADER_SIZE + frame * sizeof(struct AllInputs);
    long int current_position = ftell(fp);

    if (start_position == current_position) {
        size_t size = fread(inputs->keyboard, sizeof(char), 32, fp);
        if (size != (32*sizeof(char))) {
            printf("Did not read all, end of file?\n");
            return 0;
        }
        return 1;
    }
    else {
        printf("Did not read, bad position\n");
    }
    return 0;
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
