/*
 * (c) 2015- Hourglass Resurrection Team
 * Hourglass Resurrection is licensed under GPL v2.
 * Refer to the file COPYING.txt in the project root.
 */

//#include <Windows.h>
#include <unistd.h> // getpagesize()
#include <cstring>
#include <cstdint>
#include <sys/mman.h>
#include <fcntl.h>
#include <iomanip>
#include "../logging.h"

#include "MemoryManager.h"

static int makeBytesAligned(int bytes, int alignment)
{
    if (alignment == 0)
        return bytes;
    return bytes + ((alignment - (bytes % alignment)) % alignment);
}

MemoryManager::MemoryManager(void)
{
}

static uint8_t k_heapBMGetNID(uint8_t a, uint8_t b) {
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
			uint32_t bneed = (size / mod->bsize) * mod->bsize < size ? size / mod->bsize + 1 : size / mod->bsize;
			uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
 
			for (uint32_t x = (mod->lfb + 1 >= bcnt ? 0 : mod->lfb + 1); x != mod->lfb; ++x) {
				/* just wrap around */
				if (x >= bcnt) {
					x = 0;
				}		
 
				if (bm[x] == 0) {
                    /* check alignment here */
                    /* TODO: alignment is a power of 2. Should be optimised */
                    if (!align || !((x * mod->bsize + reinterpret_cast<uintptr_t>(mod) + size_of_mod) % align)) {
                        /* count free blocks */
                        uint32_t y;
                        for (y = 0; bm[x + y] == 0 && y < bneed && (x + y) < bcnt; ++y);
                        //debuglogstdio(LCF_MEMORY, "Found segment %d and need %d", y, bneed);

                        /* we have enough, now allocate them */
                        if (y == bneed) {
                            /* find ID that does not match left or right */
                            uint8_t nid = k_heapBMGetNID(bm[x - 1], bm[x + y]);

                            /* allocate by setting id */
                            for (uint32_t z = 0; z < y; ++z) {
                                bm[x + z] = nid;
                            }

                            /* optimization */
                            mod->lfb = (x + bneed) - 2;

                            /* count used blocks NOT bytes */
                            mod->used += y;

                            return reinterpret_cast<uint8_t*>(mod) + size_of_mod + x * mod->bsize;
                        }

                        /* x will be incremented by one ONCE more in our FOR loop */
                        x += (y - 1);
                        continue;
                    }
				}
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
    size = makeBytesAligned(static_cast<intptr_t>(size), global_align);
    while (block_size - size_of_mod - (block_size / global_align) < size)
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

    if (addr == MAP_FAILED)
    {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Could not create shared memory block");
        return;
    }

    if (flags & MemoryManager::ALLOC_ZEROINIT)
    {
        memset(addr, 0, block_size);
    }

    MemoryObjectDescription* mod = static_cast<MemoryObjectDescription*>(addr);
    mod->size = block_size - size_of_mod;
    mod->bsize = global_align;
    mod->next = fmod;
    fmod = mod;

    uint32_t bcnt = block_size / mod->bsize;
    uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;

    /* Clear bitmap */
    for (uint32_t x = 0; x < bcnt; x++)
        bm[x] = 0;

    /* Reserve room for bitmap */
    bcnt = (bcnt / mod->bsize) * mod->bsize < bcnt ? bcnt / mod->bsize + 1 : bcnt / mod->bsize;
    for (uint32_t x = 0; x < bcnt; x++)
        bm[x] = 5;

    mod->lfb = bcnt - 1;
    mod->used = bcnt;
    debuglogstdio(LCF_MEMORY, "Create new MOD of address %p and size %d", addr, mod->size);

    file_size += block_size;
}

uint8_t* MemoryManager::allocateUnprotected(uint32_t size, int flags, int align)
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);

    if (size == 0) {
        return nullptr;
    }
    /*
     * If the allocation needs 50% or more of a block, go immediately for a new block.
     * This includes allocations that needs more than one block.
     */
    if ((size * 2) < allocation_granularity)
    {
        uint8_t* allocation = allocateInExistingBlock(size, flags, align);
        if (allocation) {
            return allocation;
        }
    }
    newBlock(size, flags);
    return allocateInExistingBlock(size, flags, align);
}

uint8_t* MemoryManager::reallocateUnprotected(uint8_t* address, uint32_t size, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with address %p and size %d", __func__, address, size);

    if (address == nullptr)
    {
        return allocateUnprotected(size, flags, 0);
    }
    if (size == 0)
    {
        deallocateUnprotected(address);
        return nullptr;
    }


    for (MemoryObjectDescription *mod = fmod; mod; mod = mod->next) {
        uint8_t* mod_addr = reinterpret_cast<uint8_t*>(mod);
        if (address > mod_addr && address < mod_addr + mod->size) {
            /* found block */
            intptr_t ptroff = reinterpret_cast<intptr_t>(address) - reinterpret_cast<intptr_t>(mod) - size_of_mod;  /* get offset to get block */
            /* block offset in BM */
            uint32_t bi = ptroff / mod->bsize;
            /* .. */
            uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
            /* clear allocation */
            uint8_t id = bm[bi];
            /* oddly.. GCC did not optimize this */
            //uint32_t max = mod->size / mod->bsize;
            //for (uint32_t x = bi; bm[x] == id && x < max; ++x) {
            //    bm[x] = 0;
            //}


            int realloc_bytes = makeBytesAligned(static_cast<intptr_t>(size), mod->bsize);
            uint32_t rb = realloc_bytes / mod->bsize;

            /* Check if we have enough space for the new size */
            uint32_t max = mod->size / mod->bsize;
            uint32_t x;
            for (x = bi; bm[x] == id && x < max && x - bi < rb; ++x) {}

            if (x == bi + rb) {
                /* reallocate to smaller size. Just reset the rest of the bitmap */
                uint32_t x0 = x;
                for (; bm[x] == id && x < max; ++x) {
                    bm[x] = 0;
                }
                /* update free block count */
                mod->used -= x - x0;
                return address;
            }

            /* Continuing on free blocks */
            uint32_t x0 = x;
            for (; bm[x] == 0 && x < max && x - bi < rb; ++x) {}

            if (x == bi + rb) {
                /* Reallocate to larger size that fits in the empty blocks */
                for (x = x0; x - bi < rb; ++x) {
                    bm[x] = id;
                }
                /* update free block count */
                mod->used += x - x0;
                return address;
            }

            /* No space left for realloc. Deallocing and alloc elsewhere */
            for (x = bi; bm[x] == id && x < max; ++x) {
                bm[x] = 0;
            }

            uint8_t* newaddr = allocateUnprotected(size, flags, 0);
            if (!newaddr) {
                return nullptr;
            }

            memcpy(newaddr, address, (x - bi) * mod->bsize);

            /* update free block count */
            mod->used -= x - bi;
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
        uint8_t* mod_addr = reinterpret_cast<uint8_t*>(mod);
        if (address > mod_addr && address < mod_addr + mod->size) {
            /* found block */
            intptr_t ptroff = reinterpret_cast<intptr_t>(address) - reinterpret_cast<intptr_t>(mod) - size_of_mod;  /* get offset to get block */
            /* block offset in BM */
            uint32_t bi = ptroff / mod->bsize;
            /* .. */
            uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
            /* clear allocation */
            uint8_t id = bm[bi];
            /* oddly.. GCC did not optimize this */
            uint32_t max = mod->size / mod->bsize;
            uint32_t x;
            for (x = bi; bm[x] == id && x < max; ++x) {
                bm[x] = 0;
            }
            /* update free block count */
            mod->used -= x - bi;
            return;
        }
    }

    debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: Attempted removal of unknown memory!");
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
    size_of_mod = makeBytesAligned(static_cast<intptr_t>(sizeof(MemoryObjectDescription)), global_align);
    allocation_granularity = getpagesize();
    allocation_lock.clear();
    file_size = 0;
    mminited = true;
}

void* MemoryManager::allocate(int bytes, int flags, int align)
{
    if (!mminited)
        init();

    while (allocation_lock.test_and_set() == true) {}
    uint8_t* rv = allocateUnprotected(bytes, flags, align);
    dumpAllocationTable();
    checkIntegrity();
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void* MemoryManager::reallocate(void* address, int bytes, int flags)
{
    while (allocation_lock.test_and_set() == true) {}
    uint8_t* rv = reallocateUnprotected(static_cast<uint8_t*>(address), bytes, flags);
    checkIntegrity();
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void MemoryManager::deallocate(void* address)
{
    while (allocation_lock.test_and_set() == true) {}
    deallocateUnprotected(static_cast<uint8_t*>(address));
    checkIntegrity();
    allocation_lock.clear();
}

void MemoryManager::checkIntegrity()
{
    for (MemoryObjectDescription *mod = fmod; mod; mod = mod->next) {
        uint32_t bused = 0;
        uint8_t* bm = reinterpret_cast<uint8_t*>(mod) + size_of_mod;
        uint32_t max = mod->size / mod->bsize;
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
bool mminited = false;

