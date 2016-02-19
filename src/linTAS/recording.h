#include <stdio.h>
#include "../shared/inputs.h"

#define HEADER_SIZE 256

FILE* openRecording(const char* filename, int recording);
void writeHeader(FILE* fp);
void readHeader(FILE* fp);
void writeFrame(FILE* fp, unsigned long frame, struct AllInputs inputs);
void readFrame(FILE* fp, unsigned long frame, struct AllInputs* inputs);
void closeRecording(FILE* fp);

