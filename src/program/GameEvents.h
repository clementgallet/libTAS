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

#ifndef LIBTAS_GAMEEVENTS_H_INCLUDED
#define LIBTAS_GAMEEVENTS_H_INCLUDED

#include <QtCore/QObject>
// #include <memory>
#include <stdint.h>

#include "Context.h"
#include "KeyMapping.h"

/* Forward declaration */
class MovieFile;

class GameEvents : public QObject {
    Q_OBJECT
public:
    GameEvents(Context *c, MovieFile* m);

    virtual void init();

    /* Register and select events from the window handle */
    virtual void registerGameWindow(uint32_t gameWindow) = 0;

    /* Flags to return after a single call to `handleEvent())` */
    enum ReturnFlag {
        RETURN_FLAG_EVENT = 0x1, // an event was processed
        RETURN_FLAG_ADVANCE = 0x2, // advance to next frame
        RETURN_FLAG_UPDATE = 0x4, // update UI
    };

    /* Handle an event from the queue and return flags */
    int handleEvent();

    /* Determine if we are allowed to send inputs to the game, based on which
     * window has focus and our settings.
     */
    virtual bool haveFocus() = 0;

protected:
    Context* context;
    MovieFile* movie;

    /* Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every
     * iteration of the do loop.
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0: trigger frame advance */
    int ar_ticks;
    int ar_delay;
    int ar_freq;

    enum EventType {
        EVENT_TYPE_NONE = 0,
        EVENT_TYPE_PRESS,
        EVENT_TYPE_RELEASE,
        EVENT_TYPE_FOCUS_OUT,
        EVENT_TYPE_EXPOSE,
        EVENT_TYPE_INPUT_SET,
        EVENT_TYPE_INPUT_TOGGLE,
    };

    virtual EventType nextEvent(struct HotKey &hk) = 0;

    bool processEvent(EventType type, struct HotKey &hk);

signals:
    void alertToShow(QString str);
    void sharedConfigChanged();
    void askToShow(QString str, void* promise);

    void controllerButtonToggled(int controller_id, int button, bool pressed);

    /* Signals for notifying the input editor */
    void inputsToBeChanged();
    void inputsChanged();
    void inputsEdited(unsigned long long framecount);

    /* register a savestate */
    void savestatePerformed(int slot, unsigned long long frame);
};

#endif
