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

#include "Context.h"
#include "SaveState.h"
#include "utils.h"
#include "../shared/sockethelpers.h"
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"

#include <iostream>
#include <unistd.h> // access()

void SaveState::init(Context* context, int i)
{
    id = i;
    framecount = 0; // Special value for `no state`
    parent = -1;
    movie = std::unique_ptr<MovieFile>(new MovieFile(context));

    buildPaths(context);
    buildMessages();
}

void SaveState::buildPaths(Context* context)
{
    /* Build the savestate paths */
    if (path.empty()) {
        path = context->config.savestatedir + '/';
        path += context->gamename;
        path += ".state" + std::to_string(id);        
    }
    
    if (pagemap_path.empty())
        pagemap_path = path + ".pm";
    if (pages_path.empty())
        pages_path = path + ".p";

    /* Build the movie path */
    if (movie_path.empty()) {
        movie_path = context->config.savestatedir + '/';
        movie_path += context->gamename;
        movie_path += ".movie" + std::to_string(id) + ".ltm";
    }
}

void SaveState::buildMessages()
{
    if (no_state_msg.empty()) {
        no_state_msg = "No savestate in slot ";
        no_state_msg += std::to_string(id);
    }

    loading_branch_msg = "Loading branch ";
    loading_branch_msg += std::to_string(id);

    loaded_branch_msg = "Branch ";
    loaded_branch_msg += std::to_string(id);
    loaded_branch_msg += " loaded";

    loading_state_msg = "Loading state ";
    loading_state_msg += std::to_string(id);

    loaded_state_msg = "State ";
    loaded_state_msg += std::to_string(id);
    loaded_state_msg += " loaded";
}

const std::string& SaveState::getMoviePath() const
{
    return movie_path;
}

int SaveState::save(Context* context, const MovieFile& m)
{    
    /* Save the movie file */
    movie->copyFrom(m);
    
    /* Send the savestate index */
    sendMessage(MSGN_SAVESTATE_INDEX);
    sendData(&id, sizeof(int));

    /* Send the savestate path */
    sendMessage(MSGN_SAVESTATE_PATH);
    sendString(path);

    sendMessage(MSGN_SAVESTATE);

    /* Checking that saving succeeded */
    int message = receiveMessage();
    
    /* Set framecount */
    if (message == MSGB_SAVING_SUCCEEDED) {
        framecount = context->framecount;
    }
    
    return message;
}

int SaveState::load(Context* context, const MovieFile& m, bool branch, bool inputEditor, int common_relative_id, uint64_t common_relative_framecount)
{
    /* Check that the savestate exists (check for both savestate files and 
     * framecount, because there can be leftover savestate files from
     * forked savestate of previous execution). */
    if ((access(pagemap_path.c_str(), F_OK) != 0) || (access(pages_path.c_str(), F_OK) != 0) ||
        (framecount == 0)) {
        /* If there is no savestate but inputs are saved in the save
         * file, offer to load the movie and fast-forward to the
         * savestate movie frame.
         */

        if (movie->inputs->nbFrames() > 0) {
            return ENOSTATEMOVIEPREFIX;
        } else {
            sendMessage(MSGN_OSD_MSG);
            sendString(no_state_msg);
            return ENOSTATE;
        }
    }

    /* Send the savestate index */
    sendMessage(MSGN_SAVESTATE_INDEX);
    sendData(&id, sizeof(int));

    /* Send savestate path */
    sendMessage(MSGN_SAVESTATE_PATH);
    sendString(path);

    /* Check if we need to load a prefix movie when:
     * - not loading a branch, and
     * - being in read mode, or being in write mode with input editor opened
     */
    if ((!branch) && 
        (context->config.sc.recording == SharedConfig::RECORDING_READ ||
            (context->config.sc.recording == SharedConfig::RECORDING_WRITE &&
             inputEditor))) {

        /* Checking if the savestate movie is a prefix of our movie */
        bool isPrefix;
        if (!movie)
            isPrefix = false;
        else {
            /* We can skip checking for inputs if we are a parent of the current state */
            if (common_relative_id == id) {
                isPrefix = true;
            }
            else if (common_relative_id != -1) {
                /* We can skip most of the checked inputs if we have a common relative
                 * with the current state */
                isPrefix = movie->isEqual(m, common_relative_framecount, framecount);
            }
            else {
                /* No relative, check the entire input for prefix */
                isPrefix = m.isPrefix(*movie);
            }
        }
        if (!isPrefix) {
            /* Not a prefix, we don't allow loading */
            sendMessage(MSGN_OSD_MSG);
            sendString(std::string("Savestate inputs mismatch"));
            return EINPUTMISMATCH;
        }
    }

    std::string msg;
    sendMessage(MSGN_OSD_MSG);
    if (branch)
        sendString(loading_branch_msg);
    else
        sendString(loading_state_msg);

    sendMessage(MSGN_LOADSTATE);
     
    return 0;
}

int SaveState::postLoad(Context* context, MovieFile& m, bool branch, bool inputEditor)
{
    int message = receiveMessage();
    
    /* Loading is not assured to succeed, the following must
     * only be done if it's the case.
     */
    bool didLoad = message == MSGB_LOADING_SUCCEEDED;
    if (didLoad) {
        /* The copy of SharedConfig that the game stores may not
         * be the same as this one due to memory loading, so we
         * send it.
         */
        sendMessage(MSGN_CONFIG);
        sendData(&context->config.sc, sizeof(SharedConfig));

        if (!((!branch) && 
            (context->config.sc.recording == SharedConfig::RECORDING_READ ||
                (context->config.sc.recording == SharedConfig::RECORDING_WRITE &&
                 inputEditor)))) {
            m.copyFrom(*movie);
        }

        /* If the movie was modified since last state load, increment
         * the rerecord count. */
        if (m.inputs->modifiedSinceLastStateLoad) {
            context->rerecord_count++;
            m.inputs->modifiedSinceLastStateLoad = false;
        }

        message = receiveMessage();
    }

    /* The frame count has changed, we must get the new one */
    if (message != MSGB_FRAMECOUNT_TIME) {
        std::cerr << "Got wrong message after state loading" << std::endl;
        return ENOLOAD;
    }
    
    receiveData(&context->framecount, sizeof(uint64_t));
    receiveData(&context->current_time_sec, sizeof(uint64_t));
    receiveData(&context->current_time_nsec, sizeof(uint64_t));
    receiveData(&context->current_realtime_sec, sizeof(uint64_t));
    receiveData(&context->current_realtime_nsec, sizeof(uint64_t));
    context->new_realtime_sec = context->current_realtime_sec;
    context->new_realtime_nsec = context->current_realtime_nsec;    

    // if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
    //     context->config.sc.movie_framecount = context->framecount;
    //     m.header->length_sec = context->current_time_sec;
    //     m.header->length_nsec = context->current_time_nsec;
    // }

    if (didLoad) {
        sendMessage(MSGN_OSD_MSG);
        if (branch)
            sendString(loaded_branch_msg);
        else
            sendString(loaded_state_msg);
    }

    sendMessage(MSGN_EXPOSE);
    
    if (didLoad)
        return MSGB_LOADING_SUCCEEDED;
    
    return 0;
}

void SaveState::backupMovie()
{
    if (framecount) // 0 means no state has been made
        movie->saveMovie(movie_path);
}
