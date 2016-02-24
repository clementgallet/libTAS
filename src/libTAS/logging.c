#include "logging.h"

void debuglog(LogCategoryFlag lcf, const char* fmt, ...)
{
    /* Use the extern variable tasflags */
    if ( (lcf & tasflags.includeFlags) && !(lcf & tasflags.excludeFlags) ) {
        char str[4096];
        memset(str, '\0', sizeof(str));
        if (lcf & LCF_ERROR)
            /* Write the text in red */
            strcat(str, ANSI_COLOR_RED);
        else if ((lcf & LCF_FRAME) || (lcf & LCF_FREQUENT))
            /* Write the text in gray */
            strcat(str, ANSI_COLOR_GRAY);
        strcat(str, "[libTAS] ");
        size_t str_len = strlen(str);

        va_list args;
        va_start(args, fmt);
        vsnprintf(str + str_len, 4096 - str_len - 1, fmt, args);
        va_end(args);

        if ((lcf & LCF_ERROR) || (lcf & LCF_FRAME) || (lcf & LCF_FREQUENT))
            /* Reset color change */
            strcat(str, ANSI_COLOR_RESET);
        strcat(str, "\n");
        fprintf(stderr, str);
    }
}

