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

#include "Context.h"
#include "GameEventsQuartz.h"
#include "GameEvents.h"
#include "movie/MovieFile.h"
#include "../external/QuartzKeycodes.h"

#include <CoreGraphics/CoreGraphics.h>

#include <string>
#include <iostream>
#include <cerrno>
#include <unistd.h> // usleep()
#include <stdint.h>

NSRunningApplication* GameEventsQuartz::gameApp = nullptr;

GameEventsQuartz::GameEventsQuartz(Context* c, MovieFile* m) : GameEvents(c, m) {
    eventTap = nullptr;
}

static CGEventRef eventTapFunction(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {

    Context* context = static_cast<Context*>(refcon);

    if ((type == kCGEventKeyDown) || (type == kCGEventKeyUp)) {
        /* Skip autorepeat */
        int64_t autorepeat = CGEventGetIntegerValueField(event, kCGKeyboardEventAutorepeat);
        if (autorepeat)
            return event;
        
        CGKeyCode keycode = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));

        /* Register the key event for when building the inputs */
        if (type == kCGEventKeyDown)
            context->config.km->registerKeyDown(keycode);
        else
            context->config.km->registerKeyUp(keycode);
        
        /* Only run hotkeys if the game app has focus */
        if (!GameEventsQuartz::haveFocus(context))
            return event;
        
        /* If pressed a controller button, update the controller input window */
        /*
         if (context->config.km->input_mapping.find(keycode) != context->config.km->input_mapping.end()) {
         SingleInput si = context->config.km->input_mapping[keycode];
         if (si.inputTypeIsController())
         emit controllerButtonToggled(si.inputTypeToControllerNumber(), si.which, type == kCGEventKeyDown);
         }*/
        
        /* Build modifiers */
        CGEventFlags flags = CGEventGetFlags(event);
        keysym_t modifiers = context->config.km->get_modifiers(flags);

        /* Check if this keycode with or without modifiers is mapped to a hotkey */
        int hk_type = -1;
        keysym_t keysym = context->config.km->nativeToKeysym(keycode);
        keysym_t keyWithMod = keysym | modifiers;
        if (context->config.km->hotkey_mapping.find(keyWithMod) != context->config.km->hotkey_mapping.end()) {
            hk_type = context->config.km->hotkey_mapping[keyWithMod].type;
        }
        else if (context->config.km->hotkey_mapping.find(keysym) != context->config.km->hotkey_mapping.end()) {
            hk_type = context->config.km->hotkey_mapping[keysym].type;
        }
        
        if (hk_type != -1) {
            if (type == kCGEventKeyDown)
                context->hotkey_pressed_queue.push(hk_type);
            else
                context->hotkey_released_queue.push(hk_type);
        }
    }
    if (type == kCGEventFlagsChanged) {
        CGEventFlags flags = CGEventGetFlags(event);
        if (flags & kCGEventFlagMaskShift)
            context->config.km->registerKeyDown(kVK_Shift);
        else
            context->config.km->registerKeyUp(kVK_Shift);
        if (flags & kCGEventFlagMaskControl)
            context->config.km->registerKeyDown(kVK_Control);
        else
            context->config.km->registerKeyUp(kVK_Control);
        if (flags & kCGEventFlagMaskAlternate)
            context->config.km->registerKeyDown(kVK_Command);
        else
            context->config.km->registerKeyUp(kVK_Command);
        if (flags & kCGEventFlagMaskCommand)
            context->config.km->registerKeyDown(kVK_Option);
        else
            context->config.km->registerKeyUp(kVK_Option);
    }
    
    return event;
}

void GameEventsQuartz::init()
{
    GameEvents::init();
    
    /* Clear the game running app */
    gameApp = nullptr;

    /* Don't create event tap if already present */
    if (eventTap)
        return;
    
    /* Check for accessibility */
    NSDictionary* options = @{(__bridge NSString*)(kAXTrustedCheckOptionPrompt) : @YES};
    if (!AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options)) {
        std::cerr << "You must turn on accessibility for this app" << std::endl;
    }
    
    /* Register the event tap */
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp) | CGEventMaskBit(kCGEventFlagsChanged);
    
    //    CFMachPortRef eventTap = CGEventTapCreateForPid(pid, kCGTailAppendEventTap, kCGEventTapOptionListenOnly, eventMask, eventTapFunction, context);
    eventTap = CGEventTapCreate(kCGHIDEventTap, kCGTailAppendEventTap, kCGEventTapOptionListenOnly, eventMask, eventTapFunction, context);
    
    if (!eventTap) {
        std::cerr << "Could not create event tap" << std::endl;
        return;
    }
    
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
}

void GameEventsQuartz::registerGameWindow(uint32_t gameWindow)
{
    context->game_window = gameWindow;
}

GameEventsQuartz::EventType GameEventsQuartz::nextEvent(struct HotKey &hk)
{
    if (!context->hotkey_pressed_queue.empty()) {
        /* Processing a pressed hotkey pushed by the UI */
        context->hotkey_pressed_queue.pop(hk.type);
        return EVENT_TYPE_PRESS;
    }
    else if (!context->hotkey_released_queue.empty()) {
        /* Processing a pressed hotkey pushed by the UI */
        context->hotkey_released_queue.pop(hk.type);
        return EVENT_TYPE_RELEASE;
    }
    return EVENT_TYPE_NONE;
}

bool GameEventsQuartz::haveFocus()
{
    return GameEventsQuartz::haveFocus(context);
}

bool GameEventsQuartz::haveFocus(Context *context)
{
    /* If not received game pid, returns false */
    if (!context->game_pid)
        return false;

    /* Get game NSRunningApplication object */
    if (!gameApp) {
        gameApp = [NSRunningApplication runningApplicationWithProcessIdentifier:context->game_pid];
        
        if (!gameApp) {
            std::cerr << "Could not get NSRunningApplication object from pid: " << context->game_pid << std::endl;
        }
    }
    
    return [gameApp isActive];
}
