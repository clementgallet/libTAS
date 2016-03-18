#include "logging.h"
#include <stdlib.h>
#include "threads.h"
#include "../shared/tasflags.h"

void debuglogverbose(LogCategoryFlag lcf, std::string str)
{
    std::ostringstream oss;

    /* Use the extern variable tasflags */
    if ( (lcf & tasflags.includeFlags) && !(lcf & tasflags.excludeFlags) ) {
        if (lcf & LCF_ERROR)
            /* Write the header text in red */
            oss << ANSI_COLOR_RED;
        else if (lcf & LCF_TODO)
            /* Write the header text in light red */
            oss << ANSI_COLOR_LIGHT_RED;
        else
            /* Write the header text in white */
            oss << ANSI_COLOR_LIGHT_GRAY;

        oss << "[libTAS f:" << std::setw(6) << frame_counter << "] ";

        if (pthread_self_real) {
            std::string thstr = stringify(pthread_self_real());
            if (isMainThread())
                oss << "Thread " << thstr << " (main) ";
            else
                oss << "Thread " << thstr << "        ";
        }

        /* Reset color change */
        oss << ANSI_COLOR_RESET;

        /* Output arguments */
        oss << str << std::endl;

        std::cerr << oss.str();
    }
}



/* Print long integers as string for shorter ids. Use base64 */
std::string stringify(unsigned long int id)
{
    std::ostringstream oss;
    while (id) {
        unsigned long digit = id % 64;
        if (digit < 26) oss << (char)('A' + digit);
        else if (digit < 52) oss << (char)('a' + (digit - 26));
        else if (digit < 62) oss << (char)('0' + (digit - 52));
        else if (digit == 62) oss << '+';
        else oss << '/';

        id /= 64;
    }
    return oss.str();
}

