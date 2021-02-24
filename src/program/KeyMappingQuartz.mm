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

#include "KeyMappingQuartz.h"
#include "../shared/SingleInput.h"
#include "../external/QuartzKeycodes.h"
#include "ramsearch/MemAccess.h"

#include <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#include <AppKit/NSEvent.h>

#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <array>

KeyMappingQuartz::KeyMappingQuartz(void* c) : KeyMapping(c)
{
    /* Fill hotkey list */
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_ANSI_P}, HOTKEY_PLAYPAUSE, "Play/Pause"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_ANSI_V}, HOTKEY_FRAMEADVANCE, "Frame Advance"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_Tab}, HOTKEY_FASTFORWARD, "Fast-forward"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_READWRITE, "Toggle Read/Write"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F1 | XK_Shift_Flag}, HOTKEY_SAVESTATE1, "Save State 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F2 | XK_Shift_Flag}, HOTKEY_SAVESTATE2, "Save State 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F3 | XK_Shift_Flag}, HOTKEY_SAVESTATE3, "Save State 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F4 | XK_Shift_Flag}, HOTKEY_SAVESTATE4, "Save State 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F5 | XK_Shift_Flag}, HOTKEY_SAVESTATE5, "Save State 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F6 | XK_Shift_Flag}, HOTKEY_SAVESTATE6, "Save State 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F7 | XK_Shift_Flag}, HOTKEY_SAVESTATE7, "Save State 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F8 | XK_Shift_Flag}, HOTKEY_SAVESTATE8, "Save State 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F9 | XK_Shift_Flag}, HOTKEY_SAVESTATE9, "Save State 9"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_SAVESTATE_BACKTRACK, "Save Backtrack State"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F1}, HOTKEY_LOADSTATE1, "Load State 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F2}, HOTKEY_LOADSTATE2, "Load State 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F3}, HOTKEY_LOADSTATE3, "Load State 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F4}, HOTKEY_LOADSTATE4, "Load State 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F5}, HOTKEY_LOADSTATE5, "Load State 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F6}, HOTKEY_LOADSTATE6, "Load State 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F7}, HOTKEY_LOADSTATE7, "Load State 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F8}, HOTKEY_LOADSTATE8, "Load State 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F9}, HOTKEY_LOADSTATE9, "Load State 9"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F10}, HOTKEY_LOADSTATE_BACKTRACK, "Load Backtrack State"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F1 | XK_Control_Flag}, HOTKEY_LOADBRANCH1, "Load Branch 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F2 | XK_Control_Flag}, HOTKEY_LOADBRANCH2, "Load Branch 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F3 | XK_Control_Flag}, HOTKEY_LOADBRANCH3, "Load Branch 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F4 | XK_Control_Flag}, HOTKEY_LOADBRANCH4, "Load Branch 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F5 | XK_Control_Flag}, HOTKEY_LOADBRANCH5, "Load Branch 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F6 | XK_Control_Flag}, HOTKEY_LOADBRANCH6, "Load Branch 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F7 | XK_Control_Flag}, HOTKEY_LOADBRANCH7, "Load Branch 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F8 | XK_Control_Flag}, HOTKEY_LOADBRANCH8, "Load Branch 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F9 | XK_Control_Flag}, HOTKEY_LOADBRANCH9, "Load Branch 9"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, kVK_F10 | XK_Control_Flag}, HOTKEY_LOADBRANCH_BACKTRACK, "Load Backtrack Branch"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_TOGGLE_ENCODE, "Toggle encode"});

    /* Get the keyboard layout to get the string representation of each keycode */
    /* Taken from https://stackoverflow.com/a/64344453 */
    
    for (int kc = 0; kc < 128; kc++) {
        /* Create dummy NSEvent from a CGEvent for a keypress */
        CGEventRef coreEvent = CGEventCreateKeyboardEvent(nullptr, kc, true);
        NSEvent* keyEvent = [NSEvent eventWithCGEvent:coreEvent];
        CFRelease(coreEvent);

        if (keyEvent.type == NSEventTypeKeyDown) {
            /* Get the character from the event */
            NSString* character = [keyEvent characters];
            
            /* If non-null string, insert the key in the list */
            if ([character length] > 0) {
                SingleInput si;
                si.type = SingleInput::IT_KEYBOARD;
                si.value = kc;
                si.description = [character cStringUsingEncoding:NSASCIIStringEncoding];
                input_list[INPUTLIST_KEYBOARD_LATIN].push_back(si);
            }
        }
    }    

    /* Reset keyboard state */
    memset(keyboard_state, 0, 16);

    /* Set default hotkeys */
    default_hotkeys();

    /* Set default inputs */
    default_inputs();
}

void KeyMappingQuartz::registerKeyDown(keysym_t ks)
{
    keyboard_state[ks >> 3] |= (1 << (ks & 0x7));
}

void KeyMappingQuartz::registerKeyUp(keysym_t ks)
{
    keyboard_state[ks >> 3] &= ~(1 << (ks & 0x7));
}

struct ModifierKey {
    keysym_t ks;
    keysym_t flag;
    std::string description;
};

static std::array<ModifierKey, 8> modifier_list {{
    {kVK_Shift, XK_Shift_Flag, "Shift"},
    {kVK_Shift, XK_Shift_Flag, "Shift"},
    {kVK_Control, XK_Control_Flag, "Control"},
    {kVK_Control, XK_Control_Flag, "Control"},
    {kVK_Command, XK_Meta_Flag, "Command"},
    {kVK_Command, XK_Meta_Flag, "Command"},
    {kVK_Option, XK_Alt_Flag, "Alt"},
    {kVK_Option, XK_Alt_Flag, "Alt"},
}};

bool KeyMappingQuartz::is_modifier(keysym_t ks)
{
    for (ModifierKey modifier : modifier_list) {
        if (modifier.ks == ks)
            return true;
    }
    return false;
}

keysym_t KeyMappingQuartz::get_modifiers(int flags)
{
    keysym_t modifiers = 0;
    if (flags & kCGEventFlagMaskShift)
        modifiers |= XK_Shift_Flag;
    if (flags & kCGEventFlagMaskControl)
        modifiers |= XK_Control_Flag;
    if (flags & kCGEventFlagMaskAlternate)
        modifiers |= XK_Alt_Flag;
    if (flags & kCGEventFlagMaskCommand)
        modifiers |= XK_Meta_Flag;
    
    return modifiers;
}

keysym_t KeyMappingQuartz::build_modifiers()
{    
    keysym_t modifiers = 0;
    
    /* Conveniently, we don't need an actual NSEvent to query the modifier keys */
    NSUInteger flags = [NSEvent modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;

    if (flags & NSEventModifierFlagShift)
        modifiers |= XK_Shift_Flag;
    if (flags & NSEventModifierFlagControl)
        modifiers |= XK_Control_Flag;
    if (flags & NSEventModifierFlagOption)
        modifiers |= XK_Alt_Flag;
    if (flags & NSEventModifierFlagCommand)
        modifiers |= XK_Meta_Flag;
    
    return modifiers;
}

std::string KeyMappingQuartz::input_description_mod(keysym_t ks)
{
    std::string str = "";
    
    for (ModifierKey modifier : modifier_list) {
        if (ks & modifier.flag) {
            str += modifier.description;
            str += "+";
            ks ^= modifier.flag;
        }
    }

    str += input_description(ks);
    return str;
}

void KeyMappingQuartz::default_inputs()
{
    input_mapping.clear();

    /* Map all keycode to their respective keysym. The other keysyms are unmapped. */
    for (int kc = 0; kc < 128; kc++) {
        for (auto iter : input_list[INPUTLIST_KEYBOARD_LATIN])
            if (iter.value == kc) {
                input_mapping[iter.value] = iter;
                break;
            }
    }
}

void KeyMappingQuartz::default_input(int tab, int input_index)
{
    /* Input selected */
    SingleInput si = input_list[tab][input_index];

    /* Remove previous mapping from this key */
    for (auto iter : input_mapping) {
        if (iter.second == si) {
            input_mapping.erase(iter.first);
            break;
        }
    }

    /* Check if there's a keycode mapped to this keysym */
    if (si.type == SingleInput::IT_KEYBOARD) {
        input_mapping[si.value] = si;
    }
}

void KeyMappingQuartz::buildAllInputs(AllInputs& ai, uint32_t window, SharedConfig& sc, bool mouse_warp){
    int i,j;
    int keysym_i = 0;

    ai.emptyInputs();

    /* Don't get inputs if the game window is closed */
    if (window == 0) {
        return;
    }

    keysym_t modifiers = build_modifiers();
    for (i=0; i<16; i++) {
        if (keyboard_state[i] == 0)
            continue;
        for (j=0; j<8; j++) {
            if ((keyboard_state[i] >> j) & 0x1) {

                /* We got a pressed keycode */
                keysym_t kc = (i << 3) | j;

                /* Check if we are dealing with a hotkey with or without modifiers */
                if (hotkey_mapping.find(kc) != hotkey_mapping.end()) {
                    /* Dealing with a hotkey, skipping */
                    continue;
                }

                if (modifiers) {
                    keysym_t ksm = kc | modifiers;
                    if (hotkey_mapping.find(ksm) != hotkey_mapping.end()) {
                        /* Dealing with a hotkey, skipping */
                        continue;
                    }
                }

                /* Checking the mapped input for that key */
                SingleInput si = {SingleInput::IT_NONE,0};
                if (input_mapping.find(kc) != input_mapping.end())
                    si = input_mapping[kc];

                if (si.type == SingleInput::IT_NONE) {
                    /* Key is mapped to nothing */
                    continue;
                }

                if (si.type == SingleInput::IT_KEYBOARD) {
                    /* Checking the current number of keys */
                    if (keysym_i >= AllInputs::MAXKEYS) {
                        fprintf(stderr, "Reached maximum number of inputs (%d).", AllInputs::MAXKEYS);
                        continue;
                    }

                    /* Saving the key */
                    ai.keyboard[keysym_i++] = si.value;
                }

                if (si.type == SingleInput::IT_FLAG) {
                    ai.flags |= (1 << si.value);
                }

                if (si.inputTypeIsController()) {
                    /* Key is mapped to a game controller */

                    /* Getting Controller id
                     * Arithmetic on enums is bad, no?
                     */
                    int controller_i = (si.type >> SingleInput::IT_CONTROLLER_ID_SHIFT) - 1;

                    /* Check if we support this joystick */
                    if (controller_i >= sc.nb_controllers)
                        continue;

                    int controller_axis = si.type & SingleInput::IT_CONTROLLER_AXIS_MASK;
                    int controller_type = si.type & SingleInput::IT_CONTROLLER_TYPE_MASK;
                    if (controller_axis) {
                        ai.controller_axes[controller_i][controller_type] = static_cast<short>(si.value);
                    }
                    else {
                        ai.controller_buttons[controller_i] |= (si.value & 0x1) << controller_type;
                    }
                }
            }
        }
    }

    if (sc.mouse_support) {
        /* Get the game window bounds */
        CGRect bounds;

        CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
        for (NSMutableDictionary* entry in (__bridge NSArray*)windowList)
        {
            NSInteger ownerPID = [[entry objectForKey:(id)kCGWindowOwnerPID] integerValue];
            if (ownerPID == MemAccess::getPid()) {
                CGRectMakeWithDictionaryRepresentation((CFDictionaryRef)[entry objectForKey:(id)kCGWindowBounds], &bounds);
                break;
            }
        }
        CFRelease(windowList);
        
        /* Get the pointer position and mask */
        const NSUInteger cocoaButtons = [NSEvent pressedMouseButtons];
        const NSPoint cocoaLocation = [NSEvent mouseLocation];

        ai.pointer_mode = sc.mouse_mode_relative?SingleInput::POINTER_MODE_RELATIVE:SingleInput::POINTER_MODE_ABSOLUTE;
        ai.pointer_x = static_cast<int>(cocoaLocation.x - bounds.origin.x);
        ai.pointer_y = static_cast<int>(cocoaLocation.y - bounds.origin.y);
        
        if (sc.mouse_mode_relative) {
            ai.pointer_x = static_cast<int>(cocoaLocation.x - bounds.size.width/2);
            ai.pointer_y = static_cast<int>(cocoaLocation.y - bounds.size.height/2);

            /* Warp pointer if needed */
            if (mouse_warp) {
                const CGPoint point = CGPointMake(bounds.size.width/2, bounds.size.height/2);
                CGWarpMouseCursorPosition(point);
                CGAssociateMouseAndMouseCursorPosition(YES);
            }
        }
        else {
            ai.pointer_x = static_cast<int>(cocoaLocation.x - bounds.size.width/2);
            ai.pointer_y = static_cast<int>(cocoaLocation.y - bounds.size.height/2);
        }

        /* We only care about the five mouse buttons */
        ai.pointer_mask = 0;
            
        if (cocoaButtons & (1 << 0))
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B1);
        if (cocoaButtons & (1 << 1))
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B2);
        if (cocoaButtons & (1 << 2))
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B3);
        if (cocoaButtons & (1 << 3))
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B4);
        if (cocoaButtons & (1 << 4))
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B5);
    }
}
