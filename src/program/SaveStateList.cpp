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

#include "SaveStateList.h"
#include "SaveState.h"
#include "SaveState.h"
#include "Context.h"
#include "../shared/messages.h"

#include <iostream>

#define NB_STATES 10

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

int SaveStateList::commonRelative(int id)
{
    if (last_state_id == -1)
        return -1;
    
    /* Trivial case */
    if (last_state_id == id)
        return id;
    
    /* Clear all visited flags */
    for (int i = 0; i < NB_STATES; i++) {
        states[i].visited = false;
    }

    /* Set all visited flags for current parents */
    int current_id = last_state_id;
    while (current_id != -1) {
        states[current_id].visited = true;
        current_id = states[current_id].parent;
    }
    
    /* Look at all parents of given state */
    int other_id = id;
    while (other_id != -1) {
        if (states[other_id].visited)
            return other_id;
        other_id = states[other_id].parent;
    }

    return -1;
}

int SaveStateList::save(int id, Context* context, const MovieFile& movie)
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

int SaveStateList::load(int id, Context* context, const MovieFile& movie, bool branch, bool inputEditor)
{
    SaveState& ss = get(id);
    
    /* Get common relative information to skip most of input prefix check */
    int common_relative_id = commonRelative(id);
    uint64_t common_relative_framecount = 0;
    if (common_relative_id != -1)
        common_relative_framecount = states[common_relative_id].framecount;
    return ss.load(context, movie, branch, inputEditor, common_relative_id, common_relative_framecount);
}

int SaveStateList::postLoad(int id, Context* context, MovieFile& movie, bool branch, bool inputEditor)
{
    SaveState& ss = get(id);
    int message = ss.postLoad(context, movie, branch, inputEditor);
    
    if (message == MSGB_LOADING_SUCCEEDED) {
        /* Update root savestate */
        old_root_framecount = rootStateFramecount();
        last_state_id = id;
    }
    
    return message;
}

int SaveStateList::stateAtFrame(uint64_t frame)
{
    for (int i = 0; i < NB_STATES; i++) {
        if ((states[i].framecount == frame))
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

int SaveStateList::nearestState(uint64_t framecount, const MovieFile* movie)
{
    if (last_state_id == -1)
        return -1;
        
    int parent_id = last_state_id;
    
    while (parent_id != -1) {
        if (states[parent_id].framecount <= framecount)
            break;
        parent_id = states[parent_id].parent;
    }
    
    if (parent_id == -1)
        return -1;
        
    /* We know that `parent_id` is a good savestate to rewind, but we may find
     * a better one by looking at other childs of this state, so that we have
     * very few inputs to check */
    uint64_t parent_framecount = states[parent_id].framecount;
    int best_id = parent_id;
    uint64_t best_framecount = states[best_id].framecount;
    
    for (int i = 0; i < NB_STATES; i++) {
        /* Skip state after the desired framecount */
        if ((states[i].framecount > framecount))
            continue;
            
        /* Skip worst state to rewind */
        if ((states[i].framecount <= best_framecount))
            continue;
        
        /* Check if this state is a child */
        int cur_parent_id = states[i].parent;
        while (cur_parent_id != -1) {
            if (cur_parent_id == parent_id)
                break;
            cur_parent_id = states[cur_parent_id].parent;
        }
        
        /* Not a child, skipping */
        if (cur_parent_id != parent_id)
            continue;
            
        /* Now check for prefix, we only have to check between the parent and
         * this candidate */
        if (states[i].movie->isEqual(*movie, parent_framecount, states[i].framecount)) {
            /* We found a better state to rewind to! */
            best_id = i;
            best_framecount = states[i].framecount;
        }
    }

    return best_id;
}

void SaveStateList::backupMovies()
{
    for (int i = 0; i < NB_STATES; i++) {
        states[i].backupMovie();
    }
}
