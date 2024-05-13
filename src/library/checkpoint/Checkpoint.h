/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#ifndef LIBTAS_CHECKPOINT_H
#define LIBTAS_CHECKPOINT_H

#include <string>
#include <signal.h> // siginfo_t

namespace libtas {
    
struct StateHeader;

namespace Checkpoint
{
    void setSavestatePath(std::string path);
    void setBaseSavestatePath(std::string path);

    void setSavestateIndex(int index);
    void setBaseSavestateIndex(int index);

    void setCurrentToParent();

    void getStateHeader(StateHeader* sh);
    int checkCheckpoint();
    int checkRestore();
    void handler(int signum, siginfo_t *info, void *ucontext);
}
}

#endif
