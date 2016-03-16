#include "TimeHolder.h"

TimeHolder TimeHolder::shiftadd(TimeHolder& pow, TimeHolder& mult, int m)
{
    if (m == 0)
        return mult;
    if ((m & 0x01) != 0) {
        mult = mult + pow;
        mult.normalize();
    }
    pow.tv_sec <<= 1;
    pow.tv_nsec <<= 1;
    pow.normalize();
    m >>= 1;
    return shiftadd(pow, mult, m);
}

void TimeHolder::normalize()
{
    if (this->tv_nsec < 0) {
        int sec = (-this->tv_nsec) / 1000000000 + 1;
        this->tv_nsec += 1000000000 * sec;
        this->tv_sec -= sec;
    } 
    if (this->tv_nsec > 1000000000) {
        int sec = this->tv_nsec / 1000000000;
        this->tv_nsec -= 1000000000 * sec;
        this->tv_sec += sec;
    } 
}

