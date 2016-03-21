/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "logging.h"
#include <stdlib.h>
#include "threads.h"
#include "../shared/tasflags.h"
#include "unistd.h" // For isatty

void debuglogverbose(LogCategoryFlag lcf, std::string str)
{
    std::ostringstream oss;

    /* Use the extern variable tasflags */
    if ( (lcf & tasflags.includeFlags) && !(lcf & tasflags.excludeFlags) ) {
        /* We only print colors if displayed on a terminal */
        bool isTerm = isatty(/*cerr*/ 2);
        if (isTerm) {
            if (lcf & LCF_ERROR)
                /* Write the header text in red */
                oss << ANSI_COLOR_RED;
            else if (lcf & LCF_TODO)
                /* Write the header text in light red */
                oss << ANSI_COLOR_LIGHT_RED;
            else
                /* Write the header text in white */
                oss << ANSI_COLOR_LIGHT_GRAY;
        }
        oss << "[libTAS f:" << frame_counter << "] ";

        if (pthread_self_real) {
            std::string thstr = stringify(pthread_self_real());
            if (isMainThread())
                oss << "Thread " << thstr << " (main) ";
            else
                oss << "Thread " << thstr << "        ";
        }

        if (isTerm) {
            /* Reset color change */
            oss << ANSI_COLOR_RESET;
        }

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

