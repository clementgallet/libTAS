/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LINTAS_RAMSEARCHMODEL_H_INCLUDED
#define LINTAS_RAMSEARCHMODEL_H_INCLUDED

#include <QAbstractTableModel>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sstream>
#include <fstream>
#include <iostream>

#include "../Context.h"
#include "../ramsearch/IRamWatch.h"
#include "../ramsearch/CompareEnums.h"
#include "../ramsearch/RamWatch.h"
#include "../ramsearch/MemSection.h"

class RamSearchModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RamSearchModel(Context* c, QObject *parent = Q_NULLPTR);

    void update();

    /* List of watches */
    std::vector<std::unique_ptr<IRamWatch>> ramwatches;

    /* Flag if we display values in hex or decimal */
    bool hex;

    /* Comparison parameters so that we can display with addresses would be
     * removed by the search */
    CompareType compare_type;
    CompareOperator compare_operator;
    double compare_value;

    template <class T>
    // void new_watches(pid_t pid, int type_filter, CompareType compare_type, CompareOperator compare_operator, double compare_value, Fl_Hor_Fill_Slider *search_progress)
    void newWatches(int type_filter, CompareType ct, CompareOperator co, double cv)
    {
        compare_type = ct;
        compare_operator = co;
        compare_value = cv;

        beginResetModel();

        ramwatches.clear();
        ramwatches.reserve(0);

        IRamWatch::game_pid = context->game_pid;

        /* Compose the filename for the /proc memory map, and open it. */
        std::ostringstream oss;
        oss << "/proc/" << context->game_pid << "/maps";
        std::ifstream mapsfile(oss.str());
        if (!mapsfile) {
            std::cerr << "Could not open " << oss.str() << std::endl;
            return;
        }

        std::string line;
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

            struct iovec local, remote;

            for (uintptr_t addr = section.addr; addr < section.endaddr; addr += 4096) {

                /* Read values in chunks of 4096 bytes so we lower the number
                 * of `process_vm_readv` calls.
                 */
                T chunk[4096/sizeof(T)];
                local.iov_base = static_cast<void*>(chunk);
                local.iov_len = 4096;
                remote.iov_base = reinterpret_cast<void*>(addr);
                remote.iov_len = 4096;

                int readValues = process_vm_readv(context->game_pid, &local, 1, &remote, 1, 0);
                if (readValues < 0) {
                    continue;
                }

                for (unsigned int i = 0; i < readValues/sizeof(T); i++, cur_size += sizeof(T)) {

                    if (!(cur_size & 0xfff)) {
                        emit signalProgress(cur_size);
                    }

                    if (! watch)
                        watch = std::unique_ptr<RamWatch<T>>(new RamWatch<T>(addr+i*sizeof(T)));
                    else
                        /* Reusing a watch object that wasn't inserted */
                        watch->address = addr+i*sizeof(T);

                    watch->previous_value = chunk[i];

                    /* If only insert watches that match the compare */
                    if (compare_type == CompareType::Value) {
                        if (!watch->check(chunk[i], compare_type, compare_operator, compare_value)) {
                            ramwatches.push_back(std::move(watch));
                        }
                    }

                    /* Insert all watches, still checking for accessible and non NaN/Inf values */
                    else {
                        if (std::isfinite(chunk[i])) {
                            ramwatches.push_back(std::move(watch));
                        }
                    }
                }
            }
        }

        endResetModel();
    }

    int predictWatchCount(int type_filter);
    int watchCount();
    void searchWatches(CompareType ct, CompareOperator co, double cv);

private:
    Context *context;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:
    void signalProgress(int);

};

#endif
