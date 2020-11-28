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

#include "ioctl_joy.h"
#include "../logging.h"
#include "../hook.h"
#include "evdev.h" // get_ev_number
#include "jsdev.h" // get_js_number
#include "inputs.h" // game_ai
#include "../../shared/SingleInput.h"
#include <linux/joystick.h>
#include <linux/input.h>
#include <cstdarg>
#include <cstring>

#define CHECK_LEN_AND_SET_BIT(bit, bits, len) do {\
    if (bit < (len*8))\
        bits[bit/8] |= 1 << (bit % 8); \
    } while (false)


namespace libtas {

DEFINE_ORIG_POINTER(ioctl)

int ioctl(int fd, unsigned long request, ...) throw()
{
    LINK_NAMESPACE_GLOBAL(ioctl);

    va_list arg_list;
    void* argp;

    va_start(arg_list, request);
    argp = va_arg(arg_list, void*);
    va_end(arg_list);

    if (GlobalState::isNative())
        return orig::ioctl(fd, request, argp);

    debuglog(LCF_JOYSTICK, __func__, " call on device ", fd);

    if (fd < 0) {
        errno = EBADF;
        return -1;
    }

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
        char* buttons = static_cast<char*>(argp);
        *buttons = 11;
        return 0;
    }

    if (request == JSIOCGAXES) {
        debuglog(LCF_JOYSTICK, "ioctl access to JSIOCGAXES on fd ", fd);
        char* axes = static_cast<char*>(argp);
        *axes = 8;
        return 0;
    }

    if (request == JSIOCSCORR) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to JSIOCSCORR (not supported!) on fd ", fd);
        return 0;
    }

    if (request == JSIOCGCORR) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to JSIOCGCORR (not supported!) on fd ", fd);
        return 0;
    }

    if (request == EVIOCGVERSION) {
        debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGVERSION on fd ", fd);
        int* version = static_cast<int*>(argp);
        *version = 0x10001;
        return 0;

        // int ret = orig::ioctl(fd, request, argp);
        // int* version = static_cast<int*>(argp);
        // debuglog(LCF_JOYSTICK, "    return version ", *version);
        // return ret;
    }

    if (request == EVIOCGID) {
        debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGID on fd ", fd);

        /* Returning a custom id, taken from my XBOX360 controller */
        struct input_id* id = static_cast<struct input_id*>(argp);
        id->bustype = 3;
        id->vendor = 0x45e;
        id->product = 0x28e;
        id->version = 0x114;
        return 0;

        // int ret = orig::ioctl(fd, request, argp);
        // struct input_id* id = static_cast<struct input_id*>(argp);
        // debuglog(LCF_JOYSTICK, "    return bustype ", id->bustype, ", vendor ", id->vendor, ", product ", id->product, ", version ", id->version);
        // return ret;
    }

    /* EVIOCGNAME(len) request has variable number depending on the buffer
     * length, so checking a match for both the type and nr
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(EVIOCGNAME(0)) &&
        _IOC_NR(request) == _IOC_NR(EVIOCGNAME(0))) {
        int len = _IOC_SIZE(request);
        char* name = static_cast<char*>(argp);
        debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGNAME with len ", len, " on fd ", fd);
        strncpy(name, "Microsoft X-Box 360 pad", len);
        return strnlen(name, len);

        // debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGNAME with len ", len, " on fd ", fd);
        // int ret = orig::ioctl(fd, request, argp);
        // char* name = static_cast<char*>(argp);
        // debuglog(LCF_JOYSTICK, "    return name ", name);
        // return ret;
    }

    /* EVIOCGPHYS(len) request has variable number depending on the buffer
     * length, so checking a match for both the type and nr
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(EVIOCGPHYS(0)) &&
        _IOC_NR(request) == _IOC_NR(EVIOCGPHYS(0))) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCGPHYS (not supported) on fd ", fd);
        return 0;
        //int len = _IOC_SIZE(request);
        // int ret = orig::ioctl(fd, request, argp);
        // char* name = static_cast<char*>(argp);
        // debuglog(LCF_JOYSTICK, "    return phys ", name);
        // return ret;
    }

    /* EVIOCGUNIQ(len) request has variable number depending on the buffer
     * length, so checking a match for both the type and nr
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(EVIOCGUNIQ(0)) &&
        _IOC_NR(request) == _IOC_NR(EVIOCGUNIQ(0))) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCGUNIQ (not supported) on fd ", fd);
        return 0;
        //int len = _IOC_SIZE(request);
        // int ret = orig::ioctl(fd, request, argp);
        // char* name = static_cast<char*>(argp);
        // debuglog(LCF_JOYSTICK, "    return uniq ", name);
        // return ret;
    }

    /* EVIOCGPROP(len) request has variable number depending on the buffer
     * length, so checking a match for both the type and nr
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(EVIOCGPROP(0)) &&
        _IOC_NR(request) == _IOC_NR(EVIOCGPROP(0))) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCGPROP (not supported) on fd ", fd);
        return 0;
        //int len = _IOC_SIZE(request);
        // int ret = orig::ioctl(fd, request, argp);
        // char* name = static_cast<char*>(argp);
        // debuglog(LCF_JOYSTICK, "    return prop ", name);
        // return ret;
    }

    /* EVIOCGKEY(len) request has variable number depending on the buffer
     * length, so checking a match for type
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(EVIOCGKEY(0)) &&
        _IOC_NR(request) == _IOC_NR(EVIOCGKEY(0))) {
        debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGKEY on fd ", fd);

        /* Get the joystick number from the file descriptor */
        int jsnum = get_ev_number(fd);
        if (jsnum < 0)
            jsnum = get_js_number(fd);
        if (jsnum < 0) {
            errno = ENOTTY;
            return -1;
        }

        /* Get the buttons state */
        unsigned short buttons = game_ai.controller_buttons[jsnum];

        /* Set the corresponding bit in the key state */
        int len = _IOC_SIZE(request);
        uint8_t* bits = static_cast<uint8_t*>(argp);

        for (int bi=0; bi<11; bi++) {
            if (buttons & (1 << bi)) CHECK_LEN_AND_SET_BIT(SingleInput::toEvdevButton(bi), bits, len);
        }
        return 0;
        // int len = _IOC_SIZE(request);
        // int ret = orig::ioctl(fd, request, argp);
        // uint32_t* bits = static_cast<uint32_t*>(argp);
        // for (int i=0; i<len/4; i++)
        //     debuglog(LCF_JOYSTICK, "    return bits ", (void*)bits[i]);
        // return ret;
    }

    /* EVIOCGBIT(ev,len) request has variable number depending on the buffer
     * length and event type, so checking a match for type
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(EVIOCGBIT(0,0))) {
        if (_IOC_NR(request) >= _IOC_NR(EVIOCGBIT(EV_SYN,0)) &&
            _IOC_NR(request) < _IOC_NR(EVIOCGBIT(EV_MAX,0)) ) {

            int len = _IOC_SIZE(request);
            memset(argp, 0, len);

            uint8_t* bits = static_cast<uint8_t*>(argp);

            if (_IOC_NR(request) == _IOC_NR(EVIOCGBIT(EV_SYN,0))) {
                debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGBIT for event EV_SYN on fd ", fd);
                CHECK_LEN_AND_SET_BIT(EV_SYN, bits, len);
                CHECK_LEN_AND_SET_BIT(EV_KEY, bits, len);
                CHECK_LEN_AND_SET_BIT(EV_ABS, bits, len);
                /* XBOX360 pad has force-feedback, but we don't really want the
                 * game to write on the dev file, so not enabling it.
                 */
                // CHECK_LEN_AND_SET_BIT(EV_FF, bits, len);
                return 0;
            }

            if (_IOC_NR(request) == _IOC_NR(EVIOCGBIT(EV_KEY,0))) {
                debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGBIT for event EV_KEY on fd ", fd);
                for (int bi=0; bi<11; bi++) {
                    CHECK_LEN_AND_SET_BIT(SingleInput::toEvdevButton(bi), bits, len);
                }
                return 0;
            }

            if (_IOC_NR(request) == _IOC_NR(EVIOCGBIT(EV_ABS,0))) {
                debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGBIT for event EV_ABS on fd ", fd);
                for (int axi=0; axi<AllInputs::MAXAXES; axi++) {
                    CHECK_LEN_AND_SET_BIT(SingleInput::toEvdevAxis(axi), bits, len);
                }
                /* Add the two hat axes because they are not considered as buttons */
                CHECK_LEN_AND_SET_BIT(ABS_HAT0X, bits, len);
                CHECK_LEN_AND_SET_BIT(ABS_HAT0Y, bits, len);
                return 0;
            }

            debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGBIT for event ", _IOC_NR(request), " on fd ", fd);
            return 0;
        }
    }

    /* EVIOCGABS(abs) request has variable number depending on the axis id,
     * so checking a match for type.
     */
    if (_IOC_TYPE(request) == _IOC_TYPE(EVIOCGABS(0))) {
        if (_IOC_NR(request) >= _IOC_NR(EVIOCGABS(ABS_X)) &&
            _IOC_NR(request) < _IOC_NR(EVIOCGABS(ABS_MAX)) ) {

            debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGABS for axis ", _IOC_NR(request), " on fd ", fd);
            struct input_absinfo* absinfo = static_cast<struct input_absinfo*>(argp);

            /* Write the axis parameters */
            switch (_IOC_NR(request)) {
                case _IOC_NR(EVIOCGABS(ABS_X)):
                case _IOC_NR(EVIOCGABS(ABS_Y)):
                case _IOC_NR(EVIOCGABS(ABS_RX)):
                case _IOC_NR(EVIOCGABS(ABS_RY)):
                    absinfo->minimum = -32768;
                    absinfo->maximum = 32767;
                    absinfo->fuzz = 16;
                    absinfo->flat = 128;
                    absinfo->resolution = 0;
                    break;
                case _IOC_NR(EVIOCGABS(ABS_Z)):
                case _IOC_NR(EVIOCGABS(ABS_RZ)):
                    absinfo->minimum = 0;
                    absinfo->maximum = 255;
                    absinfo->fuzz = 0;
                    absinfo->flat = 0;
                    absinfo->resolution = 0;
                    break;
                case _IOC_NR(EVIOCGABS(ABS_HAT0X)):
                case _IOC_NR(EVIOCGABS(ABS_HAT0Y)):
                    absinfo->minimum = -1;
                    absinfo->maximum = 1;
                    absinfo->fuzz = 0;
                    absinfo->flat = 0;
                    absinfo->resolution = 0;
                    break;
                default:
                    errno = ENOTTY;
                    return -1;
            }

            /* Get the joystick number from the file descriptor */
            int jsnum = get_ev_number(fd);
            if (jsnum < 0)
                jsnum = get_js_number(fd);
            if (jsnum < 0) {
                debuglog(LCF_JOYSTICK, "   joystick not found!");
                errno = ENOTTY;
                return -1;
            }

            /* Get the axes and buttons state */
            std::array<short, AllInputs::MAXAXES> axes = game_ai.controller_axes[jsnum];
            unsigned short buttons = game_ai.controller_buttons[jsnum];

            /* Write the axis value */
            switch (_IOC_NR(request)) {
                case _IOC_NR(EVIOCGABS(ABS_X)):
                    absinfo->value = axes[SingleInput::AXIS_LEFTX];
                    return 0;
                case _IOC_NR(EVIOCGABS(ABS_Y)):
                    absinfo->value = axes[SingleInput::AXIS_LEFTY];
                    return 0;
                case _IOC_NR(EVIOCGABS(ABS_RX)):
                    absinfo->value = axes[SingleInput::AXIS_RIGHTX];
                    return 0;
                case _IOC_NR(EVIOCGABS(ABS_RY)):
                    absinfo->value = axes[SingleInput::AXIS_RIGHTY];
                    return 0;
                /* TODO: Clamp values ! */
                case _IOC_NR(EVIOCGABS(ABS_Z)):
                    absinfo->value = axes[SingleInput::AXIS_TRIGGERLEFT];
                    return 0;
                case _IOC_NR(EVIOCGABS(ABS_RZ)):
                    absinfo->value = axes[SingleInput::AXIS_TRIGGERRIGHT];
                    return 0;
                case _IOC_NR(EVIOCGABS(ABS_HAT0X)):
                    absinfo->value = SingleInput::toDevHatX(buttons);
                    return 0;
                case _IOC_NR(EVIOCGABS(ABS_HAT0Y)):
                    absinfo->value = SingleInput::toDevHatY(buttons);
                    return 0;
                default:
                    errno = ENOTTY;
                    return -1;
            }
        }
    }

    if (request == EVIOCGREP) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCGREP (not supported!) on fd ", fd);
        return 0;
    }

    if (request == EVIOCSREP) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCSREP (not supported!) on fd ", fd);
        return 0;
    }

    if (request == EVIOCGKEYCODE) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCGKEYCODE (not supported!) on fd ", fd);
        return 0;
    }

    if (request == EVIOCGKEYCODE_V2) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCGKEYCODE_V2 (not supported!) on fd ", fd);
        return 0;
    }

    if (request == EVIOCSKEYCODE) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCSKEYCODE (not supported!) on fd ", fd);
        return 0;
    }

    if (request == EVIOCSKEYCODE_V2) {
        debuglog(LCF_JOYSTICK | LCF_TODO, "ioctl access to EVIOCSKEYCODE_V2 (not supported!) on fd ", fd);
        return 0;
    }

    if (request == EVIOCSFF) {
        debuglog(LCF_JOYSTICK, "ioctl write with EVIOCSFF on fd ", fd);
        return 0;
    }

    if (request == EVIOCRMFF) {
        debuglog(LCF_JOYSTICK, "ioctl write with EVIOCSFF on fd ", fd);
        return 0;
    }

    if (request == EVIOCGEFFECTS) {
        debuglog(LCF_JOYSTICK, "ioctl access to EVIOCGEFFECTS on fd ", fd);
        int* ne = static_cast<int*>(argp);
        *ne = 0;
        return 0;
    }

    if (request == EVIOCGRAB) {
        debuglog(LCF_JOYSTICK, "ioctl write with EVIOCGRAB on fd ", fd);
        return 0;
    }

    if (request == EVIOCREVOKE) {
        debuglog(LCF_JOYSTICK, "ioctl write with EVIOCREVOKE on fd ", fd);
        return 0;
    }

    return orig::ioctl(fd, request, argp);
}

}
