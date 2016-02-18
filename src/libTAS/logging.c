#include "logging.h"

void debuglog(LogCategoryFlag lcf, struct TasFlags tasflags, const char* fmt, ...)
{
    if ( (lcf & tasflags.includeFlags) && !(lcf & tasflags.excludeFlags) ) {
        char str[4096];
        memset(str, '\0', sizeof(str));
        strcpy(str, "[libTAS] ");
        size_t str_len = strlen(str);

        va_list args;
        va_start(args, fmt);
        vsnprintf(str + str_len, 4096 - str_len - 1, fmt, args);
        va_end(args);

        strcat(str, "\n");
        fprintf(stderr, str);
    }
}

