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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#ifndef LIBTAS_CHECKPOINT_H
#define LIBTAS_CHECKPOINT_H

#include <string>

namespace libtas {
namespace Checkpoint
{
    void setSavestatePath(std::string path);
    void setBaseSavestatePath(std::string path);

    void setSavestateIndex(int index);
    void setBaseSavestateIndex(int index);

    void setCurrentToParent();

    bool checkCheckpoint();
    bool checkRestore();
    void handler(int signum);
};
}

#endif
