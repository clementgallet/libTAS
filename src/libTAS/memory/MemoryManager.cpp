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

#include <unistd.h> // getpagesize()
#include <cstring>
#include <cstdint>
#include <sys/mman.h>
#include <fcntl.h>
#include "../logging.h"

#include "MemoryManager.h"

/* Did we initialize our memory manager? *Must* be global */
bool mminited = false;

/*
 * Return a multiple of alignment just above bytes
 * Only works if alignment is a power of two.
 */
static uint32_t makeBytesAligned(uint32_t bytes, uint32_t alignment)
{
    if (alignment == 0)
        return bytes;
    if ((bytes & (alignment - 1)) == 0)
        return bytes;
    return alignment + (bytes & ~(alignment-1));
}

/* Returns an integer that is different from a, b and 0 */
static uint8_t newId(uint8_t a, uint8_t b) {
    uint8_t c;  
    for (c = a + 1; c == b || c == 0; ++c);
    return c;
}

uint8_t* MemoryManager::allocateInExistingBlock(uint32_t size, int flags, int align) {
    debuglogstdio(LCF_MEMORY, "%s call with bytes %d", __func__, size);
    size = makeBytesAligned(static_cast<intptr_t>(size), global_align);

    /* iterate blocks */
    for (MemoryObjectDescription* mod = fmod; mod; mod = mod->next) {
        /* check if block has enough room */
        if (mod->size - (mod->used * mod->bsize) >= size) {

            //debuglogstdio(LCF_MEMORY, "Possibly enough room");
            uint32_t bcnt = mod->size / mod->bsize;
            uint32_t bneed = ((size - 1) / mod->bsize) + 1; // ceil(size / mod->bsize)
            uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;

            uint8_t wrap = 0;
            for (uint32_t x = mod->lfb + 1; wrap == 0 || (wrap == 1 && x <= mod->lfb); ++x) {
                /* just wrap around */
                if (x >= bcnt) {
                    wrap++;
                    x = 0;
                }

                if (bm[x])
                    continue;

                /* check alignment here */
                if (align && (((x * mod->bsize + reinterpret_cast<uintptr_t>(mod) + size_of_mod) & (static_cast<uintptr_t>(align)-1)) != 0))
                    continue;

                /* count free blocks */
                uint32_t y;
                for (y = 0; bm[x + y] == 0 && y < bneed && (x + y) < bcnt; ++y);

                /* we have enough, now allocate them */
                if (y == bneed) {
                    /* find ID that does not match left or right */

                    /* bm[x + y] can overflow on the data,
                     * but it should not be a problem */
                    uint8_t nid = newId(bm[x - 1], bm[x + y]);

                    /* allocate by setting id */
                    for (uint32_t z = 0; z < bneed; ++z) {
                        bm[x + z] = nid;
                    }

                    /* optimization */
                    mod->lfb = (x + bneed) - 2;

                    /* count used blocks NOT bytes */
                    mod->used += bneed;

                    uint8_t* addr = reinterpret_cast<uint8_t*>(mod) + size_of_mod + x * mod->bsize;

                    if (flags & MemoryManager::ALLOC_ZEROINIT) {
                        memset(addr, 0, size);
                    }

                    return addr;
                }

                /* x will be incremented by one ONCE more in our FOR loop */
                x += (y - 1);
            }
        }
    }

    return nullptr; 
}

void MemoryManager::newBlock(uint32_t size, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);

    /*
     * Calculate the size of the mapped file and make sure the allocation is a multible of
     * global_align bytes.
     */
    size_t block_size = allocation_granularity;
    size = makeBytesAligned(size, global_align);
    while (block_size - size_of_mod - makeBytesAligned(block_size / global_align, global_align) < size)
    {
        block_size += allocation_granularity;
    }

    int access = 0;
    if (flags & MemoryManager::ALLOC_WRITE)
    {
        access = PROT_READ | PROT_WRITE;
    }
    else if (flags & MemoryManager::ALLOC_READONLY)
    {
        access = PROT_READ;
    }
    else
    {
        //DLL_ASSERT(false);
    }
    if (flags & MemoryManager::ALLOC_EXECUTE)
    {
        access |= PROT_EXEC;
    }

    if (ftruncate(fd, file_size + block_size) == -1) {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Could not extend shared memory file");
        return;
    }

    void* addr = mmap(0, block_size, access, MAP_SHARED, fd, file_size);
    file_size += block_size;

    if (addr == MAP_FAILED)
    {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Could not create shared memory block");
        return;
    }

    MemoryObjectDescription* mod = static_cast<MemoryObjectDescription*>(addr);
    mod->size = block_size - size_of_mod;
    mod->bsize = global_align;
    mod->next = fmod;
    fmod = mod;

    uint32_t bcnt = mod->size / mod->bsize;
    uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;

    /* Clear bitmap */
    for (uint32_t x = 0; x < bcnt; x++)
        bm[x] = 0;

    /* Reserve room for bitmap */
    bcnt = ((bcnt - 1) / mod->bsize) + 1; // ceil(bcnt / mod->bsize)
    for (uint32_t x = 0; x < bcnt; x++)
        bm[x] = 5;

    /* Check real remaining size */
    uint32_t remaining_size = mod->size - bcnt * mod->bsize;
    if (remaining_size < size)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Did not allocate enough size in the new block: %d and %d requested", remaining_size, size);

    mod->lfb = bcnt - 1;
    mod->used = bcnt;
    debuglogstdio(LCF_MEMORY, "Create new MOD of address %p and size %d", addr, mod->size);

}

uint8_t* MemoryManager::allocateUnprotected(uint32_t size, int flags, int align)
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);

    if (size == 0) {
        return nullptr;
    }
    /*
     * If the allocation needs 50% or more of a block, go preferably for
     * a new block (see below). Otherwise, always try to allocate in 
     * existing blocks.
     */
    if ((size * 2) < allocation_granularity) {
        uint8_t* allocation = allocateInExistingBlock(size, flags, align);
        if (allocation) {
            return allocation;
        }
    }
    else {
        /*
         * Before going directly for a new block, try only the first
         * (most recent) memory block and check for size only.
         * If enough size, try to go for allocation in existing memory.
         * This allows to reuse blocks more often, resulting in fewer
         * distinct mmap allocations (from >10000 to <1000).
         *
         * Always checking for existing free blocks (which has a higher
         * chance of failing for big allocation size) would lead to
         * a huge speed cost. So this strategy gives a decent balance
         * between performance and memory waste.
         * However, there is still significant memory wasted compared to
         * always check for existing memory.
         * We may learn from strategies of common malloc allocations.
         */
        if (fmod && (fmod->size - (fmod->used * fmod->bsize) >= size)) {
            uint8_t* allocation = allocateInExistingBlock(size, flags, align);
            if (allocation) {
                return allocation;
            }
        }
    }
    /*
     * Reserving the double size does actually saves a lot of memory and
     * reduces the number of distinct allocating blocks.
     * There might be more advanced optimizations, like using bins of memory
     * size, or using larger block size for big allocations. For now, this
     * seems to do fine.
     */
    newBlock(2 * size, flags); // Reserving double size
    return allocateInExistingBlock(size, flags, align);
}

uint8_t* MemoryManager::reallocateUnprotected(uint8_t* address, uint32_t size, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with address %p and size %d", __func__, address, size);

    if (address == nullptr) {
        return allocateUnprotected(size, flags, 0);
    }
    if (size == 0) {
        deallocateUnprotected(address);
        return nullptr;
    }

    for (MemoryObjectDescription *mod = fmod; mod; mod = mod->next) {
        uint8_t* mod_addr = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
        if (address > mod_addr && address < mod_addr + mod->size) {
            /* found block */
            intptr_t ptroff = reinterpret_cast<intptr_t>(address) - reinterpret_cast<intptr_t>(mod) - size_of_mod;  /* get offset to get block */
            /* block offset in BM */
            uint32_t bi = ptroff / mod->bsize;
            /* .. */
            uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
            /* clear allocation */
            uint8_t id = bm[bi];
            /* Check if we deallocate in the middle of a segment */
            if (bm[bi-1] == id)
                debuglogstdio(LCF_MEMORY | LCF_ERROR, "Deallocate in the middle of a segment!");

            uint32_t rb = (size - 1) / mod->bsize + 1; // ceil(size / mod->bsize)

            /* Check if we have enough space for the new size */
            uint32_t max = mod->size / mod->bsize;
            uint32_t x;
            for (x = bi; bm[x] == id && x < max && x - bi < rb; ++x) {}

            uint32_t x0 = x;
            if (x - bi == rb) {
                /* reallocate to smaller size. Just reset the rest of the bitmap */
                for (; bm[x] == id && x < max; ++x) {
                    bm[x] = 0;
                }
                /* update free block count */
                mod->used -= x - x0;
                return address;
            }

            /* Continuing on free blocks */
            for (; bm[x] == 0 && x < max && x - bi < rb; ++x) {}

            if (x - bi == rb) {
                /* Reallocate to larger size that fits in the empty blocks */

                /*
                 * Warning! we must check the id of the next segment. It might
                 * be a used block with the same id. In that case, we must
                 * change the id of the current segment
                 */
                if (x < max && bm[x] == id) {
                    /* Detected a segment with same id */
                    uint8_t nid = newId(bm[bi - 1], bm[x]);
                    for (x = bi; x - bi < rb; ++x) {
                        bm[x] = nid;
                    }
                }
                else {
                    /* Just extend the id on the free blocks */
                    for (x = x0; x - bi < rb; ++x) {
                        bm[x] = id;
                    }
                }

                /* update free block count */
                mod->used += x - x0;
                return address;
            }

            /* No space left for realloc. Deallocing and alloc elsewhere */
            for (x = bi; bm[x] == id && x < max; ++x) {
                bm[x] = 0;
            }

            /* update free block count */
            mod->used -= x - bi;

            uint8_t* newaddr = allocateUnprotected(size, flags, 0);
            if (!newaddr) {
                return nullptr;
            }

            memmove(newaddr, address, (x - bi) * mod->bsize);

            return newaddr;
        }
    }

    debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: Attempted realloc of unknown memory!");
    return nullptr;

}

void MemoryManager::deallocateUnprotected(uint8_t* address)
{
    debuglogstdio(LCF_MEMORY, "%s call with address %p", __func__, address);

    if (address == nullptr) {
        return;
    }

    for (MemoryObjectDescription *mod = fmod; mod; mod = mod->next) {
        uint8_t* mod_addr = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
        if (address > mod_addr && address < mod_addr + mod->size) {
            /* found block */
            intptr_t ptroff = reinterpret_cast<intptr_t>(address) - reinterpret_cast<intptr_t>(mod) - size_of_mod;  /* get offset to get block */
            /* Check address alignment */
            if (ptroff % mod->bsize)
                debuglogstdio(LCF_MEMORY | LCF_ERROR, "Address not aligned to block size!");
            /* block offset in BM */
            uint32_t bi = ptroff / mod->bsize;
            /* .. */
            uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
            /* clear allocation */
            uint8_t id = bm[bi];
            /* Check if we deallocate in the middle of a segment */
            if (bm[bi-1] == id)
                debuglogstdio(LCF_MEMORY | LCF_ERROR, "Deallocate in the middle of a segment!");
            /* oddly.. GCC did not optimize this */
            uint32_t max = mod->size / mod->bsize;
            uint32_t x;
            for (x = bi; bm[x] == id && x < max; ++x) {
                bm[x] = 0;
            }
            /* update free block count */
            mod->used -= x - bi;
            //mod->lfb = bi - 1;
            return;
        }
    }

    debuglogstdio(LCF_MEMORY | LCF_ERROR, "Attempted removal of unknown memory!");
    return;
}

void MemoryManager::init()
{
    debuglogstdio(LCF_MEMORY, "%s call", __func__);
    fd = shm_open("/libtas", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "  could not open shared memory file");
        /* Error */
    }

    fmod = nullptr;
    global_align = 16;
    size_of_mod = makeBytesAligned(sizeof(MemoryObjectDescription), global_align);
    allocation_granularity = getpagesize();
    allocation_lock.clear();
    file_size = 0;
    mminited = true;
}

void* MemoryManager::allocate(int bytes, int flags, int align)
{
    if (!mminited)
        init();

    /* align *must* be 0 (default) or a power of two */
    if (align && (align & (align - 1))) {
        /* align is not a power of two, taking the next one */
        /* code taken from http://graphics.stanford.edu/~seander/bithacks.html */
        align |= align >> 1;
        align |= align >> 2;
        align |= align >> 4;
        align |= align >> 8;
        align |= align >> 16;
        align++;
    }

    while (allocation_lock.test_and_set() == true) {}
    uint8_t* rv = allocateUnprotected(bytes, flags, align);
    //dumpAllocationTable();
    //checkIntegrity();
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void* MemoryManager::reallocate(void* address, int bytes, int flags)
{
    while (allocation_lock.test_and_set() == true) {}
    uint8_t* rv = reallocateUnprotected(static_cast<uint8_t*>(address), bytes, flags);
    //checkIntegrity();
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void MemoryManager::deallocate(void* address)
{
    while (allocation_lock.test_and_set() == true) {}
    deallocateUnprotected(static_cast<uint8_t*>(address));
    //checkIntegrity();
    allocation_lock.clear();
}

void MemoryManager::checkIntegrity()
{
    for (MemoryObjectDescription *mod = fmod; mod; mod = mod->next) {
        uint32_t bused = 0;
        uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
        uint32_t max = mod->size / mod->bsize;
        if (bm[0] != 5)
            debuglogstdio(LCF_MEMORY | LCF_ERROR, "First block was corrupted");

        for (uint32_t x = 0; x < max; ++x) {
            if (bm[x] != 0)
                bused++;
        }
        if (mod->used != bused)
            debuglogstdio(LCF_MEMORY | LCF_ERROR, "Incorrect number of blocks for MOD %p. MOD: %d, count: %d", mod, mod->used, bused);
    }
}

void MemoryManager::dumpAllocationTable()
{
    for (MemoryObjectDescription *mod = fmod; mod; mod = mod->next) {
        debuglogstdio(LCF_MEMORY, "MOD %p of size %d", mod, mod->size);
        uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
        uint32_t max = mod->size / mod->bsize;
        uint8_t curid = bm[0];
        uint32_t curpos = 0;
        for (uint32_t x = 0; x < max; ++x) {
            if (bm[x] != curid) {
                /* End of segment, print info */
                if (curid == 0)
                    debuglogstdio(LCF_MEMORY, "Free segment of size %d", (x - curpos)*mod->bsize);
                else
                    debuglogstdio(LCF_MEMORY, "Segment id %d of size %d", curid, (x - curpos)*mod->bsize);

                curid = bm[x];
                curpos = x;
            }
        }
        /* Last segment */
        if (curid == 0)
            debuglogstdio(LCF_MEMORY, "Free segment of size %d", (max - curpos)*mod->bsize);
        else
            debuglogstdio(LCF_MEMORY, "Segment id %d of size %d", curid, (max - curpos)*mod->bsize);
    }
}

MemoryManager memorymanager;

