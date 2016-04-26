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
#include <atomic>
#include <iomanip>
#include "../logging.h"

#include "MemoryManager.h"
//#include <print.h>
//#include <Utils.h>

namespace
{
    int MakeBytesAligned(int bytes, int alignment)
    {
        return bytes + ((alignment - (bytes % alignment)) % alignment);
    }
}

namespace MemoryManagerInternal
{
    enum MemoryBlockState
    {
        MEMORY_BLOCK_USED = 0x00000001,
        MEMORY_BLOCK_FREE = 0x00000002,
    };

    /*
     * 'head' is a pointer to the variable containing the head pointer.
     * It must be kept valid at all times.
     * This saves us from having to keep track of which object the allocation belongs to when
     * allocating new sub-blocks.
     */
    struct AddressLinkedList
    {
        AddressLinkedList* prev;
        AddressLinkedList* next;
        //AddressLinkedList** head;
        uint8_t* address;
        int bytes;
        int flags;
    };

    /*
     * Helper functions
     */

    /*
     * The list-parameter can be any element in the list.
     */
    static void LinkedListInsertSorted(AddressLinkedList* list, AddressLinkedList* item)
    {
        //ENTER(list, item);
        //DLL_ASSERT(list != nullptr && item != nullptr && list->m_address != item->m_address);

        AddressLinkedList* it = list;
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

    static void LinkedListUnlink(AddressLinkedList* item)
    {
        //DLL_ASSERT(item != nullptr);
        if (item->prev != nullptr)
        {
            item->prev->next = item->next;
        }
        if (item->next != nullptr)
        {
            item->next->prev = item->prev;
        }
    }

    struct MemoryBlockDescription :
        public AddressLinkedList
    {
        AddressLinkedList* top;
    };

    struct MemoryObjectDescription :
        public AddressLinkedList
    {
        int object;
        MemoryBlockDescription* blocks;
    };

    bool memory_manager_inited = false;

    //uintptr_t minimum_allowed_address = nullptr;
    //uintptr_t maximum_allowed_address = nullptr;
    int allocation_granularity = 0;
    int size_of_mbd = MakeBytesAligned(sizeof(MemoryBlockDescription), 8);
    std::atomic_flag allocation_lock;

    MemoryObjectDescription* memory_objects = nullptr;

    /*
     * TODO: Once debuglog is rewritten, make this a debuglog maskable call
     * -- Warepire
     */
    void DumpAllocationTable();

    /*
     * Internal allocation functions
     */
    uint8_t* AllocateUnprotected(int bytes, int flags);
    uint8_t* AllocateInExistingBlock(int bytes, int flags);
    uint8_t* AllocateWithNewBlock(int bytes, int flags);

    uint8_t* ReallocateUnprotected(uint8_t* address, int bytes, int flags);

    void DeallocateUnprotected(uint8_t* address);

#if 0
    uint8_t* FindAllocationBaseAddress(uint32_t bytes, uint32_t flags)
    {
        MEMORY_BASIC_INFORMATION best_gap;
        memset(&best_gap, 0, sizeof(best_gap));
        MEMORY_BASIC_INFORMATION this_gap;
        uintptr_t current_address = const_cast<uintptr_t>(minimum_allowed_address);

        while (current_address < maximum_allowed_address)
        {
            VirtualQuery(current_address, &this_gap, sizeof(this_gap));
            current_address = static_cast<LPBYTE>(current_address) + allocation_granularity;
            if (this_gap.State == MEM_FREE && this_gap.RegionSize >= bytes &&
                (best_gap.RegionSize > this_gap.RegionSize || best_gap.RegionSize == 0))
            {
                best_gap = this_gap;
                /*
                 * Perfect match, break early.
                 */
                if (best_gap.RegionSize == bytes)
                {
                    break;
                }
            }
        }
        /*
         * No memory allocation possible, no space available to allocate at.
         */
        if (best_gap.RegionSize == 0)
        {
            return nullptr;
        }
        return best_gap.BaseAddress;
    }
#endif
    /*
     * A 0 / nullptr value of a parameter means "any"
     * Returns nullptr if block was found.
     */
    static MemoryBlockDescription* FindBlock(const uint8_t* address,
                                      int bytes,
                                      int object_flags,
                                      int block_flags)
    {
        //ENTER(address, bytes, object_flags, block_flags);
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
        //LEAVE(rv);
        return rv;
    }

    uint8_t* AllocateInExistingBlock(int bytes, int flags)
    {
        //ENTER(bytes, flags);
        bytes = MakeBytesAligned(bytes, 8);
        MemoryBlockDescription* best_block = FindBlock(nullptr, bytes, flags, MEMORY_BLOCK_FREE);

        if (best_block == nullptr)
        {
            return 0;
        }

        best_block->flags = MEMORY_BLOCK_USED;
        if (best_block->bytes > bytes + size_of_mbd)
        {
            uint8_t* free_space = best_block->address + bytes;

            MemoryBlockDescription* mbd = reinterpret_cast<MemoryBlockDescription*>(free_space);
            mbd->address = free_space + size_of_mbd;
            mbd->bytes = best_block->bytes - (bytes + size_of_mbd);
            mbd->flags = MEMORY_BLOCK_FREE;
            mbd->top = best_block->top;

            best_block->bytes = bytes;

            LinkedListInsertSorted(best_block, mbd);
        }

        if (flags & MemoryManager::ALLOC_ZEROINIT)
        {
            memset(static_cast<void*>(best_block->address), 0, best_block->bytes);
        }

        //LEAVE(best_block->m_address);
        return best_block->address;
    }

    uint8_t* AllocateWithNewBlock(int bytes, int flags)
    {
        /*
         * Calculate the size of the mapped file and make sure the allocation is a multible of
         * 8 bytes.
         */
        //ENTER(bytes, flags);
        static off_t file_size = 0; // TODO: Move it elsewhere
        size_t block_size = allocation_granularity;
        int bytes_for_mod_and_mbd = sizeof(MemoryObjectDescription) + sizeof(MemoryBlockDescription);
        bytes_for_mod_and_mbd = MakeBytesAligned(bytes_for_mod_and_mbd, 8);
        bytes = MakeBytesAligned(bytes, 8);
        while (block_size < static_cast<size_t>(bytes + bytes_for_mod_and_mbd))
        {
            block_size += allocation_granularity;
        }

        /*uintptr_t target_address = FindAllocationBaseAddress(block_size, flags);
        if (target_address == nullptr)
        {
            return nullptr;
        }*/

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

        /*
         * TODO: Decide a naming scheme, to easier ID segments in a save state.
         * -- Warepire
         */
        /*
         * Ensure GetLastError() returns an error-code from CreateFileMapping.
         */
        //SetLastError(ERROR_SUCCESS);
        int fd = shm_open("/libtas", O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            /* Error */
        }

        if (ftruncate(fd, file_size + block_size) == -1) {
            /* Error */
        }
/*
        HANDLE map_file = CreateFileMapping(INVALID_HANDLE_VALUE,
                                            nullptr,
                                            PAGE_EXECUTE_READWRITE,
                                            0,
                                            file_size,
                                            nullptr);
        if (GetLastError() == ERROR_ALREADY_EXISTS || map_file == nullptr)
        {
            return nullptr;
        }
        */
        void* addr = mmap(0, block_size, access, MAP_SHARED, fd, file_size);
            
        //uintptr_t addr = MapViewOfFileEx(map_file, access, 0, 0, file_size, target_address);
        if (addr == nullptr)
        {
            //CloseHandle(map_file);
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
            memory_objects = mod;
        }
        else
        {
            LinkedListInsertSorted(memory_objects, mod);
        }

        addr = static_cast<void*>(mod->address + sizeof(MemoryObjectDescription));

        MemoryBlockDescription* mbd = static_cast<MemoryBlockDescription*>(addr);
        mbd->address = mod->address + bytes_for_mod_and_mbd;
        mbd->bytes = bytes;
        mbd->flags = MEMORY_BLOCK_USED;
        mbd->top = mod;
        /*
         * mod->blocks is always an invalid pointer here.
         * There is also no need to insert this one.
         */
        mod->blocks = mbd;

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
            mbd->flags = MEMORY_BLOCK_FREE;
            mbd->top = mod;

            LinkedListInsertSorted(mod->blocks, mbd);
        }

        file_size += block_size;
        //LEAVE(mod->m_blocks->m_address);
        return mod->blocks->address;
    }

    uint8_t* AllocateUnprotected(int bytes, int flags)
    {
        //ENTER(bytes, flags);
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
            uint8_t* allocation = AllocateInExistingBlock(bytes * 2, flags); // TODO: Why *2 ??
            if (allocation != nullptr)
            {
                return allocation;
            }
        }
        return AllocateWithNewBlock(bytes, flags);
    }

    uint8_t* ReallocateUnprotected(uint8_t* address, int bytes, int flags)
    {
        //ENTER(address, bytes, flags);

        if (address == nullptr)
        {
            return AllocateUnprotected(bytes, flags);
        }
        if (bytes == 0)
        {
            DeallocateUnprotected(address);
            return nullptr;
        }

        int realloc_bytes = MakeBytesAligned(bytes * 2, 8); // TODO: Why *2 ??

        MemoryBlockDescription* block = FindBlock(address, 0, flags, MEMORY_BLOCK_USED);

        /*
         * Attempt to adjust at the current location.
         */
        if (realloc_bytes < allocation_granularity && block != nullptr)
        {
            MemoryBlockDescription* mbd = block;
            int adjustment = (realloc_bytes - block->bytes);

            /*
             * No difference.
             */
            if (adjustment == 0)
            
            {
                return mbd->address;
            }
            /*
             * Reallocating to smaller buffer and it would actually free a useful memory block.
             */
            if (adjustment + size_of_mbd < 0)
            {
                uint8_t* addr = mbd->address + realloc_bytes;
                MemoryBlockDescription *new_mbd = reinterpret_cast<MemoryBlockDescription*>(addr);
                new_mbd->address = addr + size_of_mbd;
                new_mbd->bytes = abs(adjustment);
                new_mbd->flags = MEMORY_BLOCK_FREE;

                mbd->bytes = bytes;

                LinkedListInsertSorted(mbd, new_mbd);

                return mbd->address;
            }
            /*
             * The memory block that would be free'd is smaller than anything that can fit there.
             * Do nothing.
             */
            if (adjustment < 0)
            {
                return mbd->address;
            }

            /*
             * Expand block.
             */
            mbd = static_cast<MemoryBlockDescription*>(mbd->next);
            if (mbd != nullptr &&
                mbd->bytes > adjustment &&
                mbd->flags == MEMORY_BLOCK_FREE)
            {
                uint8_t* rv = nullptr;
                /*
                 * The remaining part of the block is too small for anything, give the full block.
                 */
                if (mbd->bytes + size_of_mbd <= adjustment)
                {
                    block->bytes += mbd->bytes + size_of_mbd;
                    LinkedListUnlink(mbd);
                    rv = block->address;
                }
                /*
                 * Adjust MemoryBlockDescription and update it.
                 */
                else
                {
                    uint8_t* addr = mbd->address - size_of_mbd + adjustment;
                    MemoryBlockDescription* new_mbd = reinterpret_cast<MemoryBlockDescription*>(addr);
                    LinkedListUnlink(mbd);
                    memmove(new_mbd, mbd, sizeof(MemoryBlockDescription));

                    new_mbd->address = addr + adjustment;
                    new_mbd->bytes -= adjustment;

                    LinkedListInsertSorted(block, new_mbd);
                    rv = block->address;
                }
            
                if (rv != nullptr)
                {
                    mbd->bytes = realloc_bytes;
                    if (flags & MemoryManager::ALLOC_ZEROINIT)
                    {
                        memset(rv + adjustment, 0, adjustment);
                    }
                    return rv;
                }
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
        uint8_t* allocation = AllocateUnprotected(bytes, flags);
        if (allocation == nullptr)
        {
            return nullptr;
        }
        memcpy(allocation, address, block->bytes);
        DeallocateUnprotected(address);
        return allocation;
    }

    void DeallocateUnprotected(uint8_t* address)
    {
        //ENTER(address);
        if (address == nullptr)
        {
            return;
        }

        MemoryBlockDescription* block = FindBlock(address, 0, 0, MEMORY_BLOCK_USED);
        if (block == nullptr)
        {
            debuglog(LCF_MEMORY, "WARNING: Attempted removal of unknown memory!");
            return;
        }
        block->flags = MEMORY_BLOCK_FREE;

        /*
         * Attempt block merging.
         */
        if (block->next != nullptr && block->next->flags == MEMORY_BLOCK_FREE)
        {
            block->bytes += block->next->bytes + size_of_mbd;
            LinkedListUnlink(block->next);
        }
        if (block->prev != nullptr && block->prev->flags == MEMORY_BLOCK_FREE)
        {
            block = static_cast<MemoryBlockDescription*>(block->prev);
            block->bytes += block->next->bytes + size_of_mbd;
            LinkedListUnlink(block->next);
        }

        /*
         * Entire memory block deallocated, use some address math to find the
         * MemoryObjectDescription.
         */
        if (block->prev == nullptr && block->next == nullptr && block->flags == MEMORY_BLOCK_FREE)
        {
            MemoryObjectDescription* mod = static_cast<MemoryObjectDescription*>(block->top);
            LinkedListUnlink(mod);

            munmap(mod->address, mod->bytes);
        }
        //LEAVE();
    }
};

void MemoryManager::Init()
{
    //ENTER();
    //DLL_ASSERT(MemoryManagerInternal::memory_manager_inited == false);

    //SYSTEM_INFO si;
    //GetSystemInfo(&si);

    /*
     * TODO: Decide end address using the game's /LARGEADDRESS flag.
     * -- Warepire
     */
    //const uintptr_t LARGE_ADDRESS_SPACE_START = reinterpret_cast<uintptr_t>(0x80000000);
    //const uintptr_t LARGE_ADDRESS_SPACE_END = reinterpret_cast<uintptr_t>(0xC0000000);
    //MemoryManagerInternal::minimum_allowed_address = si.lpMinimumApplicationAddress;
    //MemoryManagerInternal::maximum_allowed_address = const_cast<uintptr_t>(LARGE_ADDRESS_SPACE_START);
    /*
     * TODO: Will this need to be calculated using dwPageSize?
     * -- Warepire
     */
    MemoryManagerInternal::allocation_granularity = getpagesize();
    MemoryManagerInternal::allocation_lock.clear();
    MemoryManagerInternal::memory_manager_inited = true;
}

void* MemoryManager::Allocate(int bytes, int flags)
{
    //DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    uint8_t* rv = MemoryManagerInternal::AllocateUnprotected(bytes, flags);
    MemoryManagerInternal::allocation_lock.clear();
    return static_cast<void*>(rv);
}

void* MemoryManager::Reallocate(void* address, int bytes, int flags)
{
    //DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    uint8_t* rv = MemoryManagerInternal::ReallocateUnprotected(static_cast<uint8_t*>(address), bytes, flags);
    MemoryManagerInternal::allocation_lock.clear();
    return static_cast<void*>(rv);
}

void MemoryManager::Deallocate(void* address)
{
    //DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    MemoryManagerInternal::DeallocateUnprotected(static_cast<uint8_t*>(address));
    MemoryManagerInternal::allocation_lock.clear();
}

size_t MemoryManager::GetSizeOfAllocation(const void* address)
{
    //DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    //ENTER(address);
    if (address == nullptr)
    {
        return 0;
    }

    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    size_t rv = 0;
    MemoryManagerInternal::MemoryBlockDescription* mbd =
        MemoryManagerInternal::FindBlock(static_cast<const uint8_t*>(address), 0, 0, MemoryManagerInternal::MEMORY_BLOCK_USED);
    
    if (mbd != nullptr)
    {
        rv = mbd->bytes;
    }

    MemoryManagerInternal::allocation_lock.clear();
    return rv;
}

void MemoryManagerInternal::DumpAllocationTable()
{
    MemoryObjectDescription* mod = memory_objects;
    MemoryBlockDescription* mbd = nullptr;
    while (mod != nullptr)
    {
        mbd = mod->blocks;
        debuglog(LCF_MEMORY, std::hex, "MOD this=", mod, " addr=", mod->address, " bytes=", mod->bytes, " flags=", mod->flags, " blocks=", mod->blocks, std::dec);
        while (mbd != nullptr)
        {
            debuglog(LCF_MEMORY, std::hex, "MBD this=", mbd, " addr=", mbd->address, " bytes=", mbd->bytes, " flags=", mbd->flags, std::dec);
            mbd = static_cast<MemoryBlockDescription*>(mbd->next);
        }
        mod = static_cast<MemoryObjectDescription*>(mod->next);
    }
}

