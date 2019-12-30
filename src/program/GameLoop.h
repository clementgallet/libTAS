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

#ifndef LIBTAS_GAMELOOP_H_INCLUDED
#define LIBTAS_GAMELOOP_H_INCLUDED

#include <QObject>
#include <memory>

#include "Context.h"
#include "MovieFile.h"
#include <xcb/xcb_keysyms.h>

class GameLoop : public QObject {
    Q_OBJECT
public:
    GameLoop(Context *c);
    void start();
    MovieFile movie;

private:
    Context* context;

    /*
     * Frame advance auto-repeat variables.
     * If ar_ticks is >= 0 (auto-repeat activated), it increases by one every
     * iteration of the do loop.
     * If ar_ticks > ar_delay and ar_ticks % ar_freq == 0: trigger frame advance
     */
    int ar_ticks;
    int ar_delay;
    int ar_freq;

    /* Keyboard layout */
    std::unique_ptr<xcb_key_symbols_t, void(*)(xcb_key_symbols_t*)> keysyms;

    /* Last saved/loaded savestate */
    int current_savestate;

    /* Inputs from the previous frame */
    AllInputs prev_ai;

    /* Calibration offsets */
    int pointer_offset_x;
    int pointer_offset_y;

    void init();

    void initProcessMessages();

    bool startFrameMessages();

    /* Set the different environment variables, then start the game executable with
     * our library to be injected using the LD_PRELOAD trick.
     * Because this function eventually calls execl, it does not return.
     * So, it is called from a child process using fork().
     */
    void launchGameThread();

    uint8_t nextEvent(struct HotKey &hk);

    void notifyControllerEvent(xcb_keysym_t ks, bool pressed);

    bool processEvent(uint8_t type, struct HotKey &hk);

    void sleepSendPreview();

    void processInputs(AllInputs &ai);

    void endFrameMessages(AllInputs &ai);

    /* Determine if we are allowed to send inputs to the game, based on which
     * window has focus and our settings.
     */
    bool haveFocus();

    void loopExit();
    
    void mergeSegments();

signals:
    void statusChanged();
    void configChanged();
    void alertToShow(QString str);
    void startFrameBoundary();
    void rerecordChanged();
    void frameCountChanged();
    void sharedConfigChanged();
    void fpsChanged(float fps, float lfps);
    void askToShow(QString str, void* promise);

    void controllerButtonToggled(int controller_id, int button, bool pressed);
    void inputsToBeSent(AllInputs &allinputs);
    void gameInfoChanged(GameInfo game_info);

    /* Signals for notifying the input editor */
    void isInputEditorVisible(bool &isVisible);
    void inputsToBeChanged();
    void inputsChanged();
    void inputsToBeAdded();
    void inputsAdded();
    void inputsToBeEdited();
    void inputsEdited();

    void getRamWatch(std::string &watch);

    /* register a savestate */
    void savestatePerformed(int slot, unsigned long long frame);
};

#endif
