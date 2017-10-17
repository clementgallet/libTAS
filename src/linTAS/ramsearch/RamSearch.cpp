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

#include "RamSearch.h"
// #include "CompareEnums.h"
// #include <sys/types.h>
// #include "MemSection.h"
// #include <sstream>
// #include <fstream>

// #include "../shared/AllInputs.h"
// #include "../shared/SharedConfig.h"
// #include <X11/Xlib.h>
// #include <X11/keysym.h>
// #include <map>
// #include <vector>
// #include <array>
// #include <forward_list>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <iostream>

void RamSearch::attach() {
    /* Try to attach to the game process */
    if (ptrace(PTRACE_ATTACH, game_pid, nullptr, nullptr) != 0)
    {
        /* if ptrace() gives EPERM, it might be because another process is already attached */
        if (errno == EPERM)
        {
            std::cerr << "Process is currently attached" << std::endl;
        }
        return;
    }

    int status = 0;
    pid_t waitret = waitpid(game_pid, &status, 0);
    if (waitret != game_pid)
    {
        std::cerr << "Function waitpid failed" << std::endl;
        return;
    }
    if (!WIFSTOPPED(status))
    {
        std::cerr << "Unhandled status change: " << status << std::endl;
        return;
    }
    if (WSTOPSIG(status) != SIGSTOP)
    {
        std::cerr << "Wrong stop signal: " << WSTOPSIG(status) << std::endl;
        return;
    }
}

void RamSearch::detach()
{
    ptrace(PTRACE_DETACH, game_pid, nullptr, nullptr);
}
