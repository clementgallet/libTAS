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

#include "MemAccess.h"
#include "MemLayout.h"
#include "MemScanner.h"
#include "MemScannerThread.h"
#include "MemValue.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <thread>

std::string MemScanner::memscan_path;
std::string MemScanner::addresses_path;
std::string MemScanner::values_path;

void MemScanner::init(std::string path)
{
    memscan_path = path;
    
    std::ostringstream ossov;
    ossov << memscan_path << "/memory.bin";
    values_path = ossov.str();

    std::ostringstream ossoa;
    ossoa << memscan_path << "/addresses.bin";
    addresses_path = ossoa.str();
}

int MemScanner::first_scan(pid_t pid, int mem_flags, int type, int align, CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv)
{
    value_type = type;
    value_type_size = MemValue::type_size(value_type);
    alignment = align;
    if (alignment == 0)
        alignment = value_type_size;
    else if (alignment > value_type_size)
        alignment = value_type_size;

    /* Read the whole memory layout */
    std::unique_ptr<MemLayout> memlayout (new MemLayout(pid));

    memsections.clear();
    
    MemSection section;
    total_size = 0;
    while (memlayout->nextSection(MemSection::MemAll, mem_flags, section)) {
        memsections.push_back(section);
        total_size += section.size;
    }
        
    if (total_size == 0) return MemScannerThread::ENOERROR;

    return scan(true, ct, co, cv, dv);
}

int MemScanner::scan(bool first, CompareType ct, CompareOperator co, MemValueType cv, MemValueType dv)
{
    is_stopped = false;

    compare_type = ct;
    compare_operator = co;
    compare_value = cv;
    different_value = dv;

    CompareOperations::init(value_type, compare_operator, compare_value, different_value);

    /* Split the work between threads */
    std::vector<MemScannerThread> memscanners;
    std::vector<std::thread> memscan_threads;
    uint64_t block_size = (total_size / THREAD_COUNT) & 0xfffffffffffff000;

    size_t beg_region = 0;
    size_t end_region = 0;
    uintptr_t beg_address = 0;
    uintptr_t end_address = 0;
    size_t cur_region_offset = 0;

    int thread_count = THREAD_COUNT;
    if (block_size == 0)
        thread_count = 1;
    
    for (int t = 0; t < thread_count-1; t++) {
        uint64_t cur_block_size = memsections[beg_region].size - cur_region_offset;    
        while ((cur_block_size < block_size) && (end_region < memsections.size())) {
            end_region++;
            cur_block_size += memsections[end_region].size;
        }
        
        cur_region_offset = memsections[end_region].size - (cur_block_size - block_size);
        end_address = memsections[end_region].addr + cur_region_offset;
        
        /* Sanitize beg and end addresses */
        if (beg_address < memsections[beg_region].addr)
            beg_address = memsections[beg_region].addr;
        if (end_address > memsections[end_region].endaddr)
            end_address = memsections[end_region].endaddr;

        /* Configure the scanner thread */
        memscanners.emplace_back(*this, beg_region, end_region, beg_address, end_address, t*block_size, block_size);
        
        /* Set the beg variables for the next thread */
        if (cur_block_size == block_size) {
            /* Nothing left in this section, skip to the beginning of the next section */
            beg_region = end_region + 1;
            beg_address = memsections[beg_region].addr;
        }
        else {
            beg_region = end_region;
            beg_address = end_address;
        }
    }
    
    /* Last scanner thread gets the remaining memory */
    end_region = memsections.size() - 1;
    end_address = memsections.back().endaddr;
    memscanners.emplace_back(*this, beg_region, end_region, beg_address, end_address, (thread_count-1)*block_size, total_size-((thread_count-1)*block_size));
    
    /* Start all threads */
    for (int t = 0; t < thread_count; t++) {
        if (first) {
            if (compare_type == CompareType::Previous)
                memscan_threads.emplace_back(&MemScannerThread::first_region_scan, &memscanners[t]);
            else
                memscan_threads.emplace_back(&MemScannerThread::first_address_scan, &memscanners[t]);            
        }
        else {
            if (last_scan_was_region)
                memscan_threads.emplace_back(&MemScannerThread::next_scan_from_region, &memscanners[t]);
            else
                memscan_threads.emplace_back(&MemScannerThread::next_scan_from_address, &memscanners[t]);
        }
    }

    /* Update progress bar */
    /* We would normally just join all threads, but we need to update the scan
     * state periodically to update the progress bar. So we check if all
     * scan threads have finished. */
    bool scan_finished = false;
    uint64_t total_processed_size = 0;
    while (!scan_finished) { 
        scan_finished = true;
        total_processed_size = 0;
        for (const auto& ms : memscanners) {
            total_processed_size += ms.processed_memory_size;
            scan_finished &= ms.finished;
        }
        emit signalProgress(total_processed_size);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    last_scan_was_region = (first && (compare_type == CompareType::Previous));

    /* Wait for the thread to finish, and read error codes. */
    total_size = 0;
    total_processed_size = 0;
    int error = 0;
    for (int t = 0; t < thread_count; t++) {
        memscan_threads[t].join();

        /* If the scanner thread encounter an error, don't read the file */
        if (memscanners[t].error < 0) {
            error = memscanners[t].error;
            continue;
        }

        /* Compute total size and processed size (for display) */
        total_size += memscanners[t].new_memory_size;
        total_processed_size += memscanners[t].processed_memory_size;
    }

    /* If user requested a stop or an error occured, skip the file merging
     * and report as if we didn't find any result */
    if (is_stopped && (error < 0)) {
        addresses.clear();
        old_values.clear();
        total_size = 0;
        return error;
    }

    /* Wait for each thread to finish and merge all individual files created
     * by each thread into a single file, and inside vectors to be displayed. */
    /* TODO: this can take a bit of time, do this in another thread if we don't
     * need to display values (above threshold).
     * Prevent a new search until the file merge is completed */
    std::ofstream ovfs(values_path, std::ofstream::binary);
    std::ofstream oafs;
    if (!last_scan_was_region) {
        oafs.open(addresses_path, std::ofstream::binary);
    }

    /* Reset progress bar to show progression of file merging */
    emit signalProgress(0);
    uint64_t cur_size = 0;

    for (int t = 0; t < thread_count; t++) {
        const MemScannerThread& mst = memscanners[t];
        
        /* For some reason, trying to append an empty file messes up the stream */
        if (memscanners[t].new_memory_size == 0)
            continue;
            
        /* Append memscan_thread file to unique file */
        if (!last_scan_was_region) {
            std::ifstream iafs(mst.addresses_path, std::ios_base::binary);
            oafs << iafs.rdbuf();
            uint64_t recorded_size = memscanners[t].new_memory_size * sizeof(uintptr_t) / value_type_size;
            if (iafs.tellg() != recorded_size) {
                std::cerr << "Mismatch size between recorded (" << recorded_size << ") and file (" <<  iafs.tellg() << ") sizes" << std::endl;
                error = MemScannerThread::EOUTPUT;
                break;
            }
            if (!oafs) {
                error = MemScannerThread::EOUTPUT;
                break;
            }
        }
        std::ifstream ivfs(mst.values_path, std::ios_base::binary);
        ovfs << ivfs.rdbuf();
        if (ivfs.tellg() != memscanners[t].new_memory_size) {
            std::cerr << "Mismatch size between recorded (" << memscanners[t].new_memory_size << ") and file (" <<  ivfs.tellg() << ") sizes" << std::endl;
            error = MemScannerThread::EOUTPUT;
            break;
        }
        if (!ovfs) {
            error = MemScannerThread::EOUTPUT;
            break;
        }
        
        if (is_stopped) {
            error = MemScannerThread::ESTOPPED;
            break;
        }

        /* Some math to reuse progress bar that has the old size for total */
        cur_size += memscanners[t].new_memory_size;
        emit signalProgress(cur_size*total_processed_size/total_size);
    }
    
    if (error < 0) {
        addresses.clear();
        old_values.clear();
        total_size = 0;
        return error;
    }

    /* If the total size is below threshold, load all data (except if region data) */
    if (last_scan_was_region) return MemScannerThread::ENOERROR;

    addresses.clear();
    old_values.clear();
    
    if (total_size < (DISPLAY_THRESHOLD*value_type_size)) {
        /* Close the streams before reading the files */
        oafs.close();
        ovfs.close();
        
        std::ifstream iafs(addresses_path, std::ios::in | std::ios::binary);
        addresses = std::vector<char>(std::istreambuf_iterator<char>(iafs), std::istreambuf_iterator<char>());

        std::ifstream ivfs(values_path, std::ios::in | std::ios::binary);
        old_values = std::vector<char>(std::istreambuf_iterator<char>(ivfs), std::istreambuf_iterator<char>());
    }
    
    return MemScannerThread::ENOERROR;
}

uint64_t MemScanner::scan_size() const
{
    return total_size;
}

uint64_t MemScanner::scan_count() const
{
    return total_size / value_type_size;
}

uint64_t MemScanner::display_scan_count() const
{
    return addresses.size() / sizeof(uintptr_t);
}

uintptr_t MemScanner::get_address(int index) const
{
    if (addresses.empty())
        return 0;
        
    return *reinterpret_cast<const uintptr_t*>(&addresses[index*sizeof(uintptr_t)]);
}

const char* MemScanner::get_previous_value(int index, bool hex) const
{
    if (old_values.empty())
        return "";
        
    return MemValue::to_string(&old_values[index*value_type_size], value_type, hex);
}

const char* MemScanner::get_current_value(int index, bool hex) const
{
    uintptr_t addr = get_address(index);
    uint8_t value[8];
    int readValues = MemAccess::read(value, reinterpret_cast<void*>(addr), value_type_size);
    if (readValues != value_type_size)
        return "";

    return MemValue::to_string(value, value_type, hex);
}

void MemScanner::clear()
{
    total_size = 0;
    addresses.clear();
    old_values.clear();
    memsections.clear();
}
