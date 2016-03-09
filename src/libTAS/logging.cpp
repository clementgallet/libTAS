#include "logging.h"
#include <stdlib.h>
#include "../shared/tasflags.h"
#include "hook.h"
#include "threads.h"
#include "time.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::string stringify(unsigned long int id);
void errlog();
template<typename First, typename ...Rest>
void errlog (First && first, Rest && ...rest);


void errlog() {}

template<typename First, typename ...Rest>
void errlog (First && first, Rest && ...rest)
{
    std::cerr << std::forward<First>(first);
    errlog(std::forward<Rest>(rest)...);
}

template<typename ...Args>
void debuglog(LogCategoryFlag lcf, Args ...args)
{
    /* Use the extern variable tasflags */
    if ( (lcf & tasflags.includeFlags) && !(lcf & tasflags.excludeFlags) ) {
        if (lcf & LCF_ERROR)
            /* Write the text in red */
            std::cerr << ANSI_COLOR_RED;
        else
            /* Write the header text in white */
            std::cerr << ANSI_COLOR_LIGHT_GRAY;

        std::cerr << "[libTAS f:" << std::setw(6) << frame_counter << "] ";

        if (pthread_self_real) {
            std::string thstr = stringify(pthread_self_real());
            if (isMainThread())
                std::cerr << "Thread " << thstr << " (main) ";
            else
                std::cerr << "Thread " << thstr << "        ";
        }

        /* Reset color change */
        std::cerr << ANSI_COLOR_RESET;

        /* Output arguments */
        errlog(std::forward<Args>(args)...);

        std::cerr << std::endl;
    }
}

/* Print long integers as string for shorter ids. Use base64 */
std::string stringify(unsigned long int id)
{
    int i = 0;
    std::ostringstream oss;
    while (id) {
        unsigned long digit = id % 64;
        if (digit < 26) oss << ('A' + digit);
        else if (digit < 52) oss << ('a' + (digit - 26));
        else if (digit < 62) oss << ('0' + (digit - 52));
        else if (digit == 62) oss << '+';
        else oss << '/';

        id /= 64;
    }
    return oss.str();
}

