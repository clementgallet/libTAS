/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/* Definitions used by the library and client */

#ifndef _XINPUT2_H_
#define _XINPUT2_H_

#include <X11/Xlib.h>
#include "XI2.h"
// #include <X11/extensions/Xge.h>
// #include <X11/extensions/Xfixes.h> /* PointerBarrier */

/*******************************************************************
 *
 */
typedef struct {
    int                 type;
    char*               name;
    Bool                send_core;
    Bool                enable;
} XIAddMasterInfo;

typedef struct {
    int                 type;
    int                 deviceid;
    int                 return_mode; /* AttachToMaster, Floating */
    int                 return_pointer;
    int                 return_keyboard;
} XIRemoveMasterInfo;

typedef struct {
    int                 type;
    int                 deviceid;
    int                 new_master;
} XIAttachSlaveInfo;

typedef struct {
    int                 type;
    int                 deviceid;
} XIDetachSlaveInfo;

typedef union {
    int                   type; /* must be first element */
    XIAddMasterInfo       add;
    XIRemoveMasterInfo    remove;
    XIAttachSlaveInfo     attach;
    XIDetachSlaveInfo     detach;
} XIAnyHierarchyChangeInfo;

typedef struct
{
    int    base;
    int    latched;
    int    locked;
    int    effective;
} XIModifierState;

typedef XIModifierState XIGroupState;

typedef struct {
    int           mask_len;
    unsigned char *mask;
} XIButtonState;

typedef struct {
    int           mask_len;
    unsigned char *mask;
    double        *values;
} XIValuatorState;


typedef struct
{
    int                 deviceid;
    int                 mask_len;
    unsigned char*      mask;
} XIEventMask;

typedef struct
{
    int         type;
    int         sourceid;
} XIAnyClassInfo;

typedef struct
{
    int         type;
    int         sourceid;
    int         num_buttons;
    Atom        *labels;
    XIButtonState state;
} XIButtonClassInfo;

typedef struct
{
    int         type;
    int         sourceid;
    int         num_keycodes;
    int         *keycodes;
} XIKeyClassInfo;

typedef struct
{
    int         type;
    int         sourceid;
    int         number;
    Atom        label;
    double      min;
    double      max;
    double      value;
    int         resolution;
    int         mode;
} XIValuatorClassInfo;

/* new in XI 2.1 */
typedef struct
{
    int         type;
    int         sourceid;
    int         number;
    int         scroll_type;
    double      increment;
    int         flags;
} XIScrollClassInfo;

typedef struct
{
    int         type;
    int         sourceid;
    int         mode;
    int         num_touches;
} XITouchClassInfo;

typedef struct
{
    int                 deviceid;
    char                *name;
    int                 use;
    int                 attachment;
    Bool                enabled;
    int                 num_classes;
    XIAnyClassInfo      **classes;
} XIDeviceInfo;

typedef struct
{
    int                 modifiers;
    int                 status;
} XIGrabModifiers;

// typedef unsigned int BarrierEventID;

// typedef struct
// {
//     int                 deviceid;
//     PointerBarrier      barrier;
//     BarrierEventID      eventid;
// } XIBarrierReleasePointerInfo;

/**
 * Generic XI2 event. All XI2 events have the same header.
 */
typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;
    Time          time;
} XIEvent;


typedef struct {
    int           deviceid;
    int           attachment;
    int           use;
    Bool          enabled;
    int           flags;
} XIHierarchyInfo;

/*
 * Notifies the client that the device hierarchy has been changed. The client
 * is expected to re-query the server for the device hierarchy.
 */
typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;       /* XI_HierarchyChanged */
    Time          time;
    int           flags;
    int           num_info;
    XIHierarchyInfo *info;
} XIHierarchyEvent;

/*
 * Notifies the client that the classes have been changed. This happens when
 * the slave device that sends through the master changes.
 */
typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;       /* XI_DeviceChanged */
    Time          time;
    int           deviceid;     /* id of the device that changed */
    int           sourceid;     /* Source for the new classes. */
    int           reason;       /* Reason for the change */
    int           num_classes;
    XIAnyClassInfo **classes; /* same as in XIDeviceInfo */
} XIDeviceChangedEvent;

typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;
    Time          time;
    int           deviceid;
    int           sourceid;
    int           detail;
    Window        root;
    Window        event;
    Window        child;
    double        root_x;
    double        root_y;
    double        event_x;
    double        event_y;
    int           flags;
    XIButtonState       buttons;
    XIValuatorState     valuators;
    XIModifierState     mods;
    XIGroupState        group;
} XIDeviceEvent;

typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;       /* XI_RawKeyPress, XI_RawKeyRelease, etc. */
    Time          time;
    int           deviceid;
    int           sourceid;     /* Bug: Always 0. https://bugs.freedesktop.org//show_bug.cgi?id=34240 */
    int           detail;
    int           flags;
    XIValuatorState valuators;
    double        *raw_values;
} XIRawEvent;

typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;
    Time          time;
    int           deviceid;
    int           sourceid;
    int           detail;
    Window        root;
    Window        event;
    Window        child;
    double        root_x;
    double        root_y;
    double        event_x;
    double        event_y;
    int           mode;
    Bool          focus;
    Bool          same_screen;
    XIButtonState       buttons;
    XIModifierState     mods;
    XIGroupState        group;
} XIEnterEvent;

typedef XIEnterEvent XILeaveEvent;
typedef XIEnterEvent XIFocusInEvent;
typedef XIEnterEvent XIFocusOutEvent;

typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;       /* XI_PropertyEvent */
    Time          time;
    int           deviceid;     /* id of the device that changed */
    Atom          property;
    int           what;
} XIPropertyEvent;

typedef struct {
    int           type;         /* GenericEvent */
    unsigned long serial;       /* # of last request processed by server */
    Bool          send_event;   /* true if this came from a SendEvent request */
    Display       *display;     /* Display the event was read from */
    int           extension;    /* XI extension offset */
    int           evtype;
    Time          time;
    int           deviceid;
    int           sourceid;
    unsigned int  touchid;
    Window        root;
    Window        event;
    Window        child;
    int           flags;
} XITouchOwnershipEvent;

// typedef struct {
//     int           type;         /* GenericEvent */
//     unsigned long serial;       /* # of last request processed by server */
//     Bool          send_event;   /* true if this came from a SendEvent request */
//     Display       *display;     /* Display the event was read from */
//     int           extension;    /* XI extension offset */
//     int           evtype;
//     Time          time;
//     int           deviceid;
//     int           sourceid;
//     Window        event;
//     Window        root;
//     double        root_x;
//     double        root_y;
//     double        dx;
//     double        dy;
//     int           dtime;
//     int           flags;
//     PointerBarrier barrier;
//     BarrierEventID eventid;
// } XIBarrierEvent;

#endif /* XINPUT2_H */
