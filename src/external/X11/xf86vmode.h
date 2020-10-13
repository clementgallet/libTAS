/*

Copyright 1995  Kaleb S. KEITHLEY

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL Kaleb S. KEITHLEY BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Kaleb S. KEITHLEY
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
from Kaleb S. KEITHLEY

*/

/* THIS IS NOT AN X CONSORTIUM STANDARD OR AN X PROJECT TEAM SPECIFICATION */

#ifndef _XF86VIDMODE_H_
#define _XF86VIDMODE_H_

typedef struct {
    unsigned short	hdisplay;
    unsigned short	hsyncstart;
    unsigned short	hsyncend;
    unsigned short	htotal;
    unsigned short	hskew;
    unsigned short	vdisplay;
    unsigned short	vsyncstart;
    unsigned short	vsyncend;
    unsigned short	vtotal;
    unsigned int	flags;
    int			privsize;
#if defined(__cplusplus) || defined(c_plusplus)
    /* private is a C++ reserved word */
    int		*c_private;
#else
    int		*private;
#endif
} XF86VidModeModeLine;

typedef struct {
    unsigned int	dotclock;
    unsigned short	hdisplay;
    unsigned short	hsyncstart;
    unsigned short	hsyncend;
    unsigned short	htotal;
    unsigned short	hskew;
    unsigned short	vdisplay;
    unsigned short	vsyncstart;
    unsigned short	vsyncend;
    unsigned short	vtotal;
    unsigned int	flags;
    int			privsize;
#if defined(__cplusplus) || defined(c_plusplus)
    /* private is a C++ reserved word */
    int		*c_private;
#else
    int		*private;
#endif
} XF86VidModeModeInfo;

typedef struct {
    float		hi;
    float		lo;
} XF86VidModeSyncRange;

typedef struct {
    char*			vendor;
    char*			model;
    float			EMPTY;
    unsigned char		nhsync;
    XF86VidModeSyncRange*	hsync;
    unsigned char		nvsync;
    XF86VidModeSyncRange*	vsync;
} XF86VidModeMonitor;

typedef struct {
    int type;			/* of event */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came from a SendEvent req */
    Display *display;		/* Display the event was read from */
    Window root;		/* root window of event screen */
    int state;			/* What happened */
    int kind;			/* What happened */
    Bool forced;		/* extents of new region */
    Time time;			/* event timestamp */
} XF86VidModeNotifyEvent;

typedef struct {
    float red;			/* Red Gamma value */
    float green;		/* Green Gamma value */
    float blue;			/* Blue Gamma value */
} XF86VidModeGamma;

#endif
