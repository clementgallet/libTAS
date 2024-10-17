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

#ifndef LIBTAS_MOVIEACTIONPAINT_H_INCLUDED
#define LIBTAS_MOVIEACTIONPAINT_H_INCLUDED

#include "IMovieAction.h"
#include "../shared/inputs/SingleInput.h"

#include <cstdint>
#include <vector>

class MovieFileInputs;

class MovieActionPaint : public IMovieAction {
    // Q_OBJECT
public:
    MovieActionPaint(uint64_t start_frame, uint64_t end_frame, SingleInput si, int newV, MovieFileInputs* movie_inputs);
    MovieActionPaint(uint64_t start_frame, SingleInput si, const std::vector<int>& newV, MovieFileInputs* movie_inputs);

    void undo() override;
    void redo() override;
    
private:
    SingleInput input;
    std::vector<int> old_values;
    std::vector<int> new_values;
    int new_value;
};

#endif
