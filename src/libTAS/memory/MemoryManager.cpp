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
			uint8_t* bm = reinterpret_cast<uint8_t*>(&mod[1]);
 
			for (uint32_t x = (mod->lfb + 1 >= bcnt ? 0 : mod->lfb + 1); x != mod->lfb; ++x) {
				/* just wrap around */
				if (x >= bcnt) {
					x = 0;
				}		
 
				if (bm[x] == 0) {
                    /* check alignment here */
                    /* TODO: alignment is a power of 2. Should be optimised */
                    if (!align || !((x * mod->bsize + reinterpret_cast<uintptr_t>(&mod[1])) % align)) {
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

                            return reinterpret_cast<uint8_t*>(&mod[1]) + x * mod->bsize;
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
    //int bytes_for_mod_and_mbd = makeBytesAligned(static_cast<intptr_t>(size_of_mod + size_of_mbd), align);
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
    uint8_t* bm = reinterpret_cast<uint8_t*>(&mod[1]);

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
            intptr_t ptroff = reinterpret_cast<intptr_t>(address) - reinterpret_cast<intptr_t>(&mod[1]);  /* get offset to get block */
            /* block offset in BM */
            uint32_t bi = ptroff / mod->bsize;
            /* .. */
            uint8_t* bm = reinterpret_cast<uint8_t*>(&mod[1]);
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
            intptr_t ptroff = reinterpret_cast<intptr_t>(address) - reinterpret_cast<intptr_t>(&mod[1]);  /* get offset to get block */
            /* block offset in BM */
            uint32_t bi = ptroff / mod->bsize;
            /* .. */
            uint8_t* bm = reinterpret_cast<uint8_t*>(&mod[1]);
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
    //checkAllocationTable();
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void* MemoryManager::reallocate(void* address, int bytes, int flags)
{
    while (allocation_lock.test_and_set() == true) {}
    uint8_t* rv = reallocateUnprotected(static_cast<uint8_t*>(address), bytes, flags);
    //checkAllocationTable();
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void MemoryManager::deallocate(void* address)
{
    while (allocation_lock.test_and_set() == true) {}
    deallocateUnprotected(static_cast<uint8_t*>(address));
    //checkAllocationTable();
    allocation_lock.clear();
}

#if 0
void MemoryManager::checkAllocationTable()
{
    MemoryObjectDescription* mod = memory_objects;
    MemoryBlockDescription* mbd = nullptr;
    while (mod != nullptr)
    {
        mbd = mod->blocks;
        uint8_t* mod_end;
        while (mbd != nullptr)
        {
            /*
             * Check address value based on MBD base address and size of MBD.
             * This is only relevent for free blocks which does not have alignment
             */
            if (mbd->flags == MemoryBlockDescription::FREE) {
                uint8_t* comp_addr = reinterpret_cast<uint8_t*>(mbd) + size_of_mbd;
                if (comp_addr != mbd->address)
                    debuglogstdio(LCF_MEMORY | LCF_ERROR, "Wrong MBD address %p, should be %p", mbd->address, comp_addr);
            }

            /* Check continuity with the next MBD */
            if (mbd->next) {
                uint8_t* next_mbd_addr = reinterpret_cast<uint8_t*>(mbd->next);
                if (mbd->address + mbd->bytes != next_mbd_addr)
                    debuglogstdio(LCF_MEMORY | LCF_ERROR, "Wrong MBD interval. Next MBD is %p, should be %p", next_mbd_addr, mbd->address + mbd->bytes);
            }
            else {
                /* Compute the end address of the last block */
                mod_end = mbd->address + mbd->bytes;
            }

            mbd = static_cast<MemoryBlockDescription*>(mbd->next);
        }
        
        /* Check end of MOD */
        if (mod_end != mod->address + mod->bytes)
            debuglogstdio(LCF_MEMORY | LCF_ERROR, "Wrong MOD end. %p from MOD and %p from last MBD", mod->address + mod->bytes, mod_end);

        mod = static_cast<MemoryObjectDescription*>(mod->next);
    }
}

void MemoryManager::dumpAllocationTable()
{
    MemoryObjectDescription* mod = memory_objects;
    MemoryBlockDescription* mbd = nullptr;
    while (mod != nullptr)
    {
        mbd = mod->blocks;
        debuglogstdio(LCF_MEMORY, "MOD this=%p addr=%p bytes=%d flags=%X blocks=%p", mod, mod->address, mod->bytes, mod->flags, mod->blocks);
        while (mbd != nullptr)
        {
            debuglogstdio(LCF_MEMORY, "MBD this=%p addr=%p bytes=%d flags=%X", mbd, mbd->address, mbd->bytes, mbd->flags);
            mbd = static_cast<MemoryBlockDescription*>(mbd->next);
        }
        mod = static_cast<MemoryObjectDescription*>(mod->next);
    }
}
#endif
MemoryManager memorymanager;
bool mminited = false;

