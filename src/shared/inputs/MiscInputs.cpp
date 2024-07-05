/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "MiscInputs.h"

#include <iostream>

MiscInputs& MiscInputs::operator|=(const MiscInputs& mi)
{
    flags |= mi.flags;
    framerate_den |= mi.framerate_den;
    framerate_num |= mi.framerate_num;
    realtime_sec |= mi.realtime_sec;
    realtime_nsec |= mi.realtime_nsec;
    return *this;
}

MiscInputs& MiscInputs::operator=(const MiscInputs& mi)
{
    flags = mi.flags;
    framerate_den = mi.framerate_den;
    framerate_num = mi.framerate_num;
    realtime_sec = mi.realtime_sec;
    realtime_nsec = mi.realtime_nsec;    
    return *this;
}

void MiscInputs::clear() {
    flags = 0;
    framerate_den = 0;
    framerate_num = 0;
    realtime_sec = 0;
    realtime_nsec = 0;
}

int MiscInputs::getInput(const SingleInput &si) const
{
    switch (si.type) {
        /* Flag inputs */
        case SingleInput::IT_FLAG:
            return (flags >> si.value) & 0x1;

        /* Framerate inputs */
        case SingleInput::IT_FRAMERATE_NUM:
            return framerate_num;
        case SingleInput::IT_FRAMERATE_DEN:
            return framerate_den;

        /* Realtime inputs */
        case SingleInput::IT_REALTIME_SEC:
            return realtime_sec;
        case SingleInput::IT_REALTIME_NSEC:
            return realtime_nsec;
    }
    return 0;
}

void MiscInputs::setInput(const SingleInput &si, int value)
{
    switch (si.type) {
        /* Flag input */
        case SingleInput::IT_FLAG:
            if (value)
                flags |= (0x1 << si.value);
            else
                flags &= ~(0x1 << si.value);
            break;

        /* Framerate inputs */
        case SingleInput::IT_FRAMERATE_NUM:
            framerate_num = value;
            break;
        case SingleInput::IT_FRAMERATE_DEN:
            framerate_den = value;
            break;

        /* Realtime inputs */
        case SingleInput::IT_REALTIME_SEC:
            realtime_sec = value;
            break;
        case SingleInput::IT_REALTIME_NSEC:
            realtime_nsec = value;
            break;
    }
}

void MiscInputs::extractInputs(std::set<SingleInput> &input_set) const
{
    SingleInput si;

    if (flags) {
        uint32_t temp_flags = flags;
        for (unsigned int i=0; temp_flags!=0; i++, temp_flags >>= 1) {
            if (temp_flags & 0x1) {
                si = {SingleInput::IT_FLAG, i, ""};
                input_set.insert(si);
            }
        }
    }

    if (framerate_num) {
        si = {SingleInput::IT_FRAMERATE_NUM, 1, ""};
        input_set.insert(si);
    }
    if (framerate_den) {
        si = {SingleInput::IT_FRAMERATE_DEN, 1, ""};
        input_set.insert(si);
    }

    if (realtime_sec) {
        si = {SingleInput::IT_REALTIME_SEC, 1, ""};
        input_set.insert(si);
        si = {SingleInput::IT_REALTIME_NSEC, 1, ""};
        input_set.insert(si);
    }
}
