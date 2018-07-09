/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_RAMWATCHDETAILED_H_INCLUDED
#define LINTAS_RAMWATCHDETAILED_H_INCLUDED

#include "IRamWatchDetailed.h"
#include <sstream>
#include <sys/uio.h>

template <class T>
class RamWatchDetailed : public IRamWatchDetailed {
public:
    RamWatchDetailed(uintptr_t addr) : IRamWatchDetailed(addr) {};

    T get_value()
    {
        struct iovec local, remote;
        if (isPointer) {
            address = base_address;
            for (auto offset : pointer_offsets) {
                local.iov_base = static_cast<void*>(&address);
                local.iov_len = sizeof(uintptr_t);
                remote.iov_base = reinterpret_cast<void*>(address);
                remote.iov_len = sizeof(uintptr_t);

                process_vm_readv(game_pid, &local, 1, &remote, 1, 0);

                address += offset;
            }
        }
        
        T value = 0;
        local.iov_base = static_cast<void*>(&value);
        local.iov_len = sizeof(T);
        remote.iov_base = reinterpret_cast<void*>(address);
        remote.iov_len = sizeof(T);

        process_vm_readv(game_pid, &local, 1, &remote, 1, 0);

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
        return oss.str();
    }
};

#endif
