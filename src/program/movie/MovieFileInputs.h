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

#ifndef LIBTAS_MOVIEFILEINPUTS_H_INCLUDED
#define LIBTAS_MOVIEFILEINPUTS_H_INCLUDED

#include "ConcurrentQueue.h"
#include "../shared/inputs/AllInputs.h"

#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <stdint.h>

struct Context;

/* Struct to push movie changes from the UI to the main thread. UI thread should
 * never modify the movie */
struct InputEvent {
    uint64_t framecount;
    SingleInput si;
    int value;
};

class MovieFileInputs {
public:

    /* Flag storing if the movie has been modified since last save.
     * Used for prompting a message when the game exits if the user wants
     * to save.
     */
    bool modifiedSinceLastSave;

    /* Flag storing if the movie has been modified since last autosave. */
    bool modifiedSinceLastAutoSave;

    /* Flag storing if the movie has been modified since last state loading.
     * Used to determine when a state loading increments the rerecord count. */
    bool modifiedSinceLastStateLoad;

    /* Initial framerate values */
    unsigned int framerate_num, framerate_den;

    /* Queue of movie input changes that where pushed by the UI, to process by the main thread */
    ConcurrentQueue<InputEvent> input_event_queue;

    /* Prepare a movie file from the context */
    MovieFileInputs(Context* c);

    /* Clear */
    void clear();

    /* Import the inputs into a list, and all the parameters.
     * Returns 0 if no error, or a negative value if an error occured */
    void load();

    /* Write the inputs into a file and compress to the whole moviefile */
    void save();

    /* Write a single frame of inputs into the input stream */
    int writeFrame(std::ostream& input_stream, const AllInputs& inputs);

    /* Read a single frame of inputs from the line of inputs */
    int readFrame(const std::string& line, AllInputs& inputs);

    /* Get the number of frames of the current movie */
    uint64_t nbFrames();

    /* Set inputs for a certain frame, and truncate if keep_inputs is false */
    int setInputs(const AllInputs& inputs, uint64_t pos, bool keep_inputs);

    /* Set inputs in the current frame, and truncate if keep_inputs is false */
    int setInputs(const AllInputs& inputs, bool keep_inputs);

    /* Load inputs from a certain frame */
    int getInputs(AllInputs& inputs, uint64_t pos);
    const AllInputs& getInputs(uint64_t pos);

    /* Load inputs from the current frame */
    int getInputs(AllInputs& inputs);
    const AllInputs& getInputs();

    /* Clear a single frame of inputs */
    void clearInputs(uint64_t pos);

    /* Insert blank inputs before the requested pos */
    void insertInputsBefore(uint64_t pos, int count);

    /* Insert inputs from an array before the requested pos */
    void insertInputsBefore(const AllInputs inputs[], uint64_t pos, int count);

    /* Delete inputs at the requested pos */
    void deleteInputs(uint64_t pos, int count);

    /* Extract all single inputs of all frames and insert them in the set */
    void extractInputs(std::set<SingleInput> &set);

    /* Copy inputs to another one */
    void copyTo(MovieFileInputs* movie_inputs) const;

    /* Truncate inputs to a frame number */
    // void truncateInputs(uint64_t size);

    /* Close the moviefile */
    void close();

    /* Check if another movie has the same inputs as this movie, inside a
     * specified range of frames */
    bool isEqual(const MovieFileInputs* movie, unsigned int start_frame, unsigned int end_frame) const;

    /* Helper function called when the movie has been modified */
    void wasModified();
    
    /* Process an input event pushed by the UI thread, and returns the modified
     * framecount, so that the UI can be updated accordingly.
     * If no event left, returns UINT64_MAX */
    uint64_t processEvent();

private:
    Context* context;

    /* The list of inputs */
    std::vector<AllInputs> input_list;

    /* We need to protect the input list access, because both the main and UI
     * threads can read and write to the list */
    std::mutex input_list_mutex;

    /* Read the keyboard input string */
    int readKeyboardFrame(std::istringstream& input_string, AllInputs& inputs);

    /* Read the mouse input string */
    int readMouseFrame(std::istringstream& input_string, AllInputs& inputs);

    /* Read one controller input string */
    int readControllerFrame(std::istringstream& input_string, AllInputs& inputs, int joy);

    /* Read the flag input string */
    int readFlagFrame(std::istringstream& input_string, AllInputs& inputs);

    /* Read the framerate input string */
    int readFramerateFrame(std::istringstream& input_string, AllInputs& inputs);

    /* Read the realtime input string */
    int readRealtimeFrame(std::istringstream& input_string, AllInputs& inputs);

};

#endif
