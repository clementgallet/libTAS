/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLEVENTS_H_INCLUDED
#define LIBTAS_SDLEVENTS_H_INCLUDED

#include "global.h"
#include "../external/SDL1.h"
#include <SDL2/SDL.h>

namespace libtas {

/* Pull all events from the SDL event queue and push them into our
 * emulated event queue, filtering unwanted events (input events mainly).
 */
void pushNativeEvents(void);

/* Return if the SDL 1 event must be passed to the game or be filtered */
bool filterSDL1Event(SDL1::SDL_Event *event);

/* Return if the SDL 2 event must be passed to the game or be filtered */
bool filterSDL2Event(SDL_Event *event);

/* Print which event type is it */
void logEvent(SDL_Event *event);

OVERRIDE void SDL_PumpEvents(void);

/**
 *  Checks the event queue for messages and optionally returns them.
 *
 *  If \c action is ::SDL_ADDEVENT, up to \c numevents events will be added to
 *  the back of the event queue.
 *
 *  If \c action is ::SDL_PEEKEVENT, up to \c numevents events at the front
 *  of the event queue, within the specified minimum and maximum type,
 *  will be returned and will not be removed from the queue.
 *
 *  If \c action is ::SDL_GETEVENT, up to \c numevents events at the front
 *  of the event queue, within the specified minimum and maximum type,
 *  will be returned and will be removed from the queue.
 *
 *  \return The number of events actually stored, or -1 if there was an error.
 *
 *  This function is thread-safe.
 */
OVERRIDE int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);

/**
 *  Checks to see if certain event types are in the event queue.
 */
OVERRIDE SDL_bool SDL_HasEvent(Uint32 type);
OVERRIDE SDL_bool SDL_HasEvents(Uint32 minType, Uint32 maxType);

/**
 *  This function clears events from the event queue
 *  This function only affects currently queued events. If you want to make
 *  sure that all pending OS events are flushed, you can call SDL_PumpEvents()
 *  on the main thread immediately before the flush call.
 */
OVERRIDE void SDL_FlushEvent(Uint32 type);
OVERRIDE void SDL_FlushEvents(Uint32 minType, Uint32 maxType);

/**
 *  \brief Polls for currently pending events.
 *
 *  \return 1 if there are any pending events, or 0 if there are none available.
 *
 *  \param event If not NULL, the next event is removed from the queue and
 *               stored in that area.
 */
OVERRIDE int SDL_PollEvent(SDL_Event* event);

/**
 *  \brief Waits indefinitely for the next available event.
 *
 *  \return 1, or 0 if there was an error while waiting for events.
 *
 *  \param event If not NULL, the next event is removed from the queue and
 *               stored in that area.
 */
OVERRIDE int SDL_WaitEvent(SDL_Event * event);

/**
 *  \brief Waits until the specified timeout (in milliseconds) for the next
 *         available event.
 *
 *  \return 1, or 0 if there was an error while waiting for events.
 *
 *  \param event If not NULL, the next event is removed from the queue and
 *               stored in that area.
 *  \param timeout The timeout (in milliseconds) to wait for next event.
 */
OVERRIDE int SDL_WaitEventTimeout(SDL_Event * event, int timeout);

/**
 *  \brief Add an event to the event queue.
 *
 *  \return (SDL2) 1 on success, 0 if the event was filtered, or -1 if the
 *          event queue was full or there was some other error.
 *          (SDL1) This function returns 0 on success, or -1 if the event queue
 *          was full or there was some other error.
 */
OVERRIDE int SDL_PushEvent(SDL_Event * event);

/**
 *  Sets up a filter to process all events before they change internal state and
 *  are posted to the internal event queue.
 *
 *  The filter is prototyped as:
 *  \code
 *  SDL 1:    int SDL_EventFilter(SDL_Event * event);
 *  SDL 2:    int SDL_EventFilter(void *userdata, SDL_Event * event);
 *  \endcode
 *
 *  If the filter returns 1, then the event will be added to the internal queue.
 *  If it returns 0, then the event will be dropped from the queue, but the
 *  internal state will still be updated.  This allows selective filtering of
 *  dynamically arriving events.
 *
 *  \warning  Be very careful of what you do in the event filter function, as
 *            it may run in a different thread!
 *
 *  There is one caveat when dealing with the ::SDL_QuitEvent event type.  The
 *  event filter is only called when the window manager desires to close the
 *  application window.  If the event filter returns 1, then the window will
 *  be closed, otherwise the window will remain open if possible.
 *
 *  If the quit event is generated by an interrupt signal, it will bypass the
 *  internal queue and be delivered to the application at the next event poll.
 */
OVERRIDE void SDL_SetEventFilter(SDL_EventFilter filter, void *userdata);

/**
 *  Return the current event filter - can be used to "chain" filters.
 *  The prototype of this function in SDL 1.2 and SDL 2 is different,
 *  including the return type. So we must declare a function compatible
 *  with both prototypes.
 */
OVERRIDE SDL_bool SDL_GetEventFilter(SDL_EventFilter * filter, void **userdata);
//SDL1::SDL_EventFilter SDL_GetEventFilter(void);
// SDL_GetEventFilter(SDL_EventFilter * filter, void **userdata);

/**
 *  Add a function which is called when an event is added to the queue.
 */
OVERRIDE void SDL_AddEventWatch(SDL_EventFilter filter, void *userdata);

/**
 *  Remove an event watch function added with SDL_AddEventWatch()
 */
OVERRIDE void SDL_DelEventWatch(SDL_EventFilter filter, void *userdata);

/**
 *  Run the filter function on the current event queue, removing any
 *  events for which the filter returns 0.
 */
OVERRIDE void SDL_FilterEvents(SDL_EventFilter filter, void *userdata);

/**
 *  This function allows you to set the state of processing certain events.
 *   - If \c state is set to ::SDL_IGNORE, that event will be automatically
 *     dropped from the event queue and will not event be filtered.
 *   - If \c state is set to ::SDL_ENABLE, that event will be processed
 *     normally.
 *   - If \c state is set to ::SDL_QUERY, SDL_EventState() will return the
 *     current processing state of the specified event.
 */
OVERRIDE Uint8 SDL_EventState(Uint32 type, int state);
/* @} */

/**
 *  This function allocates a set of user-defined events, and returns
 *  the beginning event number for that set of events.
 *
 *  If there aren't enough user-defined events left, this function
 *  returns (Uint32)-1
 */
OVERRIDE Uint32 SDL_RegisterEvents(int numevents);

}

#endif
