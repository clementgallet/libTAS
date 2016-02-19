#include "recording.h"

FILE* openRecording(const char* filename, int recording)
{
    FILE* fp;

    if (recording) {
        fp = fopen(filename, "wb");
        writeHeader(fp);
    }
    else {
        fp = fopen(filename, "rb");
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


void writeFrame(FILE* fp, unsigned long frame, struct AllInputs inputs)
{
    int start_position = HEADER_SIZE + frame * sizeof(struct AllInputs);
    long int current_position = ftell(fp);

    if (start_position == current_position) {
        fwrite(inputs.keyboard, sizeof(char), 32, fp);
    }
    else {
        printf("Did not write, bad position\n");
    }
}

void readFrame(FILE* fp, unsigned long frame, struct AllInputs* inputs)
{
    int start_position = HEADER_SIZE + frame * sizeof(struct AllInputs);
    long int current_position = ftell(fp);

    if (start_position == current_position) {
        size_t size = fread(inputs->keyboard, sizeof(char), 32, fp);
        if (size != (32*sizeof(char))) {
            printf("Did not read all, end of fil?\n");
        }
    }
    else {
        printf("Did not read, bad position\n");
    }
}

void closeRecording(FILE* fp)
{
    /* TODO: Write some stuff in the header */

    fclose(fp);
} 
