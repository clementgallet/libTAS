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

#ifndef LIBTAS_MEMSCANNERTHREAD_H_INCLUDED
#define LIBTAS_MEMSCANNERTHREAD_H_INCLUDED

#include "MemScanner.h"

#include <string>
#include <cstdint>

/* Store a section of the game memory */
class MemScannerThread {
    public:
        MemScannerThread(MemScanner& ms, int br, int er, uintptr_t ba, uintptr_t ea, off_t mo, uint64_t mem);
        ~MemScannerThread();

        /* Create the output files that are named based on the scanning thread id */
        void create_output_files();
        
        /* First scan that will store the full memory when user set 'unknown value' */
        void first_region_scan();

        /* First scan that will store memory and addresses because user compare
         * to some value */
        void first_address_scan();

        /* Subsequent scan when previous was unknown (full memory) */
        void next_scan_from_region();

        /* Subsequent scan when previous had memory and addresses (common case) */
        void next_scan_from_address();

        const MemScanner& memscanner; // Reference to the scanner controller
        int beg_region, end_region; // Range of memory regions to search into
        uintptr_t beg_address, end_address; // Range of memory addresses to search into
        
        off_t memory_offset; // Offset in the memory file pointed by the starting address
        uint64_t memory_size; // Size of the memory file portion to process (in bytes)
        
        uint64_t new_memory_size; // New size after the scan (in bytes)
        volatile uint64_t processed_memory_size; // Current processed size (in bytes), used for progress bar
        
        std::string addresses_path; // Output file of addresses
        std::string values_path; // Output file of values
        
        volatile bool finished; // indicate if scan is finished, used for progress bar
};

#endif
