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

#include "xinput.h"

#include "hook.h"
#include "logging.h"
#include "global.h"
#include "GlobalState.h"

namespace libtas {

int xinput_opcode;

int XISelectEvents(Display* dpy, Window win, XIEventMask *masks, int num_masks)
{
    RETURN_IF_NATIVE(XISelectEvents, (dpy, win, masks, num_masks), "libXi.so.6");

    LOGTRACE(LCF_WINDOW);

    /* Only check if not using SDL */
    if (masks) {
        if (!(Global::game_info.keyboard & (GameInfo::SDL1 | GameInfo::SDL2))) {
            if ((masks->mask_len >= XIMaskLen(XI_KeyPress)) && XIMaskIsSet(masks->mask, XI_KeyPress)) {
                LOG(LL_DEBUG, LCF_KEYBOARD, "   selecting XI keyboard events");
                Global::game_info.keyboard |= GameInfo::XIEVENTS;
                Global::game_info.tosend = true;
            }
            if ((masks->mask_len >= XIMaskLen(XI_RawKeyPress)) && XIMaskIsSet(masks->mask, XI_RawKeyPress)) {
                LOG(LL_DEBUG, LCF_KEYBOARD, "   selecting XI raw keyboard events");
                Global::game_info.keyboard |= GameInfo::XIRAWEVENTS;
                Global::game_info.tosend = true;
            }
        }

        if (!(Global::game_info.mouse & (GameInfo::SDL1 | GameInfo::SDL2))) {
            if (((masks->mask_len >= XIMaskLen(XI_Motion)) && XIMaskIsSet(masks->mask, XI_Motion)) ||
                ((masks->mask_len >= XIMaskLen(XI_ButtonPress)) && XIMaskIsSet(masks->mask, XI_ButtonPress))) {
                LOG(LL_DEBUG, LCF_MOUSE, "   selecting XI mouse events");
                Global::game_info.mouse |= GameInfo::XIEVENTS;
                Global::game_info.tosend = true;
            }
            if (((masks->mask_len >= XIMaskLen(XI_RawMotion)) && XIMaskIsSet(masks->mask, XI_RawMotion)) ||
                ((masks->mask_len >= XIMaskLen(XI_RawButtonPress)) && XIMaskIsSet(masks->mask, XI_RawButtonPress))) {
                LOG(LL_DEBUG, LCF_MOUSE, "   selecting XI raw mouse events");
                Global::game_info.mouse |= GameInfo::XIRAWEVENTS;
                Global::game_info.tosend = true;
            }
        }
    }

    RETURN_NATIVE(XISelectEvents, (dpy, win, masks, num_masks), "libXi.so.6");
}

XIEventMask *XIGetSelectedEvents( Display *display, Window win, int *num_masks_return)
{
    RETURN_IF_NATIVE(XIGetSelectedEvents, (display, win, num_masks_return), "libXi.so.6");

    LOGTRACE(LCF_WINDOW);

    RETURN_NATIVE(XIGetSelectedEvents, (display, win, num_masks_return), "libXi.so.6");
}

XIDeviceInfo* XIQueryDevice(Display* dpy, int deviceid, int* ndevices_return)
{
    LOGTRACE(LCF_WINDOW);
    
    int device_count = 0;
    switch (deviceid) {
        case XIAllDevices:
        case XIAllMasterDevices:
            device_count = 2;
            break;
        case 2:
        case 3:
            device_count = 1;
            break;
        default:
            device_count = 0;
            break;
    }

    *ndevices_return = device_count;

    if (device_count == 0) {
        /* TODO: Generate BadDevice error */
        return nullptr;
    }

    XIDeviceInfo* info = new XIDeviceInfo[device_count+1];
    int d = 0;

    if (deviceid != 3) {
        /* Build pointer device info. Values come mostly from my mouse, while
        * keeping only the necessary number of inputs. */
        info[d].deviceid = 2;
        info[d].name = "Virtual core pointer";
        info[d].use = XIMasterPointer;
        info[d].attachment = 3;
        info[d].enabled = True;
        info[d].num_classes = 3;
        info[d].classes = new XIAnyClassInfo*[info[d].num_classes];
        
        XIButtonClassInfo* bc = new XIButtonClassInfo;
        info[d].classes[0] = reinterpret_cast<XIAnyClassInfo*>(bc);
        bc->type = XIButtonClass;
        bc->sourceid = info[d].deviceid;
        bc->num_buttons = 5;
        bc->labels = new Atom[bc->num_buttons];
        NATIVECALL(bc->labels[0] = XInternAtom(dpy, "Button Left", False));
        NATIVECALL(bc->labels[1] = XInternAtom(dpy, "Button Middle", False));
        NATIVECALL(bc->labels[2] = XInternAtom(dpy, "Button Right", False));
        NATIVECALL(bc->labels[3] = XInternAtom(dpy, "Button Wheel Up", False));
        NATIVECALL(bc->labels[4] = XInternAtom(dpy, "Button Wheel Down", False));
        bc->state.mask_len = 0;
        bc->state.mask = nullptr;
        
        XIValuatorClassInfo* v1 = new XIValuatorClassInfo;
        info[d].classes[1] = reinterpret_cast<XIAnyClassInfo*>(v1);
        v1->type = XIValuatorClass;
        v1->sourceid = info[d].deviceid;
        v1->number = 0;
        NATIVECALL(v1->label = XInternAtom(dpy, "Rel X", False));
        v1->min = -1;
        v1->max = -1;
        v1->value = -1;
        v1->resolution = 0;
        v1->mode = XIModeRelative;
        
        XIValuatorClassInfo* v2 = new XIValuatorClassInfo;
        info[d].classes[2] = reinterpret_cast<XIAnyClassInfo*>(v2);
        v2->type = XIValuatorClass;
        v2->sourceid = info[d].deviceid;
        v2->number = 0;
        NATIVECALL(v2->label = XInternAtom(dpy, "Rel Y", False));
        v2->min = -1;
        v2->max = -1;
        v2->value = -1;
        v2->resolution = 0;
        v2->mode = XIModeRelative;
        
        d++;
    }

    if (deviceid != 3) {
        /* Build keyboard device info. */
        info[d].deviceid = 3;
        info[d].name = "Virtual core keyboard";
        info[d].use = XIMasterKeyboard;
        info[d].attachment = 2;
        info[d].enabled = True;
        info[d].num_classes = 1;
        info[d].classes = new XIAnyClassInfo*[info[d].num_classes];
        
        XIKeyClassInfo* kc = new XIKeyClassInfo;
        info[d].classes[0] = reinterpret_cast<XIAnyClassInfo*>(kc);
        kc->type = XIKeyClass;
        kc->sourceid = info[d].deviceid;
        kc->num_keycodes = 248;
        kc->keycodes = new int[kc->num_keycodes];
        for (int i=0; i<kc->num_keycodes; i++) {
            kc->keycodes[i] = i+8;
        }
        
        d++;
    }
    
    /* Last dummy item to indicate the end of the list for XIFreeDeviceInfo() */
    info[d].deviceid = 0;

    return info;
}

void XIFreeDeviceInfo( XIDeviceInfo *info)
{
    LOGTRACE(LCF_WINDOW);
    
    if (!info)
        return;
        
    for (int d = 0; info[d].deviceid != 0; d++) {
        for (int c = 0; c < info[d].num_classes; c++) {
            switch (info[d].classes[c]->type) {
                case XIButtonClass: {
                    XIButtonClassInfo* bc = reinterpret_cast<XIButtonClassInfo*>(info[d].classes[c]);
                    delete bc->labels;
                    delete bc;
                    break;
                }
                case XIValuatorClass: {
                    XIValuatorClassInfo* vc = reinterpret_cast<XIValuatorClassInfo*>(info[d].classes[c]);
                    delete vc;
                    break;
                }
                case XIKeyClass: {
                    XIKeyClassInfo* kc = reinterpret_cast<XIKeyClassInfo*>(info[d].classes[c]);
                    delete kc->keycodes;
                    delete kc;
                    break;
                }                
            }
        }
        delete info[d].classes;
    }
    delete info;    
}

}
