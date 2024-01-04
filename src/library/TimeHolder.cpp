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
 */

#include "TimeHolder.h"

#include <iostream>

namespace libtas {

void TimeHolder::shiftadd(TimeHolder& pow, int m)
{
    if (m == 0)
        return;
    if ((m & 0x01) != 0) {
        *this += pow;
        normalize();
    }
    pow.tv_sec <<= 1;
    pow.tv_nsec <<= 1;
    pow.normalize();
    return shiftadd(pow, m>>1);
}

void TimeHolder::normalize()
{
    if (this->tv_nsec < 0) {
        int sec = (-this->tv_nsec) / 1000000000 + 1;
        this->tv_nsec += 1000000000 * sec;
        this->tv_sec -= sec;
    }
    if (this->tv_nsec >= 1000000000) {
        int sec = this->tv_nsec / 1000000000;
        this->tv_nsec -= 1000000000 * sec;
        this->tv_sec += sec;
    }
}

}
