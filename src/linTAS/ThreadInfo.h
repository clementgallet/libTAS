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

#ifndef LINTAS_THREADINFO_H_INCLUDED
#define LINTAS_THREADINFO_H_INCLUDED

#include <sys/types.h>
#include <sys/user.h>

/* Store a section of the game memory */
class ThreadInfo {
    public:
        pid_t tid;
        struct user_regs_struct regs;
        bool needattach;

        void saveRegisters(void);
        void loadRegisters(void);
};

#endif
