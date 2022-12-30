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

#include <iostream>
#include <unistd.h> // access()

#include "Context.h"
#include "SaveState.h"
#include "utils.h"
#include "../shared/sockethelpers.h"
#include "../shared/SharedConfig.h"
#include "../shared/messages.h"

void SaveState::init(Context* context, int i)
{
    id = i;
    is_backtrack = (i == 10);
    framecount = 0; // Special value for `no state`
    parent = -1;
    invalid = false;
    movie = std::unique_ptr<MovieFile>(new MovieFile(context));

    buildPaths(context);
    buildMessages(context);
}

void SaveState::invalidate()
{
    parent = -1;
    invalid = true;    
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

void SaveState::buildMessages(Context* context)
{
    if (saving_msg.empty()) {
        if (is_backtrack) {
            saving_msg = "Saving backtrack state";
        }
        else {
            saving_msg = "Saving state ";
            saving_msg += std::to_string(id);
        }
    }

    if (no_state_msg.empty()) {
        no_state_msg = "No savestate in slot ";
        no_state_msg += std::to_string(id);
    }

    if (loading_msg.empty()) {
        if (is_backtrack) {
            loading_msg = "Loading backtrack state";
        }
        else {
            loading_msg = "Loading state ";
            loading_msg += std::to_string(id);
        }
    }

    if (loaded_msg.empty()) {
        if (is_backtrack) {
            loaded_msg = "Backtrack state loaded";
        }
        else {
            loaded_msg = "State ";
            loaded_msg += std::to_string(id);
            loaded_msg += " loaded";
        }
    }
}

const std::string& SaveState::getMoviePath() const
{
    return movie_path;
}

int SaveState::save(Context* context, const MovieFile& m)
{    
    if (context->config.sc.recording != SharedConfig::NO_RECORDING) {
        /* Save the movie file */
        m.copyTo(*movie);
    }
    
    /* Send the savestate index */
    sendMessage(MSGN_SAVESTATE_INDEX);
    sendData(&id, sizeof(int));

    /* Send the savestate path */
    if (! (context->config.sc.savestate_settings & SharedConfig::SS_RAM)) {
        sendMessage(MSGN_SAVESTATE_PATH);
        sendString(path);
    }
    else {
        /* Create empty savestate files if stored in RAM */
        std::ofstream opm(pagemap_path);
        opm.close();
        std::ofstream op(pages_path);
        op.close();
    }

    if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
        sendMessage(MSGN_OSD_MSG);
        sendString(saving_msg);
    }

    sendMessage(MSGN_SAVESTATE);

    /* Checking that saving succeeded */
    int message = receiveMessage();
    
    /* Set framecount */
    if (message == MSGB_SAVING_SUCCEEDED) {
        framecount = context->framecount;
        invalid = false;
    }
    
    return message;
}

int SaveState::load(Context* context, const MovieFile& m, bool branch)
{
    /* Check that the savestate exists (check for both savestate files and 
     * framecount, because there can be leftover savestate files from
     * forked savestate of previous execution). */
    if ((access(pagemap_path.c_str(), F_OK) != 0) || (access(pages_path.c_str(), F_OK) != 0) ||
        (framecount == 0)) {
        /* If there is no savestate but a movie file, offer to load
         * the movie and fast-forward to the savestate movie frame.
         */

        if ((context->config.sc.recording != SharedConfig::NO_RECORDING) &&
            (access(movie_path.c_str(), F_OK) == 0)) {

            /* If there is no savestate but a movie file, offer to load
             * the movie and fast-forward to the savestate movie frame.
             */

            if ((context->config.sc.recording != SharedConfig::NO_RECORDING) &&
                (access(movie_path.c_str(), F_OK) == 0)) {

                /* Load the savestate movie from disk */
                MovieFile savedmovie(context);
                int ret = savedmovie.loadSavestateMovie(movie_path);

                /* Checking if our movie is a prefix of the savestate movie */
                if ((ret == 0) && savedmovie.inputs->isPrefix(m.inputs, context->framecount)) {
                    return ENOSTATEMOVIEPREFIX;
                }
            }
        }

        if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
            sendMessage(MSGN_OSD_MSG);
            sendString(no_state_msg);
        }
        return ENOSTATE;
    }

    if (invalid) {
        if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
            sendMessage(MSGN_OSD_MSG);
            sendString(std::string("State invalid because new threads were created"));
        }
        return EINVALID;
    }
    
    /* Send the savestate index */
    sendMessage(MSGN_SAVESTATE_INDEX);
    sendData(&id, sizeof(int));

    /* Send savestate path */
    if (! (context->config.sc.savestate_settings & SharedConfig::SS_RAM)) {
        sendMessage(MSGN_SAVESTATE_PATH);
        sendString(path);
    }

    /* When loading in read mode and not branch, we don't allow loading a non-prefix movie */
    if ((context->config.sc.recording == SharedConfig::RECORDING_READ) && (!branch)) {

        /* Checking if the savestate movie is a prefix of our movie */
        if (!movie || !m.isPrefix(*movie)) {
            /* Not a prefix, we don't allow loading */
            if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
                sendMessage(MSGN_OSD_MSG);
                sendString(std::string("Savestate inputs mismatch"));
            }
            return EINPUTMISMATCH;
        }
    }

    if (context->config.sc.osd & SharedConfig::OSD_MESSAGES) {
        std::string msg;
        sendMessage(MSGN_OSD_MSG);
        sendString(loading_msg);
    }

    sendMessage(MSGN_LOADSTATE);
     
    return 0;
}

int SaveState::postLoad(Context* context, MovieFile& m, bool branch)
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

        if ((context->config.sc.recording == SharedConfig::RECORDING_WRITE) || branch) {
            /* When in writing move or loading a branch,
             * we load the movie associated with the savestate.
             */
            movie->copyTo(m);
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

    if (context->config.sc.recording == SharedConfig::RECORDING_WRITE) {
        context->config.sc.movie_framecount = context->framecount;
        m.header->length_sec = context->current_time_sec;
        m.header->length_nsec = context->current_time_nsec;
    }

    if (didLoad && (context->config.sc.osd & SharedConfig::OSD_MESSAGES)) {
        sendMessage(MSGN_OSD_MSG);
        sendString(loaded_msg);
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
