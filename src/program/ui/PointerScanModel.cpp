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

#include "PointerScanModel.h"
#include "../utils.h"
#include "../ramsearch/MemAccess.h"
#include "../ramsearch/MemLayout.h"
#include "../ramsearch/BaseAddresses.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <memory>

PointerScanModel::PointerScanModel(Context* c, QObject *parent) : QAbstractTableModel(parent), context(c) {}

void PointerScanModel::locatePointers()
{
    pointer_map.clear();
    static_pointer_map.clear();

    std::unique_ptr<MemLayout> memlayout (new MemLayout(context->game_pid));
    
    std::vector<MemSection> memory_sections;
    file_mapping_sections.clear();

    int type_flag = (MemSection::MemDataRW | MemSection::MemBSS | MemSection::MemHeap | MemSection::MemAnonymousMappingRW | MemSection::MemFileMappingRW | MemSection::MemStack);
    uint64_t total_size = memlayout->totalSize(type_flag, 0);
    
    MemSection section;
    while (memlayout->nextSection(type_flag, 0, section)) {
        /* Only store sections that could contain pointers */
        memory_sections.push_back(section);
        
        /* Keep the file mapping to access to the file and offsets */
        if (section.type & (MemSection::MemDataRW | MemSection::MemBSS | MemSection::MemFileMappingRW | MemSection::MemStack)) {
            file_mapping_sections.push_back(section);
        }
    }

    /* Read all memory and store all pointers */
    int cur_size = 0;
    for (const MemSection &section : memory_sections) {

        for (uintptr_t addr = section.addr; addr < section.endaddr; addr += 4096) {

            /* Read values in chunks of 4096 bytes so we lower the number of calls. */
            uintptr_t chunk[4096/sizeof(uintptr_t)];
            int readValues = MemAccess::read(chunk, reinterpret_cast<void*>(addr), 4096);
            if (readValues < 0) {
                continue;
            }

            /* Update progress bar */
            emit signalProgress((int)(100 * ((float)cur_size / total_size)));

            for (unsigned int i = 0; i < readValues/sizeof(uintptr_t); i++, cur_size += sizeof(uintptr_t)) {
                /* Check if the value could be a pointer */
                bool isPointer = false;

                for (const MemSection &ms : memory_sections) {
                    /* If pointing to a static section, we can skip it */
                    if (ms.type & (MemSection::MemDataRW | MemSection::MemBSS | MemSection::MemStack)) {
                        continue;
                    }

                    /* We take advantage of the fact that sections are ordered */
                    if (chunk[i] < ms.addr) {
                        break;
                    }
                    if (chunk[i] < ms.endaddr) {
                        isPointer = true;
                        break;
                    }
                }

                if (isPointer) {
                    uintptr_t stored_addr = addr + i*sizeof(uintptr_t);
                    if (section.type & (MemSection::MemDataRW | MemSection::MemBSS | MemSection::MemStack)) {
                        static_pointer_map.insert(std::make_pair(chunk[i], stored_addr));
                    }
                    else {
                        pointer_map.insert(std::make_pair(chunk[i], stored_addr));
                    }
                }
            }
        }
    }
}

void PointerScanModel::findPointerChain(uintptr_t addr, int ml, int max_offset)
{
    static uint64_t last_scan_frame = 1 << 30;
    /* Don't locate pointers again if this is the same frame */
    if (last_scan_frame != context->framecount) {
        locatePointers();
        last_scan_frame = context->framecount;
    }

    beginResetModel();

    max_level = ml;
    pointer_chains.clear();
    int offsets[10];

    // std::cout << "max offset " << max_offset << std::endl;

    recursiveFind(addr, 0, offsets, max_offset);

    endResetModel();
}

void PointerScanModel::recursiveFind(uintptr_t addr, int level, int offsets[], int max_offset)
{
    /* Search inside static data */
    auto iter = static_pointer_map.lower_bound(addr - max_offset);
    while ((iter != static_pointer_map.end()) && (iter->first <= addr)) {
        offsets[level] = addr - iter->first;
        uintptr_t base_address = iter->second;
        // std::cout << "Found static chain with last offset " << std::dec << offsets[level] << " and base address " << std::hex << base_address << std::endl;
        std::vector<int> offset_vec(offsets, offsets + level + 1);
        pointer_chains.push_back(std::make_pair(base_address,offset_vec));
        iter++;
    }

    /* Stop if we reached the last level */
    if (level == (max_level-1))
        return;

    /* Search inside dynamic data */
    iter = pointer_map.lower_bound(addr - max_offset);
    while ((iter != pointer_map.end()) && (iter->first <= addr)) {
        offsets[level] = addr - iter->first;
        uintptr_t base_address = iter->second;
        // std::cout << "Found chain with offset " << std::dec << offsets[level] << " and base address " << std::hex << base_address << std::endl;
        recursiveFind(base_address, level+1, offsets, max_offset);
        iter++;
    }
}

int PointerScanModel::rowCount(const QModelIndex & /*parent*/) const
{
    return pointer_chains.size();
}

int PointerScanModel::columnCount(const QModelIndex & /*parent*/) const
{
    return max_level+1;
}

QVariant PointerScanModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section == 0) {
                return QString("Base Address");
            }
            return QString("Offset %1").arg(section);
        }
    }
    return QVariant();
}

QVariant PointerScanModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const std::pair<uintptr_t, std::vector<int>> &chain = pointer_chains.at(index.row());
        if (index.column() == 0) {
            /* Get file and offset */
            off_t offset;
            std::string file = BaseAddresses::getFileAndOffset(chain.first, offset);
            if (offset >= 0)
                return QString("%1+0x%2").arg(file.c_str()).arg(offset, 0, 16);
            else
                return QString("%1-0x%2").arg(file.c_str()).arg(-offset, 0, 16);
        }
        if (index.column() > static_cast<int>(chain.second.size())) {
            return QString("");
        }
        /* Offsets are stored in reverse order */
        return QString("%1").arg(*(chain.second.rbegin() + (index.column() - 1)), 0, 16);
    }
    return QVariant();
}
