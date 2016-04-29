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
    return bytes + ((alignment - (bytes % alignment)) % alignment);
}

void AddressLinkedList::insertSorted(AddressLinkedList* item)
{
    item->head = head;
    AddressLinkedList* it = this;
    while (true)
    {
        if (item->address < it->address)
        {
            if (it->prev != nullptr && item->address < it->prev->address)
            {
                it = it->prev;
            }
            else
            {
                item->prev = it->prev;
                item->next = it;
                it->prev = item;
                if (item->prev == nullptr)
                    *(item->head) = item;
                break;
            }
        }
        else
        {
            if (it->next != nullptr && item->address > it->next->address)
            {
                it = it->next;
            }
            else
            {
                item->prev = it;
                item->next = it->next;
                it->next = item;
                break;
            }
        }
    }
}

void AddressLinkedList::unlink()
{
    if (prev != nullptr)
    {
        prev->next = next;
    }
    if (next != nullptr)
    {
        next->prev = prev;
    }
}

MemoryManager::MemoryManager(void)
{
    debuglogstdio(LCF_MEMORY, "%s constructor called!", __func__);
    if (!mminited) {
        debuglogstdio(LCF_MEMORY, "%s constructor init!", __func__);
        memory_objects = nullptr;
        mminited = true;
    }
}

/*
 * A 0 / nullptr value of a parameter means "any"
 * Returns nullptr if block was found.
 */
MemoryBlockDescription* MemoryManager::findBlock(const uint8_t* address,
        int bytes,
        int object_flags,
        int block_flags)
{
    /*
     * Make sure only relevant flags are set.
     */
    object_flags &= MemoryManager::ALLOC_EXECUTE |
        MemoryManager::ALLOC_READONLY |
        MemoryManager::ALLOC_WRITE;

    MemoryBlockDescription* rv = nullptr;

    for (MemoryObjectDescription* mod = memory_objects;
            mod != nullptr;
            mod = static_cast<MemoryObjectDescription*>(mod->next))
    {
        uint8_t* mod_end_address = mod->address + mod->bytes;
        if (((address != nullptr && address >= mod->address && address < mod_end_address) || address == nullptr) &&
                ((object_flags != 0 && object_flags == mod->flags) || object_flags == 0))
        {
            for (MemoryBlockDescription* mbd = mod->blocks;
                    mbd != nullptr;
                    mbd = static_cast<MemoryBlockDescription*>(mbd->next))
            {
                if (((address != nullptr && address == mbd->address) || address == nullptr) &&
                        ((bytes != 0 && bytes <= mbd->bytes) || bytes == 0) &&
                        ((block_flags != 0 && block_flags == mbd->flags) || block_flags == 0))
                {
                    if (rv == nullptr || rv->bytes > mbd->bytes)
                    {
                        rv = mbd;
                    }
                    if (bytes == rv->bytes)
                    {
                        goto memory_block_search_done;
                    }
                }
            }
        }
    }
memory_block_search_done:
    return rv;
}

uint8_t* MemoryManager::allocateInExistingBlock(int bytes, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with bytes %d", __func__, bytes);
    bytes = makeBytesAligned(bytes, 16);
    MemoryBlockDescription* best_block = findBlock(nullptr, bytes, flags, MemoryBlockDescription::FREE);

    if (best_block == nullptr)
    {
        return 0;
    }

    best_block->flags = MemoryBlockDescription::USED;
    if (best_block->bytes > bytes + size_of_mbd)
    {
        uint8_t* free_space = best_block->address + bytes;

        MemoryBlockDescription* mbd = reinterpret_cast<MemoryBlockDescription*>(free_space);
        mbd->address = free_space + size_of_mbd;
        mbd->bytes = best_block->bytes - (bytes + size_of_mbd);
        mbd->flags = MemoryBlockDescription::FREE;
        mbd->top = best_block->top;

        best_block->bytes = bytes;

        best_block->insertSorted(mbd);
    }

    if (flags & MemoryManager::ALLOC_ZEROINIT)
    {
        memset(static_cast<void*>(best_block->address), 0, best_block->bytes);
    }

    return best_block->address;
}

uint8_t* MemoryManager::allocateWithNewBlock(int bytes, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with bytes %d", __func__, bytes);

    /*
     * Calculate the size of the mapped file and make sure the allocation is a multible of
     * 16 bytes.
     */
    size_t block_size = allocation_granularity;
    int bytes_for_mod_and_mbd = sizeof(MemoryObjectDescription) + sizeof(MemoryBlockDescription);
    bytes_for_mod_and_mbd = makeBytesAligned(bytes_for_mod_and_mbd, 16);
    bytes = makeBytesAligned(bytes, 16);
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
        memset(addr, 0, bytes + bytes_for_mod_and_mbd);
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
        memory_objects = mod;
    }
    else
    {
        memory_objects->insertSorted(mod);
    }

    debuglogstdio(LCF_MEMORY, "Create new MOD of address %p and size %d", mod->address, mod->bytes);
    addr = static_cast<void*>(mod->address + sizeof(MemoryObjectDescription));

    MemoryBlockDescription* mbd = static_cast<MemoryBlockDescription*>(addr);
    mbd->address = mod->address + bytes_for_mod_and_mbd;
    mbd->bytes = bytes;
    mbd->flags = MemoryBlockDescription::USED;
    mbd->top = mod;
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
        addr = static_cast<void*>(mod->address + bytes + bytes_for_mod_and_mbd);
        mbd = static_cast<MemoryBlockDescription*>(addr);
        int free_space = block_size - (bytes + bytes_for_mod_and_mbd + size_of_mbd);

        mbd->address = mod->address + bytes + bytes_for_mod_and_mbd + size_of_mbd;
        mbd->bytes = free_space;
        mbd->flags = MemoryBlockDescription::FREE;
        mbd->top = mod;

        mod->blocks->insertSorted(mbd);
        debuglogstdio(LCF_MEMORY, "Create new free MBD of address %p and size %d", mbd->address, mbd->bytes);
    }

    file_size += block_size;
    return mod->blocks->address;
}

uint8_t* MemoryManager::allocateUnprotected(int bytes, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with bytes %d", __func__, bytes);
    //dumpAllocationTable();

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
        uint8_t* allocation = allocateInExistingBlock(bytes, flags);
        if (allocation != nullptr)
        {
            return allocation;
        }
    }
    return allocateWithNewBlock(bytes, flags);
}

uint8_t* MemoryManager::reallocateUnprotected(uint8_t* address, int bytes, int flags)
{
    debuglogstdio(LCF_MEMORY, "%s call with address %p and bytes %d", __func__, address, bytes);

    if (address == nullptr)
    {
        return allocateUnprotected(bytes, flags);
    }
    if (bytes == 0)
    {
        deallocateUnprotected(address);
        return nullptr;
    }

    int realloc_bytes = makeBytesAligned(bytes, 16);

    MemoryBlockDescription* block = findBlock(address, 0, flags, MemoryBlockDescription::USED);

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

            block->bytes = bytes;

            block->insertSorted(new_mbd);

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
        if (mbd != nullptr &&
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
                mbd->unlink();
            }
            /*
             * Adjust MemoryBlockDescription and update it.
             */
            else
            {
                debuglogstdio(LCF_MEMORY, "  larger size");
                uint8_t* addr = mbd->address - size_of_mbd + adjustment;
                MemoryBlockDescription* new_mbd = reinterpret_cast<MemoryBlockDescription*>(addr);
                mbd->unlink();
                memmove(new_mbd, mbd, sizeof(MemoryBlockDescription));

                new_mbd->address = addr + size_of_mbd;
                new_mbd->bytes -= adjustment;

                block->bytes += adjustment;
                block->insertSorted(new_mbd);
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
    uint8_t* allocation = allocateUnprotected(bytes, flags);
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

    if (address == nullptr)
    {
        return;
    }

    MemoryBlockDescription* block = findBlock(address, 0, 0, MemoryBlockDescription::USED);
    if (block == nullptr)
    {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: Attempted removal of unknown memory!");
        return;
    }
    block->flags = MemoryBlockDescription::FREE;

    /*
     * Attempt block merging.
     */
    if (block->next != nullptr && block->next->flags == MemoryBlockDescription::FREE)
    {
        block->bytes += block->next->bytes + size_of_mbd;
        block->next->unlink();
    }
    if (block->prev != nullptr && block->prev->flags == MemoryBlockDescription::FREE)
    {
        block = static_cast<MemoryBlockDescription*>(block->prev);
        block->bytes += block->next->bytes + size_of_mbd;
        block->next->unlink();
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

    size_of_mbd = makeBytesAligned(sizeof(MemoryBlockDescription), 16);
    allocation_granularity = getpagesize();
    allocation_lock.clear();
    file_size = 0;
    mminited = true;
}

void* MemoryManager::allocate(int bytes, int flags)
{
    if (!mminited)
        init();

    while (allocation_lock.test_and_set() == true) {}
    uint8_t* rv = allocateUnprotected(bytes, flags);
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void* MemoryManager::reallocate(void* address, int bytes, int flags)
{
    while (allocation_lock.test_and_set() == true) {}
    uint8_t* rv = reallocateUnprotected(static_cast<uint8_t*>(address), bytes, flags);
    allocation_lock.clear();
    if (!rv)
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "WARNING: returning null pointer!");
    return static_cast<void*>(rv);
}

void MemoryManager::deallocate(void* address)
{
    while (allocation_lock.test_and_set() == true) {}
    deallocateUnprotected(static_cast<uint8_t*>(address));
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
        findBlock(static_cast<const uint8_t*>(address), 0, 0, MemoryBlockDescription::USED);

    if (mbd != nullptr)
    {
        rv = mbd->bytes;
    }

    allocation_lock.clear();
    return rv;
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

