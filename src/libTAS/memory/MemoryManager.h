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

#ifndef LIBTAS_MEMORYMANAGER_H_INCLUDED
#define LIBTAS_MEMORYMANAGER_H_INCLUDED

#include <atomic>

/*
 * Memory Manager
 * --------------
 * This is a manager of dynamic allocations that allows us to have full control of the heap.
 * This manager uses a bitmap heap implementation that is described by Leonard Kevin McGuire Jr:
 * http://wiki.osdev.org/User:Pancakes/BitmapHeapImplementation
 * The principle is to store at the beginning of an allocated segment a map which holds information
 * about which blocks are free or used and to distinguish different allocations using ids.
 * As an optimization, it uses a next fit technique where new allocations are preferably done
 * next to recent allocations, in order to improve caching.
 * Moreover, memory is requested from the system using mmap and is tagged as shared, so that
 * it can easily be accessed from the executable, in order to have an effective RAM Search.
 * Another benefit to shared memory is that it becomes easier to manage save states with
 * these over regular RAM pages as we can have full READ / WRITE access from the executable side,
 * as well as shared ownership.
 */

/*
 * Structure of the header of a memory segment obtained from mmap.
 * Segments are stored as a linked list.
 */
struct MemoryObjectDescription
{
    /* Pointer to the next segment in the linked list */
    MemoryObjectDescription* next;

    /* Size of the segment (in bytes), excluding the size of the header */
    uint32_t size;

    /* Number of blocks used in the segment. Allow for faster search for free memory */
    uint32_t used;

    /* Size of a block of memory. We always allocate a multiple of bsize */
    uint32_t bsize;

    /* Index of the block from where we will start searching. Optimization purpose */
    uint32_t lfb;
};

/* Main class of memory management */
class MemoryManager
{
    public:
        /* Flags of memory segments. Only ZEROINIT is actually used */
        enum AllocationFlags
        {
            ALLOC_WRITE     = 0x00000001,
            ALLOC_READONLY  = 0x00000002,
            ALLOC_EXECUTE   = 0x00000004,
            ALLOC_ZEROINIT  = 0x00000008,
            REALLOC_NO_MOVE = 0x00000010,
        };

        void init();

        /*
         * Allocate memory of size bytes. Returned address must be a multiple
         * of align if align > 0, otherwise it is default aligned.
         */
        void* allocate(int size, int flags, int align);

        /*
         * Attempt to reallocate memory in-place, or allocate a new
         * memory segment and copy values in the new address
         */
        void* reallocate(void* address, int size, int flags);

        /* 
         * Deallocate a segment of memory using the base address.
         * We know how much size we need to deallocate
         */
        void deallocate(void* address);

        /*
         * Debug functions to print the current allocation layout,
         * and check for allocation corruption (very slow)
         */
        void dumpAllocationTable();
        void checkIntegrity();
    private:

        /*
         * Warning: we must *not* enter default value for parameters,
         * because they will be initialized too late, after that some
         * allocation has been done. Variable initialization is done
         * inside the init() function which is protected by a global variable,
         * as globals are intialized early enough.
         */

        /* Pointer to the first memory segment */
        MemoryObjectDescription* fmod;

        /* Pointer to the first memory segment */
        MemoryObjectDescription* lmod;

        /* Base memory size we can allocate with mmap. Usually the page size */
        uint32_t allocation_granularity;

        /* All our memory will be aligned using this variable.
         * The specifications required that the alignement is at least the size
         * of the biggest atomic type, so 8 bytes in our case.
         * However, the usual implementation of malloc (dlmalloc) uses
         * twice the size of a pointer, which is 16 bytes in a amd64 arch.
         * Also, I've seen some segfault when using 8 byte alignment on
         * strcpy code using SSE instructions that requires 16 bytes.
         *
         * To sum up, 8 bytes minimum, 16 bytes safer.
         */
        int global_align;

        /* Size of the MemoryObjectDescription struct, aligned with global_align */
        int size_of_mod;

        /* Our allocation must be tread-safe */
        std::atomic_flag allocation_lock;

        /* File descriptor of the mmap-ed file */
        int fd;

        /* Current offset in the mmap-ed file, from where we can allocate more memory */
        off_t file_size;

        /*
         * Internal allocation functions
         */

        /* Thread unsafe allocate */
        uint8_t* allocateUnprotected(uint32_t size, int flags, int align);

        /* Tries to allocate in current memory. Returns nullptr if not possible */
        uint8_t* allocateInExistingBlock(uint32_t size, int flags, int align);

        /* 
         * Request a new memory segment with mmap, that will contain at least
         * the asked size of free bytes (after removing header and bitmap)
         */
        void newBlock(uint32_t size, int flags);

        /* Thread unsafe reallocate */
        uint8_t* reallocateUnprotected(uint8_t* address, uint32_t size, int flags);

        /* Thread unsafe deallocate */
        void deallocateUnprotected(uint8_t* address);

};

extern MemoryManager memorymanager;

#endif

