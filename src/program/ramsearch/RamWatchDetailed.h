/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_RAMWATCHDETAILED_H_INCLUDED
#define LIBTAS_RAMWATCHDETAILED_H_INCLUDED

#include "IRamWatchDetailed.h"
#include "TypeIndex.h"
#include "MemAccess.h"
#include <sstream>
#include <iostream>

template <class T>
class RamWatchDetailed : public IRamWatchDetailed {
public:
    RamWatchDetailed(uintptr_t addr) : IRamWatchDetailed(addr) {};

    T get_value()
    {
        update_addr();

        if (!isValid)
            return 0;

        T value = 0;
        isValid = (MemAccess::read(&value, reinterpret_cast<void*>(address), sizeof(T)) == sizeof(T));
        return value;
    }

    std::string value_str()
    {
        std::ostringstream oss;
        if (hex) oss << std::hex;
        /* Output char and unsigned char as integer values. There might be a
         * more elegant solution.
         */
        if (std::is_same<T, char>::value) {
            oss << static_cast<int>(get_value());
        }
        else if (std::is_same<T, unsigned char>::value) {
            oss << static_cast<unsigned int>(get_value());
        }
        else {
            oss << get_value();
        }
        if (!isValid)
            return std::string("??????");

        return oss.str();
    }

    int poke_value(std::string str_value)
    {
        std::istringstream iss(str_value);
        T value;
        if (hex) iss >> std::hex;

        /* ISS will consider char and unsigned char as text element. To get the
         * integer value, I need to cast it to another integer type.
         */
        if (std::is_same<T, char>::value) {
            int intval;
            iss >> intval;
            value = static_cast<char>(intval);
        }
        else if (std::is_same<T, unsigned char>::value) {
            unsigned int uintval;
            iss >> uintval;
            value = static_cast<unsigned char>(uintval);
        }
        else {
            iss >> value;
        }

        /* Write value into the game process address */
        return MemAccess::write(&value, reinterpret_cast<void*>(address), sizeof(T));
    }

    int type()
    {
        return type_index<T>();
    }

};

#endif
