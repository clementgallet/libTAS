#ifndef LOGGING_H_INCL
#define LOGGING_H_INCL

#include "../shared/lcf.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
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
void debuglogverbose(LogCategoryFlag lcf, std::string str);

void catlog(std::ostringstream &oss);
inline void catlog(std::ostringstream &oss) {}

template<typename First, typename ...Rest>
void catlog (std::ostringstream &oss, First && first, Rest && ...rest);
template<typename First, typename ...Rest>
inline void catlog (std::ostringstream &oss, First && first, Rest && ...rest)
{
    oss << std::forward<First>(first);
    catlog(oss, std::forward<Rest>(rest)...);
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
    std::ostringstream oss;
    catlog(oss, std::forward<Args>(args)...);
    debuglogverbose(lcf, oss.str());
}

#define DEBUGLOGCALL(lcf) debuglog(lcf, __func__, " call.")

#endif // LOGCATEGORY_H_INCL

