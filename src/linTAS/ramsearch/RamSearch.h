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

#ifndef LINTAS_RAMSEARCH_H_INCLUDED
#define LINTAS_RAMSEARCH_H_INCLUDED

#include "IRamWatch.h"
#include "RamWatch.h"
#include "CompareEnums.h"
#include "MemSection.h"

#include <sys/types.h>
#include <sstream>
#include <fstream>
#include <FL/Fl.H>
#include <FL/Fl_Hor_Fill_Slider.H>

#include <memory>
#include <vector>
// #include <sys/ptrace.h>
// #include <sys/wait.h>
#include <iostream>

class RamSearch {

    public:
        std::vector<std::unique_ptr<IRamWatch>> ramwatches;
        pid_t game_pid;

        template <class T>
        void new_watches(pid_t pid, int type_filter, CompareType compare_type, CompareOperator compare_operator, double compare_value, Fl_Hor_Fill_Slider *search_progress)
        {
            game_pid = pid;
            ramwatches.clear();
            IRamWatch::game_pid = pid;

            /* Compose the filename for the /proc memory map, and open it. */
            std::ostringstream oss;
            oss << "/proc/" << game_pid << "/maps";
            std::ifstream mapsfile(oss.str());
            if (!mapsfile) {
                std::cerr << "Could not open " << oss.str() << std::endl;
                return;
            }

            std::string line;
            MemSection::reset();

            /* We first get the total size of the watches, to be able to
             * print the progress bar
             */
            int total_size = 0;
            while (std::getline(mapsfile, line)) {
                MemSection section;
                section.readMap(line);

                /* Filter based on type */
                if (!(type_filter & section.type))
                    continue;

                total_size += section.size/sizeof(T);
            }

            search_progress->show();
            search_progress->bounds(0, total_size);

            /* Rewinding the ifstream */
            mapsfile.clear( );
            mapsfile.seekg( 0, std::ios::beg );

            MemSection::reset();
            int cur_size = 0;
            while (std::getline(mapsfile, line)) {

                MemSection section;
                section.readMap(line);

                /* Filter based on type */
                if (!(type_filter & section.type))
                    continue;

                /* Reserve the vector space so we avoid multiple reallocations */
                ramwatches.reserve(ramwatches.size() + section.size/sizeof(T));

                /* For now we only store aligned addresses */
                std::unique_ptr<RamWatch<T>> watch(nullptr);
                for (uintptr_t addr = section.addr; addr < section.endaddr; addr += sizeof(T)) {

                    if (!(cur_size++ & 0xfff)) {
                        search_progress->value(cur_size);
                        Fl::flush();
                    }

                    if (! watch)
                        watch = std::unique_ptr<RamWatch<T>>(new RamWatch<T>(addr));
                    else
                        /* Reusing a watch object that wasn't inserted */
                        watch->address = addr;

                    /* If only insert watches that match the compare */
                    if (compare_type == CompareType::Value) {
                        if (!watch->check_update(compare_type, compare_operator, compare_value)) {
                            ramwatches.push_back(std::move(watch));
                        }
                    }

                    /* Insert all watches, still checking for accessible and non NaN/Inf values */
                    else {
                        if (!watch->query()) {
                            ramwatches.push_back(std::move(watch));
                        }
                    }
                }
            }

            search_progress->hide();
        }
};

#endif
