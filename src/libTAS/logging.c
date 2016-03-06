#include "logging.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../shared/tasflags.h"
#include "hook.h"
#include "threads.h"


void debuglog(LogCategoryFlag lcf, const char* fmt, ...)
{
    /* Use the extern variable tasflags */
    if ( (lcf & tasflags.includeFlags) && !(lcf & tasflags.excludeFlags) ) {
        char str[4096];
        memset(str, '\0', sizeof(str));
        if (lcf & LCF_ERROR)
            /* Write the text in red */
            strcat(str, ANSI_COLOR_RED);
        else
            /* Write the header text in white */
            strcat(str, ANSI_COLOR_LIGHT_GRAY);
        strcat(str, "[libTAS] ");
        size_t str_len = strlen(str);
        if (pthread_self_real) {
            char thstr[12];
            stringify(pthread_self_real(), thstr);
            if (isMainThread())
                snprintf(str + str_len, 4096 - str_len - 1, "Thread %s (main) ", thstr);
            else
                snprintf(str + str_len, 4096 - str_len - 1, "Thread %s        ", thstr);
        }

        /* Reset color change */
        strcat(str, ANSI_COLOR_RESET);

        str_len = strlen(str);

        va_list args;
        va_start(args, fmt);
        vsnprintf(str + str_len, 4096 - str_len - 1, fmt, args);
        va_end(args);

        strcat(str, "\n");
        fprintf(stderr, str);
    }
}

/* Print long integers as string for shorter ids. Use base64 */
void stringify(unsigned long int id, char* str)
{
    int i = 0;
    while (id) {
        unsigned long digit = id % 64;
        if (digit < 26) str[i] = (char)('A' + digit);
        else if (digit < 52) str[i] = (char)('a' + (digit - 26));
        else if (digit < 62) str[i] = (char)('0' + (digit - 52));
        else if (digit == 62) str[i] = '+';
        else str[i] = '/';

        id /= 64;
        i++;
    }
    str[i] = '\0';
}
