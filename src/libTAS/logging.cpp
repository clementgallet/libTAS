#include "logging.h"
#include <stdlib.h>



/* Print long integers as string for shorter ids. Use base64 */
std::string stringify(unsigned long int id)
{
    int i = 0;
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

