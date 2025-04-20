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

#ifndef LIBTAS_MOVIEFILEEDITOR_H_INCLUDED
#define LIBTAS_MOVIEFILEEDITOR_H_INCLUDED

#include "../shared/inputs/SingleInput.h"

#include <vector>
#include <set>
#include <map>
#include <stdint.h>

class AllInputs;
struct Context;

class MovieFileEditor {
public:
    /* Ordered list of single inputs to be shown on the input editor */
    std::vector<SingleInput> input_set;

    /* List of single inputs to autohold or autofire.
     * 0: no, 1: autohold, 2: autofire on even frames, 3: autofire on odd frames */
    std::vector<unsigned int> autohold;

    /* Ordered list of markers to be shown on the input editor */
    std::map<int, std::string> markers;

    /* List of locked single inputs. They won't be modified even in recording mode */
    std::set<SingleInput> locked_inputs;

    /* List of nondraw frames */
    std::set<uint64_t> nondraw_frames;

    /* Prepare a movie file from the context */
    MovieFileEditor(Context* c);

    /* Clear */
    void clear();

    /* Import the inputs into a list, and all the parameters.
     * Returns 0 if no error, or a negative value if an error occured */
    void load();

    /* Write the inputs into a file and compress to the whole moviefile */
    void save();

    /* Copy locked inputs from the current inputs to the inputs in argument */
    void setLockedInputs(AllInputs& inputs, const AllInputs& movie_inputs);

    /* Set draw of current frame */
    void setDraw(bool draw);

    /* Returns if draw frame */
    bool isDraw(uint64_t frame);

    /* Set autohold of single input */
    void setAutohold(int index, bool checked);

    /* Returns if autohold of single input */
    bool isAutohold(int index);

    /* Set autofire of single input */
    void setAutofire(int index, bool checked);

    /* Returns if autofire of single input */
    bool isAutofire(int index);

    /* Close the moviefile */
    void close();

private:
    Context* context;

};

#endif
