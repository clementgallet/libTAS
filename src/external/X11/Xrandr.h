/*
 * Copyright © 2000 Compaq Computer Corporation, Inc.
 * Copyright © 2002 Hewlett-Packard Company, Inc.
 * Copyright © 2006 Intel Corporation
 * Copyright © 2008 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, HP Labs, Hewlett-Packard, Inc.
 *	    Keith Packard, Intel Corporation
 */

#ifndef _XRANDR_H_
#define _XRANDR_H_

#include "randr.h"
#include "Xrender.h"
// 
// #include <X11/Xfuncproto.h>

typedef XID RROutput;
typedef XID RRCrtc;
typedef XID RRMode;
typedef XID RRProvider;

typedef struct {
    int	width, height;
    int	mwidth, mheight;
} XRRScreenSize;

/*
 *  Events.
 */

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    Window root;		/* Root window for changed screen */
    Time timestamp;		/* when the screen change occurred */
    Time config_timestamp;	/* when the last configuration change */
    SizeID size_index;
    SubpixelOrder subpixel_order;
    Rotation rotation;
    int width;
    int height;
    int mwidth;
    int mheight;
} XRRScreenChangeNotifyEvent;

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    int subtype;		/* RRNotify_ subtype */
} XRRNotifyEvent;

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    int subtype;		/* RRNotify_OutputChange */
    RROutput output;		/* affected output */
    RRCrtc crtc;	    	/* current crtc (or None) */
    RRMode mode;	    	/* current mode (or None) */
    Rotation rotation;		/* current rotation of associated crtc */
    Connection connection;	/* current connection status */
    SubpixelOrder subpixel_order;
} XRROutputChangeNotifyEvent;

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    int subtype;		/* RRNotify_CrtcChange */
    RRCrtc crtc;    		/* current crtc (or None) */
    RRMode mode;	    	/* current mode (or None) */
    Rotation rotation;		/* current rotation of associated crtc */
    int x, y;			/* position */
    unsigned int width, height;	/* size */
} XRRCrtcChangeNotifyEvent;

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    int subtype;		/* RRNotify_OutputProperty */
    RROutput output;		/* related output */
    Atom property;		/* changed property */
    Time timestamp;		/* time of change */
    int state;			/* NewValue, Deleted */
} XRROutputPropertyNotifyEvent;

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    int subtype;		/* RRNotify_ProviderChange */
    RRProvider provider; 	/* current provider (or None) */
    Time timestamp;		/* time of change */
    unsigned int current_role;
} XRRProviderChangeNotifyEvent;

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    int subtype;		/* RRNotify_ProviderProperty */
    RRProvider provider;		/* related provider */
    Atom property;		/* changed property */
    Time timestamp;		/* time of change */
    int state;			/* NewValue, Deleted */
} XRRProviderPropertyNotifyEvent;

typedef struct {
    int type;			/* event base */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent request */
    Display *display;		/* Display the event was read from */
    Window window;		/* window which selected for this event */
    int subtype;		/* RRNotify_ResourceChange */
    Time timestamp;		/* time of change */
} XRRResourceChangeNotifyEvent;

/* internal representation is private to the library */
typedef struct _XRRScreenConfiguration XRRScreenConfiguration;

/* Version 1.2 additions */

typedef unsigned long XRRModeFlags;

typedef struct _XRRModeInfo {
    RRMode		id;
    unsigned int	width;
    unsigned int	height;
    unsigned long	dotClock;
    unsigned int	hSyncStart;
    unsigned int	hSyncEnd;
    unsigned int	hTotal;
    unsigned int	hSkew;
    unsigned int	vSyncStart;
    unsigned int	vSyncEnd;
    unsigned int	vTotal;
    char		*name;
    unsigned int	nameLength;
    XRRModeFlags	modeFlags;
} XRRModeInfo;

typedef struct _XRRScreenResources {
    Time	timestamp;
    Time	configTimestamp;
    int		ncrtc;
    RRCrtc	*crtcs;
    int		noutput;
    RROutput	*outputs;
    int		nmode;
    XRRModeInfo	*modes;
} XRRScreenResources;

typedef struct _XRROutputInfo {
    Time	    timestamp;
    RRCrtc	    crtc;
    char	    *name;
    int		    nameLen;
    unsigned long   mm_width;
    unsigned long   mm_height;
    Connection	    connection;
    SubpixelOrder   subpixel_order;
    int		    ncrtc;
    RRCrtc	    *crtcs;
    int		    nclone;
    RROutput	    *clones;
    int		    nmode;
    int		    npreferred;
    RRMode	    *modes;
} XRROutputInfo;

typedef struct {
    Bool    pending;
    Bool    range;
    Bool    immutable;
    int	    num_values;
    long    *values;
} XRRPropertyInfo;

typedef struct _XRRCrtcInfo {
    Time	    timestamp;
    int		    x, y;
    unsigned int    width, height;
    RRMode	    mode;
    Rotation	    rotation;
    int		    noutput;
    RROutput	    *outputs;
    Rotation	    rotations;
    int		    npossible;
    RROutput	    *possible;
} XRRCrtcInfo;

typedef struct _XRRCrtcGamma {
    int		    size;
    unsigned short  *red;
    unsigned short  *green;
    unsigned short  *blue;
} XRRCrtcGamma;

/* Version 1.3 additions */

typedef struct _XRRCrtcTransformAttributes {
    XTransform	pendingTransform;
    char	*pendingFilter;
    int		pendingNparams;
    XFixed	*pendingParams;
    XTransform	currentTransform;
    char	*currentFilter;
    int		currentNparams;
    XFixed	*currentParams;
} XRRCrtcTransformAttributes;

typedef struct _XRRPanning {
    Time            timestamp;
    unsigned int left;
    unsigned int top;
    unsigned int width;
    unsigned int height;
    unsigned int track_left;
    unsigned int track_top;
    unsigned int track_width;
    unsigned int track_height;
    int          border_left;
    int          border_top;
    int          border_right;
    int          border_bottom;
} XRRPanning;

typedef struct _XRRProviderResources {
    Time timestamp;
    int nproviders;
    RRProvider *providers;
} XRRProviderResources;

typedef struct _XRRProviderInfo {
    unsigned int capabilities;
    int ncrtcs;
    RRCrtc	*crtcs;
    int noutputs;
    RROutput    *outputs;
    char	    *name;
    int nassociatedproviders;
    RRProvider *associated_providers;
    unsigned int *associated_capability;
    int		    nameLen;
} XRRProviderInfo;
  
typedef struct _XRRMonitorInfo {
    Atom name;
    Bool primary;
    Bool automatic;
    int noutput;
    int x;
    int y;
    int width;
    int height;
    int mwidth;
    int mheight;
    RROutput *outputs;
} XRRMonitorInfo;

#endif /* _XRANDR_H_ */
