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

namespace orig {
    static void (*free)(void*);
}

static intptr_t makeBytesAligned(intptr_t bytes, int alignment)
{
    if (alignment == 0)
        return bytes;
    return bytes + ((alignment - (bytes % alignment)) % alignment);
}

int AddressLinkedList::deltaAlign(int align)
{
    if (align) {
        intptr_t addr = reinterpret_cast<intptr_t>(address);
        intptr_t delta = makeBytesAligned(addr, align) - addr;
        return static_cast<int>(delta);
    }
    return 0;
}

void AddressLinkedList::insert(AddressLinkedList* all)
{
    all->head = head;
    all->next = next;
    next = all;
}

MemoryManager::MemoryManager(void)
{
    if (!mminited) {
        memory_objects = nullptr;
        mminited = true;
    }
}

/*
 * A 0 / nullptr value of a parameter means "any"
 * Returns nullptr if no block was found.
 */
MemoryBlockDescription* MemoryManager::findBlock(const uint8_t* address)
{
    MemoryObjectDescription* base_mod = memory_objects;
    MemoryObjectDescription* mod = memory_objects;
    do {
        uint8_t* mod_end_address = mod->address + mod->bytes;
        if (address >= mod->address && address < mod_end_address) {
            MemoryBlockDescription* base_mbd = mod->blocks;
            MemoryBlockDescription* mbd = mod->blocks;
            do {
                if (address == mbd->address) {
                    /* Update pointers to current mod and next mbd for optimisation */
                    memory_objects = mod;
                    mod->blocks = static_cast<MemoryBlockDescription*>(mbd->next);
                    return mbd;
                }
                mbd = static_cast<MemoryBlockDescription*>(mbd->next);
            } while (mbd != base_mbd);
        }
        mod = static_cast<MemoryObjectDescription*>(mod->next);
    } while (mod != base_mod);

    return nullptr;
}

/*
 * A 0 / nullptr value of a parameter means "any"
 * Returns nullptr if no block was found.
 */
MemoryBlockDescription* MemoryManager::findBlock(int bytes,
        int object_flags,
        int block_flags,
        int align)
{
    /*
     * Make sure only relevant flags are set.
     */
    object_flags &= MemoryManager::ALLOC_EXECUTE |
        MemoryManager::ALLOC_READONLY |
        MemoryManager::ALLOC_WRITE;

    MemoryBlockDescription* rv = nullptr;
    int best_size = 0;

    MemoryObjectDescription* base_mod = memory_objects;
    MemoryObjectDescription* mod = memory_objects;
    do {
        if (object_flags == 0 || object_flags == mod->flags) {
            MemoryBlockDescription* base_mbd = mod->blocks;
            MemoryBlockDescription* mbd = mod->blocks;
            do {
                /* In case we need an aligned address, we have to compute the real size of the block */
                int block_size = mbd->bytes - mbd->deltaAlign(align);

                if ((bytes == 0 || block_size >= bytes) &&
                        (block_flags == 0 || block_flags == mbd->flags))
                {
                    /* If we have a good size match, returning immediatly */
                    if (block_size < bytes * 2)
                    {
                        /* Update pointers to current mod and mbd for optimisation */
                        memory_objects = mod;
                        mod->blocks = mbd;
                        return mbd;
                    }

                    /* If we find a block by its size, we try to return the block with the smallest difference size */
                    if (rv == nullptr || block_size < best_size)
                    {
                        rv = mbd;
                        best_size = block_size;
                    }

                }
                mbd = static_cast<MemoryBlockDescription*>(mbd->next);
            } while (mbd != base_mbd);
        }
        mod = static_cast<MemoryObjectDescription*>(mod->next);
    } while (mod != base_mod);
    return rv;
}

uint8_t* MemoryManager::allocateInExistingBlock(int bytes, int flags, int align)
{
    debuglogstdio(LCF_MEMORY, "%s call with bytes %d", __func__, bytes);
    bytes = makeBytesAligned(static_cast<intptr_t>(bytes), global_align);
    MemoryBlockDescription* best_block = findBlock(bytes, flags, MemoryBlockDescription::FREE, align);

    if (best_block == nullptr)
    {
        return 0;
    }

    int da = best_block->deltaAlign(align);

    /* Update the returned block */
    best_block->flags = MemoryBlockDescription::USED;
    best_block->address = best_block->address + da;
    if (makeBytesAligned(reinterpret_cast<intptr_t>(best_block->address), align) != reinterpret_cast<intptr_t>(best_block->address))
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Address won't be aligned, bug somewhere!");

    best_block->bytes -= da;

    /* Check if there is enough space to build a new empty block.
     * Note: empty blocks have zero alignment!
     *
     * Before:
     *   |--------|--------------------------------------------------|
     *       bb                       bb->bytes
     *
     * Alignment:
     *   |--------|------|-------------------------------------------|
     *       bb      da               bb->bytes
     *
     * New block:
     *   |--------|------|------------------|--------|---------------|
     *       bb      da          bytes          mbd      mbd->bytes
     */
    if (best_block->bytes > bytes + size_of_mbd)
    {
        uint8_t* free_space = best_block->address + bytes;

        MemoryBlockDescription* mbd = reinterpret_cast<MemoryBlockDescription*>(free_space);
        mbd->address = free_space + size_of_mbd;
        mbd->bytes = best_block->bytes - (bytes + size_of_mbd);
        mbd->flags = MemoryBlockDescription::FREE;
        mbd->top = best_block->top;

        best_block->bytes = bytes;

        best_block->insert(mbd);
    }

    if (flags & MemoryManager::ALLOC_ZEROINIT)
    {
        memset(static_cast<void*>(best_block->address), 0, best_block->bytes);
    }

    return best_block->address;
}

uint8_t* MemoryManager::allocateWithNewBlock(int bytes, int flags, int align)
{
    debuglogstdio(LCF_MEMORY, "%s call with bytes %d and align %d", __func__, bytes, align);

    /*
     * Calculate the size of the mapped file and make sure the allocation is a multible of
     * global_align bytes.
     */
    size_t block_size = allocation_granularity;
    int bytes_for_mod_and_mbd = makeBytesAligned(static_cast<intptr_t>(size_of_mod + size_of_mbd), align);
    bytes = makeBytesAligned(static_cast<intptr_t>(bytes), global_align);
    while (block_size < static_cast<size_t>(bytes + bytes_for_mod_and_mbd))
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
        return nullptr;
    }

    void* addr = mmap(0, block_size, access, MAP_SHARED, fd, file_size);

    if (addr == MAP_FAILED)
    {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Could not create shared memory block");
        return nullptr;
    }

    if (flags & MemoryManager::ALLOC_ZEROINIT)
    {
        memset(addr, 0, block_size);
    }

    MemoryObjectDescription* mod = static_cast<MemoryObjectDescription*>(addr);
    mod->address = static_cast<uint8_t*>(addr);
    mod->object = file_size;
    mod->bytes = block_size;
    mod->flags = flags & (MemoryManager::ALLOC_EXECUTE |
            MemoryManager::ALLOC_READONLY |
            MemoryManager::ALLOC_WRITE);

    if (memory_objects == nullptr)
    {
        mod->head = reinterpret_cast<AddressLinkedList**>(&memory_objects);
        mod->next = mod;
        memory_objects = mod;
    }
    else
    {
        memory_objects->insert(mod);
    }

    debuglogstdio(LCF_MEMORY, "Create new MOD of address %p and size %d", mod->address, mod->bytes);
    addr = static_cast<void*>(mod->address + size_of_mod);

    MemoryBlockDescription* mbd = static_cast<MemoryBlockDescription*>(addr);
    mbd->address = mod->address + bytes_for_mod_and_mbd;
    /* Check alignment */
    if (makeBytesAligned(reinterpret_cast<intptr_t>(mbd->address), align) != reinterpret_cast<intptr_t>(mbd->address))
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Address won't be aligned, bug somewhere!");
    mbd->bytes = bytes;
    mbd->flags = MemoryBlockDescription::USED;
    mbd->top = mod;
    mbd->next = mbd;
    mbd->head = reinterpret_cast<AddressLinkedList**>(&mod->blocks);
    /*
     * mod->blocks is always an invalid pointer here.
     * There is also no need to insert this one.
     */
    mod->blocks = mbd;
    debuglogstdio(LCF_MEMORY, "Create new MBD of address %p and size %d", mbd->address, mbd->bytes);

    if (static_cast<size_t>(bytes + bytes_for_mod_and_mbd + size_of_mbd) < block_size)
    {
        /*
         * This block can be used for more than one allocation.
         */
        addr = static_cast<void*>(mbd->address + bytes);
        MemoryBlockDescription* freembd = static_cast<MemoryBlockDescription*>(addr);
        int free_space = block_size - (bytes + bytes_for_mod_and_mbd + size_of_mbd);

        freembd->address = mod->address + bytes + bytes_for_mod_and_mbd + size_of_mbd;
        freembd->bytes = free_space;
        freembd->flags = MemoryBlockDescription::FREE;
        freembd->top = mod;

        mbd->insert(freembd);
        debuglogstdio(LCF_MEMORY, "Create new free MBD of address %p and size %d", freembd->address, freembd->bytes);
    }
    else {
        /* We cannot build a free block, so we update the first block to take the whole size */
        mbd->bytes = block_size - bytes_for_mod_and_mbd;
    }

    file_size += block_size;
    return mbd->address;
}

uint8_t* MemoryManager::allocateUnprotected(int bytes, int flags, int align)
{
    debuglogstdio(LCF_MEMORY, "%s call with bytes %d", __func__, bytes);

    if (bytes == 0)
    {
        return nullptr;
    }
    /*
     * If the allocation needs 50% or more of a block, go immediately for a new block.
     * This includes allocations that needs more than one block.
     */
    if ((bytes * 2) < allocation_granularity)
    {
        uint8_t* allocation = allocateInExistingBlock(bytes, flags, align);
        if (allocation != nullptr)
        {
            return allocation;
        }
    }
    return allocateWithNewBlock(bytes, flags, align);
}

uint8_t* MemoryManager::reallocateUnprotected(uint8_t* address, int bytes, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with address %p and bytes %d", __func__, address, bytes);

    if (address == nullptr)
    {
        return allocateUnprotected(bytes, flags, 0);
    }
    if (bytes == 0)
    {
        deallocateUnprotected(address);
        return nullptr;
    }

    int realloc_bytes = makeBytesAligned(static_cast<intptr_t>(bytes), global_align);

    MemoryBlockDescription* block = findBlock(address);

    if (block == nullptr)
    {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: Attempted realloc of unknown memory!");
        return nullptr;
    }

    /*
     * Attempt to adjust at the current location.
     */
    if (realloc_bytes < allocation_granularity && block != nullptr)
    {
        int adjustment = (realloc_bytes - block->bytes);

        /*
         * No difference.
         */
        if (adjustment == 0)
        {
            return block->address;
        }
        /*
         * Reallocating to smaller buffer and it would actually free a useful memory block.
         */
        if (adjustment + size_of_mbd < 0)
        {
            debuglogstdio(LCF_MEMORY, "  smaller size and new free block");
            uint8_t* addr = block->address + realloc_bytes;
            MemoryBlockDescription *new_mbd = reinterpret_cast<MemoryBlockDescription*>(addr);
            new_mbd->address = addr + size_of_mbd;
            new_mbd->bytes = block->bytes - realloc_bytes - size_of_mbd;
            new_mbd->flags = MemoryBlockDescription::FREE;

            block->bytes = realloc_bytes;

            block->insert(new_mbd);

            return block->address;
        }
        /*
         * The memory block that would be free'd is smaller than anything that can fit there.
         * Do nothing.
         */
        if (adjustment < 0)
        {
            debuglogstdio(LCF_MEMORY, "  smaller size");
            return block->address;
        }

        /*
         * Expand block.
         */
        MemoryBlockDescription* mbd = static_cast<MemoryBlockDescription*>(block->next);
        if (mbd->address > block->address &&
                adjustment <= mbd->bytes + size_of_mbd &&
                mbd->flags == MemoryBlockDescription::FREE)
        {
            /*
             * The remaining part of the block is too small for anything, give the full block.
             */
            if (adjustment >= mbd->bytes)
            {
                debuglogstdio(LCF_MEMORY, "  larger size to full block");
                block->bytes += mbd->bytes + size_of_mbd;
                /* 
                 * Removing mbd from the list, and checking if it's the
                 * reference in the mod. In that case, switching to block.
                 */
                block->next = mbd->next;
                if (*block->head == mbd)
                    *block->head = block;
            }
            /*
             * Adjust MemoryBlockDescription and update it.
             */
            else
            {
                debuglogstdio(LCF_MEMORY, "  larger size");
                uint8_t* addr = mbd->address - size_of_mbd + adjustment;
                MemoryBlockDescription* new_mbd = reinterpret_cast<MemoryBlockDescription*>(addr);

                //mbd->unlink();
                memmove(new_mbd, mbd, size_of_mbd);

                new_mbd->address = addr + size_of_mbd;
                new_mbd->bytes -= adjustment;

                block->bytes += adjustment;
                block->next = new_mbd;
                if (*block->head == mbd)
                    *block->head = new_mbd;
            }

            if (flags & MemoryManager::ALLOC_ZEROINIT)
            {
                memset(block->address, 0, block->bytes);
            }

            return block->address;
        }
    }
    /*
     * Changing the base pointer is not allowed, return nullptr.
     */
    if (flags & MemoryManager::REALLOC_NO_MOVE)
    {
        return nullptr;
    }
    /*
     * Adjustment is not possible, allocate somewhere else.
     */
    debuglogstdio(LCF_MEMORY, "  allocate elsewhere");
    uint8_t* allocation = allocateUnprotected(bytes, flags, 0);
    if (allocation == nullptr)
    {
        return nullptr;
    }
    if (bytes < block->bytes) {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "  cannot copy all the realloc memory");
        memcpy(allocation, address, bytes);
    }
    else
        memcpy(allocation, address, block->bytes);
    deallocateUnprotected(address);
    return allocation;
}

void MemoryManager::deallocateUnprotected(uint8_t* address)
{
    debuglogstdio(LCF_MEMORY, "%s call with address %p", __func__, address);
    //dumpAllocationTable();

    if (address == nullptr)
    {
        return;
    }

    MemoryBlockDescription* block = findBlock(address);
    if (block == nullptr)
    {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: Attempted removal of unknown memory!");
        //LINK_NAMESPACE(free, nullptr);
        //orig::free(address);
        return;
    }

    /* Flag the block as free. Also, remove any alignment */
    block->flags = MemoryBlockDescription::FREE;
    uint8_t* base_addr = reinterpret_cast<uint8_t*>(block);
    intptr_t delta = reinterpret_cast<intptr_t>(block->address) - reinterpret_cast<intptr_t>(base_addr + size_of_mbd);
    block->address = base_addr + size_of_mbd;
    block->bytes += delta;

    /*
     * Attempt block merging.
     */
    if (block->next->address > block->address && block->next->flags == MemoryBlockDescription::FREE)
    {
        block->bytes += block->next->bytes + size_of_mbd;

        /* Remove block->next from the linked list.
         * Also, check the reference mbd in the mod.
         */
        if (*block->head == block->next)
            *block->head = block->next->next;
        block->next = block->next->next;
    }

#if 0
    /* Entire memory block deallocated. */
    if (block->prev == nullptr && block->next == nullptr && block->flags == MemoryBlockDescription::FREE)
    {
        debuglogstdio(LCF_MEMORY, "  unmap a block");
        MemoryObjectDescription* mod = static_cast<MemoryObjectDescription*>(block->top);

        mod->unlink();

        munmap(mod->address, mod->bytes);
    }
#endif
}

void MemoryManager::init()
{
    debuglogstdio(LCF_MEMORY, "%s call", __func__);
    fd = shm_open("/libtas", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "  could not open shared memory file");
        /* Error */
    }

    global_align = 16;
    size_of_mbd = makeBytesAligned(static_cast<intptr_t>(sizeof(MemoryBlockDescription)), global_align);
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

size_t MemoryManager::getSizeOfAllocation(const void* address)
{
    if (address == nullptr)
    {
        return 0;
    }

    while (MemoryManager::allocation_lock.test_and_set() == true) {}
    size_t rv = 0;
    MemoryBlockDescription* mbd =
        findBlock(static_cast<const uint8_t*>(address));

    if (mbd != nullptr)
    {
        rv = mbd->bytes;
    }

    allocation_lock.clear();
    return rv;
}

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

MemoryManager memorymanager;
bool mminited = false;

