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

#include "IRamWatchDetailed.h"
#include "MemSection.h"
#include "../utils.h"
#include <sys/uio.h>
#include <sstream>
#include <fstream>
#include <iostream>

bool IRamWatchDetailed::isValid;
pid_t IRamWatchDetailed::game_pid;

void IRamWatchDetailed::update_addr()
{
    isValid = true;
    if (isPointer) {
        struct iovec local, remote;

        /* Update the base address from the file and file offset */
        if (!base_address) {

            /* If file is empty, address is absolute */
            if (base_file.empty()) {
                base_address = base_file_offset;
            }
            else {
                /* Compose the filename for the /proc memory map, and open it. */
                std::ostringstream oss;
                oss << "/proc/" << game_pid << "/maps";
                std::ifstream mapsfile(oss.str());
                if (!mapsfile) {
                    std::cerr << "Could not open " << oss.str() << std::endl;
                    return;
                }

                std::string line;
                MemSection::reset();

                while (std::getline(mapsfile, line)) {
                    MemSection section;
                    section.readMap(line);
                    std::string file = fileFromPath(section.filename);

                    if (base_file.compare(file) == 0) {
                        if ((base_file_offset >= 0) &&
                            (base_file_offset >= section.offset) &&
                            (base_file_offset < static_cast<off_t>(section.offset + section.size))) {

                            base_address = section.addr - section.offset + base_file_offset;
                            break;
                        }
                        if (base_file_offset < 0) {
                            base_address = section.endaddr + base_file_offset;
                            break;                            
                        }
                    }
                }
            }
        }

        address = base_address;
        for (auto offset : pointer_offsets) {
            uintptr_t next_address;
            local.iov_base = static_cast<void*>(&next_address);
            local.iov_len = sizeof(uintptr_t);
            remote.iov_base = reinterpret_cast<void*>(address);
            remote.iov_len = sizeof(uintptr_t);

            isValid = (process_vm_readv(game_pid, &local, 1, &remote, 1, 0) == sizeof(uintptr_t));
            if (!isValid)
                return;

            address = next_address + offset;
        }
    }

}
