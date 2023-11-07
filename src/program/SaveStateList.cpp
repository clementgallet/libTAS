/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "SaveStateList.h"
#include "SaveState.h"
#include "SaveState.h"
#include "Context.h"
#include "../shared/messages.h"

#include <iostream>

#define NB_STATES 11

/* Array of savestates */
static SaveState states[NB_STATES];

/* Id of last loaded or saved savestate */
static int last_state_id;

/* Old id of root savestate */
static uint64_t old_root_framecount;

void SaveStateList::init(Context* context)
{
    for (int i = 0; i < NB_STATES; i++) {
        states[i].init(context, i);
    }
    
    last_state_id = -1;
    old_root_framecount = 0;
}

SaveState& SaveStateList::get(int id)
{
    if (id < 0 || id >= NB_STATES) {
        std::cerr << "Unknown savestate " << id << std::endl;
        id = 0;
    }

    return states[id];
}

int SaveStateList::save(int id, Context* context, MovieFile& movie)
{
    SaveState& ss = get(id);
    int message = ss.save(context, movie);
    
    if (message == MSGB_SAVING_SUCCEEDED) {
        /* Update root savestate */
        old_root_framecount = rootStateFramecount();        
        
        /* Update parent of every child to its grandparent */
        for (int cid = 0; cid < NB_STATES; cid++) {
            if (cid == id)
                continue;
            if (states[cid].parent == id)
                states[cid].parent = ss.parent;
        }
        
        /* Update parent of savestate */
        if (id != last_state_id)
            ss.parent = last_state_id;
            
        last_state_id = id;
    }
    
    return message;
}

int SaveStateList::load(int id, Context* context, MovieFile& movie, bool branch)
{
    SaveState& ss = get(id);
    return ss.load(context, movie, branch);
}

int SaveStateList::postLoad(int id, Context* context, MovieFile& movie, bool branch)
{
    SaveState& ss = get(id);
    int message = ss.postLoad(context, movie, branch);
    
    if (message == MSGB_LOADING_SUCCEEDED) {
        /* Update root savestate */
        old_root_framecount = rootStateFramecount();
        last_state_id = id;
    }
    
    return message;
}

void SaveStateList::invalidate()
{
    for (int i = 0; i < NB_STATES; i++) {
        states[i].invalidate();
    }
    
    last_state_id = -1;
    old_root_framecount = 0;
}

int SaveStateList::stateAtFrame(uint64_t frame)
{
    for (int i = 0; i < NB_STATES; i++) {
        if ((states[i].framecount == frame) && !states[i].invalid)
            return states[i].id;
    }

    return -1;
}

uint64_t SaveStateList::rootStateFramecount()
{
    if (last_state_id == -1)
        return 0;
        
    int parent_id = last_state_id;
    uint64_t framecount;
    
    while (parent_id != -1) {
        framecount = states[parent_id].framecount;
        parent_id = states[parent_id].parent;
    }
    
    return framecount;
}

uint64_t SaveStateList::oldRootStateFramecount()
{
    return old_root_framecount;
}

int SaveStateList::nearestState(uint64_t framecount)
{
    if (last_state_id == -1)
        return -1;
        
    int parent_id = last_state_id;
    
    while (parent_id != -1) {
        if (states[parent_id].framecount <= framecount)
            return parent_id;
        parent_id = states[parent_id].parent;
    }
    
    return -1;
}

void SaveStateList::backupMovies()
{
    for (int i = 0; i < NB_STATES; i++) {
        states[i].backupMovie();
    }
}
