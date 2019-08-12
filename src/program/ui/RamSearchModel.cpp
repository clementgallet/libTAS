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

#include "RamSearchModel.h"

RamSearchModel::RamSearchModel(Context* c, QObject *parent) : QAbstractTableModel(parent), context(c) {}

int RamSearchModel::rowCount(const QModelIndex & /*parent*/) const
{
   return ramwatches.size();
}

int RamSearchModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant RamSearchModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return QString("Address");
            case 1:
                return QString("Value");
            case 2:
                return QString("Previous Value");
            }
        }
    }
    return QVariant();
}

QVariant RamSearchModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const RamWatch &watch = ramwatches.at(index.row());
        switch(index.column()) {
            case 0:
                return QString("%1").arg(watch.address, 0, 16);
            case 1:
                return QString(watch.tostring(hex, watch.get_value()));
            case 2:
                return QString(watch.tostring(hex, watch.previous_value));
            default:
                return QString();
        }
    }
    return QVariant();
}

int RamSearchModel::predictWatchCount(int mem_filter)
{
    /* Compose the filename for the /proc memory map, and open it. */
    std::ostringstream oss;
    oss << "/proc/" << context->game_pid << "/maps";
    std::ifstream mapsfile(oss.str());
    if (!mapsfile) {
        std::cerr << "Could not open " << oss.str() << std::endl;
        return 0;
    }

    std::string line;
    MemSection::reset();

    int total_size = 0;
    while (std::getline(mapsfile, line)) {
        MemSection section;
        section.readMap(line);

        /* Filter based on type */
        if (!(mem_filter & section.type))
            continue;

        total_size += section.size;
    }

    return total_size;
}

int RamSearchModel::watchCount()
{
    return ramwatches.size();
}

void RamSearchModel::newWatches(int mem_filter, int type, CompareType ct, CompareOperator co, double cv)
{
    compare_type = ct;
    compare_operator = co;
    compare_value = cv;

    beginResetModel();

    ramwatches.clear();
    ramwatches.reserve(0);

    RamWatch::game_pid = context->game_pid;
    RamWatch::type = type;
    RamWatch::type_size = RamWatch::type_to_size();

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
        if (!(mem_filter & section.type))
            continue;

        struct iovec local, remote;

        /* Read values in chunks of 4096 bytes so we lower the number
         * of `process_vm_readv` calls. */
        uint8_t chunk[4096+8];
        local.iov_base = static_cast<void*>(chunk);
        local.iov_len = 4096;
        remote.iov_len = 4096;

        for (uintptr_t addr = section.addr; addr < section.endaddr; addr += 4096) {

            remote.iov_base = reinterpret_cast<void*>(addr);

            int readValues = process_vm_readv(context->game_pid, &local, 1, &remote, 1, 0);
            if (readValues < 0) {
                continue;
            }

            for (unsigned int i = 0; i < readValues; i += RamWatch::type_size, cur_size += RamWatch::type_size) {

                if (!(cur_size & 0xfff)) {
                    emit signalProgress(cur_size);
                }

                ramwatches.emplace_back(addr+i);
                ramwatches.back().previous_value = *reinterpret_cast<uint64_t*>(chunk+i);

                /* If only insert watches that match the compare */
                if (compare_type == CompareType::Value) {
                    if (ramwatches.back().check(ramwatches.back().previous_value, compare_type, compare_operator, compare_value)) {
                        ramwatches.pop_back();
                    }
                }
            }
        }
    }

    endResetModel();
}

void RamSearchModel::searchWatches(CompareType ct, CompareOperator co, double cv)
{
    compare_type = ct;
    compare_operator = co;
    compare_value = cv;

    beginResetModel();

    int count = 0;
    ramwatches.erase(
        std::remove_if(ramwatches.begin(), ramwatches.end(),
            [this, &count] (RamWatch &watch) {
                if (!(count++ & 0xfff)) {
                    emit signalProgress(count);
                }
                return watch.check_update(compare_type, compare_operator, compare_value);
            }),
        ramwatches.end());

    endResetModel();
}

void RamSearchModel::update()
{
    emit dataChanged(createIndex(0,1), createIndex(rowCount(),1));
}
