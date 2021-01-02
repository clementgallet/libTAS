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

#ifndef LIBTAS_POINTERSCANMODEL_H_INCLUDED
#define LIBTAS_POINTERSCANMODEL_H_INCLUDED

#include <QAbstractTableModel>
#include <vector>
#include <map>
// #include <pair>
#include <memory>
#include <sys/types.h>
#include <stdint.h>

#include "../Context.h"
#include "../ramsearch/MemSection.h"

class PointerScanModel : public QAbstractTableModel {
    Q_OBJECT

public:
    PointerScanModel(Context* c, QObject *parent = Q_NULLPTR);

    /* Map of pointers (key) and addresses (value) */
    std::multimap<uintptr_t,uintptr_t> pointer_map;

    /* Map of pointers (key) and addresses (value) that are in a static area */
    std::multimap<uintptr_t,uintptr_t> static_pointer_map;

    /* Results of pointer scan */
    std::vector<std::pair<uintptr_t, std::vector<int>>> pointer_chains;

    /* Max size of pointer chain */
    int max_level = 5;

    /* Get the file and file offset from an address */
    std::string getFileAndOffset(uintptr_t addr, off_t& offset) const;

    /* Store all pointers from the game memory into a map */
    void locatePointers();

    /* Find all chains of pointers that start from a static address and
     * end with the specified address, in maximum `ml` levels and with a maximum
     * offset of `max_offset`
     */
    void findPointerChain(uintptr_t addr, int ml, int max_offset);

private:
    Context *context;

    /* File mapping sections */
    std::vector<MemSection> file_mapping_sections;

    /* Recursive call for the pointer chain search */
    void recursiveFind(uintptr_t addr, int level, int offsets[], int max_offset);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:
    void signalProgress(int);

};

#endif
