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
#include <sys/types.h>
#include "MemSection.h"
#include <sstream>
#include <fstream>

// #include "../shared/AllInputs.h"
// #include "../shared/SharedConfig.h"
// #include <X11/Xlib.h>
// #include <X11/keysym.h>
// #include <map>
// #include <vector>
#include <memory>
#include <forward_list>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <iostream>

class RamSearch {

    public:
        std::forward_list<std::unique_ptr<IRamWatch>> ramwatches;
        pid_t game_pid;

        void attach();
        void detach();

        template <class T>
        void new_watches(pid_t pid, int type_filter)
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
                // detachToGame(game_pid);
                return;
            }

            std::string line;
            MemSection::reset();
            while (std::getline(mapsfile, line)) {

                MemSection section;
                section.readMap(line);

                // std::cerr << "Found section at addr " <<  (void*)section.addr << " of type " << section.type << std::endl;

                /* Filter based on type */
                if (!(type_filter & section.type))
                    continue;

                /* For now we only store aligned addresses */
                for (uintptr_t addr = section.addr; addr < section.endaddr; addr += sizeof(T)) {
                    std::unique_ptr<IRamWatch> watch(new RamWatch<T>);
                    watch->address = addr;
                    ramwatches.push_front(std::move(watch));
                }
            }

            attach();

            /* Update the previous_value attribute of each RamWatch object in the list,
             * and remove objects from the list where we couldn't access its address.
             */
            ramwatches.remove_if([] (std::unique_ptr<IRamWatch> &watch) {return dynamic_cast<RamWatch<T>*>(watch.get())->update();});
            // ramwatches.remove_if([] (auto &watch) {return dynamic_cast<RamWatch<T>&>(watch).update();});
            // ramwatches.remove_if([] (IRamWatch &watch) {return watch.update();});

            detach();
        }

        void search_watches(CompareType compare_type, CompareOperator compare_operator, double compare_value)
        {
            attach();

            /* Update the previous_value attribute of each RamWatch object in the list,
             * and remove objects from the list where we couldn't access its address.
             */
            // ramwatches.remove_if([compare_type, compare_operator, compare_value] (RamWatch<T> &watch) {return watch.search(compare_type, compare_operator, compare_value);});
            ramwatches.remove_if([compare_type, compare_operator, compare_value] (std::unique_ptr<IRamWatch> &watch) {return watch->search(compare_type, compare_operator, compare_value);});

            detach();
        }
};

#endif
