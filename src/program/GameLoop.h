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

#ifndef LIBTAS_GAMELOOP_H_INCLUDED
#define LIBTAS_GAMELOOP_H_INCLUDED

#include <QtCore/QObject>

#include "movie/MovieFile.h"
#include "../shared/GameInfo.h"

/* Forward declaration */
class GameEvents;
struct Context;

class GameLoop : public QObject {
    Q_OBJECT
public:
    GameLoop(Context *c);
    void start();

    void killForkProcess();

    MovieFile movie;

    /* Handle hotkeys */
    GameEvents* gameEvents;

private:
    Context* context;

    /* Last saved/loaded savestate */
    int current_savestate;

    /* Inputs from the previous frame */
    AllInputs prev_ai;

    /* Current encoding segment. Sent when game is restarted */
    int encoding_segment = 0;

    /* PID of the forked `sh` process which executes the game */
    pid_t fork_pid;

    void init();

    void initProcessMessages();

    bool startFrameMessages();

    void sleepSendPreview();

    void processInputs(AllInputs &ai);

    void endFrameMessages(AllInputs &ai);

    void loopExit();
    
signals:
    void uiChanged();
    void statusChanged(int status);
    void configChanged();
    void alertToShow(QString str);
    void sharedConfigChanged();
    void askToShow(QString str, void* promise);
    void updateFramerate();

    void showControllerInputs(const AllInputs &allinputs);
    void fillControllerInputs(AllInputs &allinputs);
    void gameInfoChanged(GameInfo game_info);

    /* Signals for notifying the input editor */
    void isInputEditorVisible(bool &isVisible);
    void inputsToBeChanged();
    void inputsChanged();
    void inputsToBeAdded();
    void inputsAdded();
    void inputsToBeEdited(unsigned long long framecount);
    void inputsEdited(unsigned long long framecount);

    void getRamWatch(std::string &watch);
    
    void getMarkerText(std::string &text);

    void getTimeTrace(int type, unsigned long long hash, std::string stacktrace);
    
    /* Savestates have been invalidated by thread change */
    void invalidateSavestates();
};

#endif
