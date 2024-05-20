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

#ifndef LIBTAS_TIMEHOLDER_H_INCL
#define LIBTAS_TIMEHOLDER_H_INCL

#include <time.h>

/* Class containing a time value stored as two fields:
 * - tv_sec: number of seconds
 * - tv_nsec: number of nanoseconds
 * Those fields are inherited from the timespec struct, which allows
 * casting.
 */

namespace libtas {
class TimeHolder : public timespec
{
    public:

        TimeHolder() {this->tv_sec = 0; this->tv_nsec = 0;};
        TimeHolder(time_t s, long ns) {this->tv_sec = s; this->tv_nsec = ns;};
        
        TimeHolder(const timespec& th)
        {
            this->tv_sec = th.tv_sec;
            this->tv_nsec = th.tv_nsec;
        }

        bool operator!() const
        {
            return this->tv_sec == 0 && this->tv_nsec == 0;
        }

        bool operator!=(const timespec& th) const
        {
            return ((this->tv_sec != th.tv_sec) || (this->tv_nsec != th.tv_nsec));
        }

        TimeHolder operator+(const timespec& th) 
        {
            TimeHolder sum;
            sum.tv_sec = this->tv_sec + th.tv_sec;
            sum.tv_nsec = this->tv_nsec + th.tv_nsec;
            sum.normalize();
            return sum;
        }

        TimeHolder& operator+=(const timespec& th)
        {
            this->tv_sec += th.tv_sec;
            this->tv_nsec += th.tv_nsec;
            this->normalize();
            return *this;
        }

        TimeHolder& operator-=(const timespec& th)
        {
            this->tv_sec -= th.tv_sec;
            this->tv_nsec -= th.tv_nsec;
            this->normalize();
            return *this;
        }

        TimeHolder operator-(const timespec& th) const
        {
            TimeHolder diff;
            diff.tv_sec = this->tv_sec - th.tv_sec;
            diff.tv_nsec = this->tv_nsec - th.tv_nsec;
            diff.normalize();
            return diff;
        }

        TimeHolder operator*(const int& m) const
        {
            TimeHolder orig = *this;
            TimeHolder mult;
            mult.tv_sec = 0;
            mult.tv_nsec = 0;

            mult.shiftadd(orig, m);
            return mult;
        }

        bool operator>(const timespec& th ) const
        {
            return ((this->tv_sec > th.tv_sec) || ((this->tv_sec == th.tv_sec) && (this->tv_nsec > th.tv_nsec)));
        }

        bool operator<(const timespec& th ) const
        {
            return ((this->tv_sec < th.tv_sec) || ((this->tv_sec == th.tv_sec) && (this->tv_nsec < th.tv_nsec)));
        }

        /* Use a shift and add algorithm for multiplying a TimeHolder
         * by an integer, so that we should never overflow the tv_nsec value
         */
        void shiftadd(TimeHolder& pow, int m);

        /* Bring the tv_nsec value inside the range [0,999999999] */
        void normalize();
};
}

#endif
