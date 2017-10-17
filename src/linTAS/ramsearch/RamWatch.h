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

#ifndef LINTAS_RAMWATCH_H_INCLUDED
#define LINTAS_RAMWATCH_H_INCLUDED

#include "CompareEnums.h"
#include "IRamWatch.h"
#include <cstdint>
#include <sys/types.h>
#include <sys/uio.h>
#include <cerrno>
#include <iostream>
#include <sstream>

template <class T>
class RamWatch : public IRamWatch {
public:
    T previous_value;

    std::string get_line()
    {
        std::ostringstream oss;
        oss << std::hex << address << '\t';
        oss << std::dec << get_value() << '\t';
        oss << previous_value;
        return oss.str();
    }

    std::string get_line_update()
    {
        previous_value = get_value();
        std::ostringstream oss;
        oss << std::hex << address << '\t';
        oss << std::dec << previous_value << '\t';
        oss << previous_value;
        return oss.str();
    }

    T get_value()
    {
        struct iovec local, remote;
        uint64_t value = 0;
        local.iov_base = static_cast<void*>(&value);
        local.iov_len = sizeof(T);
        remote.iov_base = reinterpret_cast<void*>(address);
        remote.iov_len = sizeof(T);

        last_read = process_vm_readv(game_pid, &local, 1, &remote, 1, 0);

        /* Checking for errors */
        if (last_read == -1) {
            // switch (errno) {
            //     case EINVAL:
            //         std::cerr << "The amount of bytes to read is too big!" << std::endl;
            //         break;
            //     case EFAULT:
            //         std::cerr << "Bad address space of the game process or own process!" << std::endl;
            //         break;
            //     case ENOMEM:
            //         std::cerr << "Could not allocate memory for internal copies of the iovec structures." << std::endl;
            //         break;
            //     case EPERM:
            //         std::cerr << "Do not have permission to read the game process memory." << std::endl;
            //         break;
            //     case ESRCH:
            //         std::cerr << "The game PID does not exist." << std::endl;
            //         break;
            // }
        }

        return value;
    }

    bool update()
    {
        T value = get_value(); // sets last_read == -1 if error
        if (last_read == -1)
            return true;

        previous_value = value;
        return false;
    }

    bool search(CompareType compare_type, CompareOperator compare_operator, double compare_value_db)
    {
        T value = get_value(); // sets last_read == -1 if error
        if (last_read == -1)
            return true;

        T compare_value;

        if (compare_type == CompareType::Previous) {
            compare_value = previous_value;
        }
        if (compare_type == CompareType::Value) {
            compare_value = static_cast<T>(compare_value_db);
        }

        switch(compare_operator) {
            case CompareOperator::Equal:
                if (value != compare_value)
                    return true;
                break;
            case CompareOperator::NotEqual:
                if (value == compare_value)
                    return true;
                break;
            case CompareOperator::Less:
                if (value >= compare_value)
                    return true;
                break;
            case CompareOperator::Greater:
                if (value <= compare_value)
                    return true;
                break;
            case CompareOperator::LessEqual:
                if (value > compare_value)
                    return true;
                break;
            case CompareOperator::GreaterEqual:
                if (value < compare_value)
                    return true;
                break;
        }

        previous_value = value;
        return false;
    }
};

#endif
