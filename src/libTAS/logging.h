#ifndef LOGGING_H_INCL
#define LOGGING_H_INCL

#include "../shared/lcf.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "../shared/tasflags.h"
#include "hook.h" // For pthread_self_real
#include "time.h" // For frame_counter

/* Color printing
 * Taken from http://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
 */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_GRAY    "\x1b[37m"

#define ANSI_COLOR_LIGHT_RED     "\x1b[91m"
#define ANSI_COLOR_LIGHT_GREEN   "\x1b[92m"
#define ANSI_COLOR_LIGHT_YELLOW  "\x1b[93m"
#define ANSI_COLOR_LIGHT_BLUE    "\x1b[94m"
#define ANSI_COLOR_LIGHT_MAGENTA "\x1b[95m"
#define ANSI_COLOR_LIGHT_CYAN    "\x1b[96m"
#define ANSI_COLOR_LIGHT_GRAY    "\x1b[97m"

#define ANSI_COLOR_RESET   "\x1b[0m"

std::string stringify(unsigned long int id);

void errlog();
inline void errlog() {}

template<typename First, typename ...Rest>
void errlog (First && first, Rest && ...rest);
template<typename First, typename ...Rest>
inline void errlog (First && first, Rest && ...rest)
{
    std::cerr << std::forward<First>(first);
    errlog(std::forward<Rest>(rest)...);
}

/* Because of how templates work, the definition of the function has to be
 * in the header so the compiler knows for which types he has to build
 * a function. As a consequence, it increases the generated library by 200 kB...
 * Maybe we should switch back to the old C varargs
 */

template<typename ...Args>
void debuglog(LogCategoryFlag lcf, Args ...args);
template<typename ...Args>
inline void debuglog(LogCategoryFlag lcf, Args ...args)
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
            //if (isMainThread())
            //    std::cerr << "Thread " << thstr << " (main) ";
            //else
                std::cerr << "Thread " << thstr << "        ";
        }

        /* Reset color change */
        std::cerr << ANSI_COLOR_RESET;

        /* Output arguments */
        errlog(std::forward<Args>(args)...);

        std::cerr << std::endl;
    }
}

#endif // LOGCATEGORY_H_INCL
