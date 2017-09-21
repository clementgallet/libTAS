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

#include "ioctl_joy.h"
#include "../logging.h"
#include "../hook.h"
#include <linux/joystick.h>

namespace libtas {

namespace orig {
    static int (*ioctl)(int fd, unsigned long request, ...);
}

int ioctl(int fd, unsigned long request, ...)
{
//    debuglog(LCF_JOYSTICK, __func__, " call on device ", fd);
    LINK_NAMESPACE(ioctl, nullptr);

    va_list arg_list;
    void* argp;

    va_start(arg_list, request);
    argp = va_arg(arg_list, void*);
    va_end(arg_list);

    if (request == JSIOCGVERSION) {
        debuglog(LCF_JOYSTICK, "ioctl access to JSIOCGVERSION on fd ", fd);
        int* version = static_cast<int*>(argp);
        *version = 0x20100; // version 2.1.0
        return 0;
    }

    /* JSIOCGNAME(len) request has variable number depending on the buffer
     * length, so checking a match for both the type and nr
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(JSIOCGNAME(0)) &&
        _IOC_NR(request) == _IOC_NR(JSIOCGNAME(0))) {
        int len = _IOC_SIZE(request);
        char* name = static_cast<char*>(argp);
        debuglog(LCF_JOYSTICK, "ioctl access to JSIOCGNAME with len ", len, " on fd ", fd);
        strncpy(name, "Microsoft X-Box 360 pad", len);
        return 0;
    }

    if (request == JSIOCGBUTTONS) {
        debuglog(LCF_JOYSTICK, "ioctl access to JSIOCGBUTTONS on fd ", fd);
        int* buttons = static_cast<int*>(argp);
        *buttons = 11;
        return 0;
    }

    if (request == JSIOCGAXES) {
        debuglog(LCF_JOYSTICK, "ioctl access to JSIOCGAXES on fd ", fd);
        int* axes = static_cast<int*>(argp);
        *axes = 8;
        return 0;
    }

    if (request == JSIOCSCORR) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to JSIOCSCORR (not supported!) on fd ", fd);
    }

    if (request == JSIOCGCORR) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to JSIOCGCORR (not supported!) on fd ", fd);
    }

    return orig::ioctl(fd, request, argp);
}

}
