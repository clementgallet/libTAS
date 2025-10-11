/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_BASEADDRESSES_H_INCLUDED
#define LIBTAS_BASEADDRESSES_H_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <string>
#include <cstdint>

/* Holds the base address for executable and each loaded library */
namespace BaseAddresses {

    /* Query all loaded libraries and executable, and store base and end addresses */
    void load();

    /* Query base and end addresses for a specific file that was not stored */
    std::pair<uintptr_t,uintptr_t> findNewFile(std::string file);

    /* Return the base address of a specific file */
    uintptr_t getBaseAddress(std::string file);

    /* Query base and end addresses from a file */
    std::pair<uintptr_t,uintptr_t> getAddress(std::string file);

    /* Return the address from a file and offset */
    uintptr_t getAddress(std::string file, off_t offset);

    /* Return the memory section of the mapped game executable */    
    std::pair<uintptr_t,uintptr_t> getExecutableSection();

    /* Get the file and offset from an address */
    std::string getFileAndOffset(uintptr_t addr, off_t &offset);

    /* Clear all addresses */
    void clear();
}

#endif
