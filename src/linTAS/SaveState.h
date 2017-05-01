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

#ifndef LINTAS_SAVESTATE_H_INCLUDED
#define LINTAS_SAVESTATE_H_INCLUDED

#include "StateSection.h"
#include "ThreadInfo.h"
#include <vector>
#include <memory>

/* Store the full game memory */
class SaveState {
    public:
        /* Meta data */
        uint64_t frame_count;

        /* Memory sections */
        int n_sections;
        uint64_t total_size;

        /* Vector of memory sections */
        std::vector<std::unique_ptr<StateSection>> sections;

        std::vector<ThreadInfo> threads;

        /* Access and save all memory regions of the game process that are writable. */
        void fillSections(pid_t game_pid);
        void fillRegisters(pid_t game_pid);
        bool save(pid_t game_pid);
        bool load(pid_t game_pid);

};

#endif
