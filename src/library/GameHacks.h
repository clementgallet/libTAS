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

#ifndef LIBTAS_GAMEHACKS_H_INCLUDED
#define LIBTAS_GAMEHACKS_H_INCLUDED

#include <sys/types.h>
#include <cstddef>

namespace libtas {

class GameHacks
{
    public:
        static void setUnity();

        static bool isUnity();
        
        static bool isUnityLoadingThread(std::ptrdiff_t routine_id);

        /* Regsiter that the game linked `libcoreclr.so` library */
        static void setCoreclr();
    
        /* Returns if the game linked `libcoreclr.so` library */
        static bool hasCoreclr();

        /* Regsiter the pid of the .NET finalizer thread */
        static void setFinalizerThread(pid_t pid);

        /* Get the pid of the .NET finalizer thread, or 0 */
        static pid_t getFinalizerThread();
        
    private:
        static bool unity;
        static bool coreclr;
        static pid_t finalizer_pid;
};

}

#endif
