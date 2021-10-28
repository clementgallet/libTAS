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
#include "../external/keysymdef.h"
#include "../external/QuartzKeycodes.h"
#include "../external/keysymdesc.h"
#include "ramsearch/MemAccess.h"

#include <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#include <AppKit/NSEvent.h>
#include <AppKit/NSText.h>

#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <array>

KeyMappingQuartz::KeyMappingQuartz(void* c) : KeyMapping(c)
{
    /* Get the keyboard layout and fill input list */
    initKeyboardLayout();
   
    /* Reset keyboard state */
    memset(keyboard_state, 0, 16);

    /* Set default hotkeys */
    default_hotkeys();

    /* Set default inputs */
    default_inputs();
}

void KeyMappingQuartz::registerKeyDown(uint16_t kc)
{
    keyboard_state[kc >> 3] |= (1 << (kc & 0x7));
}

void KeyMappingQuartz::registerKeyUp(uint16_t kc)
{
    keyboard_state[kc >> 3] &= ~(1 << (kc & 0x7));
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
            if (iter.value == keyboard_layout[kc]) {
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
        for (int kc = 0; kc < 128; kc++) {
            if (si.value == keyboard_layout[kc]) {
                input_mapping[si.value] = si;
                break;
            }
        }
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
                keysym_t ks = nativeToKeysym((i << 3) | j);

                /* Check if we are dealing with a hotkey with or without modifiers */
                if (hotkey_mapping.find(ks) != hotkey_mapping.end()) {
                    /* Dealing with a hotkey, skipping */
                    continue;
                }

                if (modifiers) {
                    keysym_t ksm = ks | modifiers;
                    if (hotkey_mapping.find(ksm) != hotkey_mapping.end()) {
                        /* Dealing with a hotkey, skipping */
                        continue;
                    }
                }

                /* Checking the mapped input for that key */
                SingleInput si = {SingleInput::IT_NONE,0};
                if (input_mapping.find(ks) != input_mapping.end())
                    si = input_mapping[ks];

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

                if (sc.mouse_support) {
                    if (si.type == SingleInput::IT_POINTER_B1)
                        ai.pointer_mask |= (0x1u << SingleInput::POINTER_B1);
                    if (si.type == SingleInput::IT_POINTER_B2)
                        ai.pointer_mask |= (0x1u << SingleInput::POINTER_B2);
                    if (si.type == SingleInput::IT_POINTER_B3)
                        ai.pointer_mask |= (0x1u << SingleInput::POINTER_B3);
                    if (si.type == SingleInput::IT_POINTER_B4)
                        ai.pointer_mask |= (0x1u << SingleInput::POINTER_B4);
                    if (si.type == SingleInput::IT_POINTER_B5)
                        ai.pointer_mask |= (0x1u << SingleInput::POINTER_B5);
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

struct key_translate {
    uint16_t unicode_char;
    keysym_t keysym;
};

static const key_translate char_to_xcb_keycode[59] = {
    { NSEnterCharacter, XK_Return },
    { NSBackspaceCharacter, XK_BackSpace },
    { NSTabCharacter, XK_Tab },
    { NSNewlineCharacter, XK_Linefeed },
    { NSCarriageReturnCharacter, XK_Return },
    { NSBackTabCharacter, XK_VoidSymbol },
    { 27, XK_Escape },
    { NSDeleteCharacter, XK_Delete },
    { NSUpArrowFunctionKey, XK_Up },
    { NSDownArrowFunctionKey, XK_Down },
    { NSLeftArrowFunctionKey, XK_Left },
    { NSRightArrowFunctionKey, XK_Right },
    { NSF1FunctionKey, XK_F1 },
    { NSF2FunctionKey, XK_F2 },
    { NSF3FunctionKey, XK_F3 },
    { NSF4FunctionKey, XK_F4 },
    { NSF5FunctionKey, XK_F5 },
    { NSF6FunctionKey, XK_F6 },
    { NSF7FunctionKey, XK_F7 },
    { NSF8FunctionKey, XK_F8 },
    { NSF9FunctionKey, XK_F9 },
    { NSF10FunctionKey, XK_F10 },
    { NSF11FunctionKey, XK_F11 },
    { NSF12FunctionKey, XK_F12 },
    { NSF13FunctionKey, XK_F13 },
    { NSF14FunctionKey, XK_F14 },
    { NSF15FunctionKey, XK_F15 },
    { NSF16FunctionKey, XK_F16 },
    { NSF17FunctionKey, XK_F17 },
    { NSF18FunctionKey, XK_F18 },
    { NSF19FunctionKey, XK_F19 },
    { NSF20FunctionKey, XK_F20 },
    { NSF21FunctionKey, XK_F21 },
    { NSF22FunctionKey, XK_F22 },
    { NSF23FunctionKey, XK_F23 },
    { NSF24FunctionKey, XK_F24 },
    { NSF25FunctionKey, XK_F25 },
    { NSF26FunctionKey, XK_F26 },
    { NSF27FunctionKey, XK_F27 },
    { NSF28FunctionKey, XK_F28 },
    { NSF29FunctionKey, XK_F29 },
    { NSF30FunctionKey, XK_F30 },
    { NSF31FunctionKey, XK_F31 },
    { NSF32FunctionKey, XK_F32 },
    { NSF33FunctionKey, XK_F33 },
    { NSF34FunctionKey, XK_F34 },
    { NSF35FunctionKey, XK_F35 },
    { NSInsertFunctionKey, XK_Insert },
    { NSDeleteFunctionKey, XK_Delete },
    { NSHomeFunctionKey, XK_Home },
    { NSEndFunctionKey, XK_End },
    { NSPageUpFunctionKey, XK_Page_Up },
    { NSPageDownFunctionKey, XK_Page_Down },
    { NSPrintScreenFunctionKey, XK_Print },
    { NSScrollLockFunctionKey, XK_Scroll_Lock },
    { NSPauseFunctionKey, XK_Pause },
    { NSSysReqFunctionKey, XK_Sys_Req },
    { NSMenuFunctionKey, XK_Menu },
    { NSHelpFunctionKey, XK_Help },
};

keysym_t KeyMappingQuartz::nativeToKeysym(int keycode)
{
    if (keycode < 0 || keycode >= 128)
        std::cerr << "Unknown native keycode " << keycode << std::endl;
    return keyboard_layout[keycode];
}

void KeyMappingQuartz::initKeyboardLayout()
{
    /* Get the keyboard layout to get the string representation of each keycode */
    /* Taken from https://stackoverflow.com/a/64344453 */
    
    for (int kc = 0; kc < 128; kc++) {
        keyboard_layout[kc] = 0;

        /* Create dummy NSEvent from a CGEvent for a keypress */
        CGEventRef coreEvent = CGEventCreateKeyboardEvent(nullptr, kc, true);
        NSEvent* keyEvent = [NSEvent eventWithCGEvent:coreEvent];
        CFRelease(coreEvent);

        if (keyEvent.type == NSEventTypeKeyDown) {
            /* Get the character from the event */
            NSString* character = [keyEvent characters];
            
            /* If non-null string, use the first character (UTF-16) as a
             * representation of the key. */
            if ([character length] > 0) {
                uint16_t ch = [character characterAtIndex:0];
                
                /* First check special (non-ASCII) characters */
                for (int t = 0; t < 59; t++) {
                    if (char_to_xcb_keycode[t].unicode_char == ch) {
                        keyboard_layout[kc] = char_to_xcb_keycode[t].keysym;

                        /* Check if we have a description string of the key */
                        const char* str = KEYSYM_TO_DESC_MISC(keyboard_layout[kc]);
                        if (str) {
                            /* Insert key in the misc list */
                            SingleInput si;
                            si.type = SingleInput::IT_KEYBOARD;
                            si.value = keyboard_layout[kc];
                            si.description = str;
                            input_list[INPUTLIST_KEYBOARD_MISC].push_back(si);
                        }
                        break;
                    }                    
                }

                /* If not found, then represent it as an ASCII character, which
                 * matches the first 256 values of xcb keysyms */
                if (keyboard_layout[kc] == 0) {
                    keyboard_layout[kc] = (ch & 0xff);
                    
                    /* Check if we have a description string of the key */
                    const char* str = KEYSYM_TO_DESC_LATIN(keyboard_layout[kc]);
                    if (str) {
                        /* Insert key in the ascii list */
                        SingleInput si;
                        si.type = SingleInput::IT_KEYBOARD;
                        si.value = keyboard_layout[kc];
                        si.description = str;
                        input_list[INPUTLIST_KEYBOARD_LATIN].push_back(si);
                    }
                }
            }
        }
    }
    
    /* Add modifiers using the left keys for translation */
    keyboard_layout[kVK_Shift] = XK_Shift_L;
    keyboard_layout[kVK_Control] = XK_Control_L;
    keyboard_layout[kVK_Command] = XK_Meta_L;
    keyboard_layout[kVK_Option] = XK_Alt_L;
}
