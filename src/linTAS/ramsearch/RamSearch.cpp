/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "RamSearch.h"
// #include <iostream>
#include <algorithm>

void RamSearch::search_watches(CompareType compare_type, CompareOperator compare_operator, double compare_value)
{
    /* Update the previous_value attribute of each RamWatch object in the vector,
     * and remove objects from the vector where the search condition returns false.
     */
    ramwatches.erase(
        std::remove_if(ramwatches.begin(), ramwatches.end(),
            [compare_type, compare_operator, compare_value] (std::unique_ptr<IRamWatch> &watch) {return watch->check_update(compare_type, compare_operator, compare_value);}),
        ramwatches.end());
}
