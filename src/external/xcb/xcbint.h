/*
 * Copyright (C) 2001-2004 Bart Massey and Jamey Sharp.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the names of the authors or their
 * institutions shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the authors.
 */

#ifndef __XCBINT_H
#define __XCBINT_H

#include "pthread.h"

#define HAVE_SENDMSG 1
#define XCB_QUEUE_BUFFER_SIZE 16384 // This is default value

enum lazy_reply_tag
{
    LAZY_NONE = 0,
    LAZY_COOKIE,
    LAZY_FORCED
};

/* xcb_out.c */

#if HAVE_SENDMSG
#define XCB_MAX_PASS_FD 16

typedef struct _xcb_fd {
    int fd[XCB_MAX_PASS_FD];
    int nfd;
    int ifd;
} _xcb_fd;
#endif

typedef struct _xcb_out {
    pthread_cond_t cond;
    int writing;

    pthread_cond_t socket_cond;
    void (*return_socket)(void *closure);
    void *socket_closure;
    int socket_moving;

    char queue[XCB_QUEUE_BUFFER_SIZE];
    int queue_len;

    uint64_t request;
    uint64_t request_written;

    pthread_mutex_t reqlenlock;
    enum lazy_reply_tag maximum_request_length_tag;
    uint32_t value;
#if HAVE_SENDMSG
    _xcb_fd out_fd;
#endif
} _xcb_out;

/* xcb_in.c */

typedef struct _xcb_in {
    pthread_cond_t event_cond;
    int reading;

    char queue[4096];
    int queue_len;

    uint64_t request_expected;
    uint64_t request_read;
    uint64_t request_completed;
    void *current_reply;
    void **current_reply_tail;

    void *replies;
    void *events;
    void **events_tail;
    void *readers;
    void *special_waiters;

    void *pending_replies;
    void **pending_replies_tail;
#if HAVE_SENDMSG
    _xcb_fd in_fd;
#endif
    void *special_events;
} _xcb_in;

/* xcb_xid.c */

typedef struct _xcb_xid {
    pthread_mutex_t lock;
    uint32_t last;
    uint32_t base;
    uint32_t max;
    uint32_t inc;
} _xcb_xid;

/* xcb_ext.c */

typedef struct _xcb_ext {
    pthread_mutex_t lock;
    void *extensions;
    int extensions_size;
} _xcb_ext;


/* xcb_conn.c */

struct xcb_connection_t {
    /* This must be the first field; see _xcb_conn_ret_error(). */
    int has_error;

    /* constant data */
    void *setup;
    int fd;

    /* I/O data */
    pthread_mutex_t iolock;
    _xcb_in in;
    _xcb_out out;

    /* misc data */
    _xcb_ext ext;
    _xcb_xid xid;
};

#endif
