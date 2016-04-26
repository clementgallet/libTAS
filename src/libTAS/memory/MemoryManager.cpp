/*
 * (c) 2015- Hourglass Resurrection Team
 * Hourglass Resurrection is licensed under GPL v2.
 * Refer to the file COPYING.txt in the project root.
 */

#include <Windows.h>

#include <atomic>

#include "MemoryManager.h"
#include <print.h>
#include <Utils.h>

namespace
{
    UINT MakeBytesAligned(UINT bytes, UINT alignment)
    {
        return bytes + ((alignment - (bytes % alignment)) % alignment);
    }
}

namespace MemoryManagerInternal
{
    enum MemoryBlockState : UINT
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
        AddressLinkedList* m_prev;
        AddressLinkedList* m_next;
        AddressLinkedList** m_head;
        LPVOID m_address;
        UINT m_bytes;
        UINT m_flags;
    };

    /*
     * Helper functions
     */

    /*
     * The list-parameter can be any element in the list.
     */
    void LinkedListInsertSorted(AddressLinkedList* list, AddressLinkedList* item)
    {
        ENTER(list, item);
        DLL_ASSERT(list != nullptr && item != nullptr && list->m_address != item->m_address);

        AddressLinkedList* it = list;
        while (true)
        {
            if (item->m_address < it->m_address)
            {
                if (it->m_prev != nullptr && it->m_prev->m_address < item->m_address)
                {
                    it = it->m_prev;
                }
                else
                {
                    item->m_prev = it->m_prev;
                    item->m_next = it;
                    it->m_prev = item;

                    if (item->m_prev == nullptr)
                    {
                        *(item->m_head) = item;
                    }
                    break;
                }
            }
            else
            {
                if (it->m_next != nullptr && item->m_address > it->m_next->m_address)
                {
                    it = it->m_next;
                }
                else
                {
                    item->m_prev = it;
                    item->m_next = it->m_next;
                    it->m_next = item;
                    break;
                }
            }
        }
    }

    void LinkedListUnlink(AddressLinkedList* item)
    {
        DLL_ASSERT(item != nullptr);
        if (item->m_prev == nullptr)
        {
            *(item->m_head) = item->m_next;
        }
        else
        {
            item->m_prev->m_next = item->m_next;
        }
        if (item->m_next != nullptr)
        {
            item->m_next->m_prev = item->m_prev;
        }
    }

    struct MemoryBlockDescription :
        public AddressLinkedList
    {
    };

    struct MemoryObjectDescription :
        public AddressLinkedList
    {
        HANDLE m_object;
        MemoryBlockDescription* m_blocks;
    };

    bool memory_manager_inited = false;

    LPVOID minimum_allowed_address = nullptr;
    LPVOID maximum_allowed_address = nullptr;
    DWORD allocation_granularity = 0;
    UINT size_of_mbd = MakeBytesAligned(sizeof(MemoryBlockDescription), 8);
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
    LPVOID AllocateUnprotected(UINT bytes, UINT flags);
    LPVOID AllocateInExistingBlock(UINT bytes, UINT flags);
    LPVOID AllocateWithNewBlock(UINT bytes, UINT flags);

    LPVOID ReallocateUnprotected(LPVOID address, UINT bytes, UINT flags);

    void DeallocateUnprotected(LPVOID address);

    LPVOID FindAllocationBaseAddress(UINT bytes, UINT flags)
    {
        MEMORY_BASIC_INFORMATION best_gap;
        memset(&best_gap, 0, sizeof(best_gap));
        MEMORY_BASIC_INFORMATION this_gap;
        LPVOID current_address = const_cast<LPVOID>(minimum_allowed_address);

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

    /*
     * A 0 / nullptr value or a parameter means "any"
     * Returns nullptr if block was found.
     */
    MemoryBlockDescription* FindBlock(LPCVOID address,
                                      UINT bytes,
                                      UINT object_flags,
                                      UINT block_flags)
    {
        ENTER(address, bytes, object_flags, block_flags);
        /*
         * Make sure only relevant flags are set.
         */
        object_flags &= MemoryManager::ALLOC_EXECUTE |
                        MemoryManager::ALLOC_READONLY |
                        MemoryManager::ALLOC_WRITE;

        MemoryBlockDescription* rv = nullptr;

        if ((address != nullptr && address >= minimum_allowed_address || address < maximum_allowed_address) ||
            address == nullptr)
        {
            for (MemoryObjectDescription* mod = memory_objects;
                 mod != nullptr;
                 mod = static_cast<MemoryObjectDescription*>(mod->m_next))
            {
                LPVOID mod_end_address = static_cast<LPBYTE>(mod->m_address) + mod->m_bytes;
                if (((address != nullptr && address >= mod->m_address && address < mod_end_address) || address == nullptr) &&
                    ((object_flags != 0 && object_flags == mod->m_flags) || object_flags == 0))
                {
                    for (MemoryBlockDescription* mbd = mod->m_blocks;
                         mbd != nullptr;
                         mbd = static_cast<MemoryBlockDescription*>(mbd->m_next))
                    {
                        if (((address != nullptr && address == mbd->m_address) || address == nullptr) &&
                            ((bytes != 0 && bytes <= mbd->m_bytes) || bytes == 0) &&
                            ((block_flags != 0 && block_flags == mbd->m_flags) || block_flags == 0))
                        {
                            if (rv == nullptr || rv->m_bytes > mbd->m_bytes)
                            {
                                rv = mbd;
                            }
                            if (bytes == rv->m_bytes)
                            {
                                goto memory_block_search_done;
                            }
                        }
                    }
                }
            }
        }
    memory_block_search_done:
        LEAVE(rv);
        return rv;
    }

    LPVOID AllocateInExistingBlock(UINT bytes, UINT flags)
    {
        ENTER(bytes, flags);
        bytes = MakeBytesAligned(bytes, 8);
        MemoryBlockDescription* best_block = FindBlock(nullptr, bytes, flags, MEMORY_BLOCK_FREE);

        if (best_block == nullptr)
        {
            return nullptr;
        }

        best_block->m_flags = MEMORY_BLOCK_USED;
        if (best_block->m_bytes > bytes + size_of_mbd)
        {
            LPBYTE free_space = static_cast<LPBYTE>(best_block->m_address) + bytes;

            MemoryBlockDescription* mbd = reinterpret_cast<MemoryBlockDescription*>(free_space);
            mbd->m_address = free_space + size_of_mbd;
            mbd->m_bytes = best_block->m_bytes - (bytes + size_of_mbd);
            mbd->m_flags = MEMORY_BLOCK_FREE;
            mbd->m_head = best_block->m_head;

            best_block->m_bytes = bytes;

            LinkedListInsertSorted(best_block, mbd);
        }

        if ((flags & MemoryManager::ALLOC_ZEROINIT) == MemoryManager::ALLOC_ZEROINIT)
        {
            memset(best_block->m_address, 0, best_block->m_bytes);
        }

        LEAVE(best_block->m_address);
        return best_block->m_address;
    }

    LPVOID AllocateWithNewBlock(UINT bytes, UINT flags)
    {
        /*
         * Calculate the size of the mapped file and make sure the allocation is a multible of
         * 8 bytes.
         */
        ENTER(bytes, flags);
        SIZE_T file_size = allocation_granularity;
        UINT bytes_for_mod_and_mbd = sizeof(MemoryObjectDescription) + sizeof(MemoryBlockDescription);
        bytes_for_mod_and_mbd = MakeBytesAligned(bytes_for_mod_and_mbd, 8);
        bytes = MakeBytesAligned(bytes, 8);
        while (bytes + bytes_for_mod_and_mbd > file_size)
        {
            file_size += allocation_granularity;
        }

        LPVOID target_address = FindAllocationBaseAddress(file_size, flags);
        if (target_address == nullptr)
        {
            return nullptr;
        }

        DWORD access = 0;
        if ((flags & MemoryManager::ALLOC_WRITE) == MemoryManager::ALLOC_WRITE)
        {
            access = FILE_MAP_WRITE;
        }
        else if ((flags & MemoryManager::ALLOC_READONLY) == MemoryManager::ALLOC_READONLY)
        {
            access = FILE_MAP_READ;
        }
        else
        {
            DLL_ASSERT(false);
        }
        if ((flags & MemoryManager::ALLOC_EXECUTE) == MemoryManager::ALLOC_EXECUTE)
        {
            access |= FILE_MAP_EXECUTE;
        }

        /*
         * TODO: Decide a naming scheme, to easier ID segments in a save state.
         * -- Warepire
         */
        /*
         * Ensure GetLastError() returns an error-code from CreateFileMapping.
         */
        SetLastError(ERROR_SUCCESS);
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
        LPVOID addr = MapViewOfFileEx(map_file, access, 0, 0, file_size, target_address);
        if (addr == nullptr)
        {
            CloseHandle(map_file);
            return nullptr;
        }

        if ((flags & MemoryManager::ALLOC_ZEROINIT) == MemoryManager::ALLOC_ZEROINIT)
        {
            memset(addr, 0, bytes + bytes_for_mod_and_mbd);
        }

        MemoryObjectDescription* mod = static_cast<MemoryObjectDescription*>(addr);
        mod->m_address = addr;
        mod->m_object = map_file;
        mod->m_bytes = file_size;
        mod->m_flags = flags & (MemoryManager::ALLOC_EXECUTE |
                                MemoryManager::ALLOC_READONLY |
                                MemoryManager::ALLOC_WRITE);
        mod->m_head = reinterpret_cast<AddressLinkedList**>(&memory_objects);

        if (memory_objects == nullptr)
        {
            memory_objects = mod;
        }
        else
        {
            LinkedListInsertSorted(memory_objects, mod);
        }

        addr = static_cast<LPBYTE>(mod->m_address) + sizeof(MemoryObjectDescription);

        MemoryBlockDescription* mbd = static_cast<MemoryBlockDescription*>(addr);
        mbd->m_address = static_cast<LPBYTE>(mod->m_address) + bytes_for_mod_and_mbd;
        mbd->m_bytes = bytes;
        mbd->m_flags = MEMORY_BLOCK_USED;
        mbd->m_head = reinterpret_cast<AddressLinkedList**>(&mod->m_blocks);
        /*
         * mod->blocks is always an invalid pointer here.
         * There is also no need to insert this one.
         */
        mod->m_blocks = mbd;

        if (bytes + bytes_for_mod_and_mbd + size_of_mbd < allocation_granularity)
        {
            /*
             * This block can be used for more than one allocation.
             */
            addr = static_cast<LPBYTE>(mod->m_address) + bytes + bytes_for_mod_and_mbd;
            mbd = reinterpret_cast<MemoryBlockDescription*>(addr);
            UINT free_space = file_size - (bytes + bytes_for_mod_and_mbd + size_of_mbd);

            mbd->m_address = static_cast<LPBYTE>(addr) + size_of_mbd;
            mbd->m_bytes = free_space;
            mbd->m_flags = MEMORY_BLOCK_FREE;
            mbd->m_head = reinterpret_cast<AddressLinkedList**>(&mod->m_blocks);

            LinkedListInsertSorted(mod->m_blocks, mbd);
        }

        LEAVE(mod->m_blocks->m_address);
        return mod->m_blocks->m_address;
    }

    LPVOID AllocateUnprotected(UINT bytes, UINT flags)
    {
        ENTER(bytes, flags);
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
            LPVOID allocation = AllocateInExistingBlock(bytes * 2, flags);
            if (allocation != nullptr)
            {
                return allocation;
            }
        }
        return AllocateWithNewBlock(bytes, flags);
    }

    LPVOID ReallocateUnprotected(LPVOID address, UINT bytes, UINT flags)
    {
        ENTER(address, bytes, flags);

        if (address == nullptr)
        {
            return AllocateUnprotected(bytes, flags);
        }
        if (bytes == 0)
        {
            DeallocateUnprotected(address);
            return nullptr;
        }

        UINT realloc_bytes = MakeBytesAligned(bytes * 2, 8);

        MemoryBlockDescription* block = FindBlock(address, 0, flags, MEMORY_BLOCK_USED);

        /*
         * Attempt to adjust at the current location.
         */
        if (realloc_bytes < allocation_granularity && block != nullptr)
        {
            MemoryBlockDescription* mbd = block;
            INT adjustment = (realloc_bytes - block->m_bytes);

            /*
             * No difference.
             */
            if (adjustment == 0)
            {
                return mbd->m_address;
            }
            /*
             * Reallocating to smaller buffer and it would actually free a useful memory block.
             */
            if (static_cast<INT>(adjustment + size_of_mbd) < 0)
            {
                LPVOID addr = static_cast<LPBYTE>(mbd->m_address) + realloc_bytes;
                MemoryBlockDescription *new_mbd = reinterpret_cast<MemoryBlockDescription*>(addr);
                new_mbd->m_address = static_cast<LPBYTE>(addr) + size_of_mbd;
                new_mbd->m_bytes = abs(adjustment);
                new_mbd->m_flags = MEMORY_BLOCK_FREE;

                mbd->m_bytes = bytes;

                LinkedListInsertSorted(mbd, new_mbd);

                return mbd->m_address;
            }
            /*
             * The memory block that would be free'd is smaller than anything that can fit there.
             * Do nothing.
             */
            if (adjustment < 0)
            {
                return mbd->m_address;
            }

            /*
             * Expand block.
             */
            mbd = static_cast<MemoryBlockDescription*>(mbd->m_next);
            if (mbd != nullptr &&
                static_cast<INT64>(mbd->m_bytes) > static_cast<INT64>(adjustment) &&
                mbd->m_flags == MEMORY_BLOCK_FREE)
            {
                LPVOID rv = nullptr;
                /*
                 * The remaining part of the block is too small for anything, give the full block.
                 */
                if (mbd->m_bytes + size_of_mbd <= static_cast<UINT>(adjustment))
                {
                    block->m_bytes += mbd->m_bytes + size_of_mbd;
                    LinkedListUnlink(mbd);
                    rv = block->m_address;
                }
                /*
                 * Adjust MemoryBlockDescription and update it.
                 */
                else
                {
                    LPVOID addr = (static_cast<LPBYTE>(mbd->m_address) - size_of_mbd) + adjustment;
                    MemoryBlockDescription* new_mbd = reinterpret_cast<MemoryBlockDescription*>(addr);
                    LinkedListUnlink(mbd);
                    memmove(new_mbd, mbd, sizeof(MemoryBlockDescription));

                    new_mbd->m_address = static_cast<LPBYTE>(addr) + adjustment;
                    new_mbd->m_bytes -= adjustment;

                    LinkedListInsertSorted(block, new_mbd);
                    rv = block->m_address;
                }
            
                if (rv != nullptr)
                {
                    mbd->m_bytes = realloc_bytes;
                    if ((flags & MemoryManager::ALLOC_ZEROINIT) == MemoryManager::ALLOC_ZEROINIT)
                    {
                        memset(static_cast<LPBYTE>(rv) + adjustment, 0, adjustment);
                    }
                    return rv;
                }
            }
        }
        /*
         * Changing the base pointer is not allowed, return nullptr.
         */
        if ((flags & MemoryManager::REALLOC_NO_MOVE) == MemoryManager::REALLOC_NO_MOVE)
        {
            return nullptr;
        }
        /*
         * Adjustment is not possible, allocate somewhere else.
         */
        LPVOID allocation = AllocateUnprotected(bytes, flags);
        if (allocation == nullptr)
        {
            return nullptr;
        }
        memcpy(allocation, address, block->m_bytes);
        DeallocateUnprotected(address);
        return allocation;
    }

    void DeallocateUnprotected(LPVOID address)
    {
        ENTER(address);
        if (address == nullptr)
        {
            return;
        }

        MemoryBlockDescription* block = FindBlock(address, 0, 0, MEMORY_BLOCK_USED);
        if (block == nullptr)
        {
            debugprintf("%s WARNING: Attempted removal of unknown memory!.\n", __FUNCTION__);
            return;
        }
        block->m_flags = MEMORY_BLOCK_FREE;

        /*
         * Attempt block merging.
         */
        if (block->m_next != nullptr && block->m_next->m_flags == MEMORY_BLOCK_FREE)
        {
            block->m_bytes += block->m_next->m_bytes + size_of_mbd;
            LinkedListUnlink(block->m_next);
        }
        if (block->m_prev != nullptr && block->m_prev->m_flags == MEMORY_BLOCK_FREE)
        {
            block->m_prev->m_bytes += block->m_bytes + size_of_mbd;
            LinkedListUnlink(block);
        }

        /*
         * Entire memory block deallocated, use some address math to find the
         * MemoryObjectDescription.
         */
        if (*(block->m_head) != nullptr &&
            reinterpret_cast<MemoryBlockDescription*>(*block->m_head)->m_next == nullptr &&
            reinterpret_cast<MemoryBlockDescription*>(*block->m_head)->m_flags == MEMORY_BLOCK_FREE)
        {
            LPVOID addr = reinterpret_cast<MemoryBlockDescription*>(*block->m_head)->m_address;
            addr = reinterpret_cast<LPVOID>(reinterpret_cast<UINT>(addr) & 0xFFFF0000);

            MemoryObjectDescription* mod = static_cast<MemoryObjectDescription*>(addr);
            LinkedListUnlink(mod);

            UnmapViewOfFile(mod->m_address);
            CloseHandle(mod->m_object);
        }
        LEAVE();
    }
};

void MemoryManager::Init()
{
    ENTER();
    DLL_ASSERT(MemoryManagerInternal::memory_manager_inited == false);

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    /*
     * TODO: Decide end address using the game's /LARGEADDRESS flag.
     * -- Warepire
     */
    LPCVOID LARGE_ADDRESS_SPACE_START = reinterpret_cast<LPVOID>(0x80000000);
    LPCVOID LARGE_ADDRESS_SPACE_END = reinterpret_cast<LPVOID>(0xC0000000);
    MemoryManagerInternal::minimum_allowed_address = si.lpMinimumApplicationAddress;
    MemoryManagerInternal::maximum_allowed_address = const_cast<LPVOID>(LARGE_ADDRESS_SPACE_START);
    /*
     * TODO: Will this need to be calculated using dwPageSize?
     * -- Warepire
     */
    MemoryManagerInternal::allocation_granularity = si.dwAllocationGranularity;
    MemoryManagerInternal::allocation_lock.clear();
    MemoryManagerInternal::memory_manager_inited = true;
}

LPVOID MemoryManager::Allocate(UINT bytes, UINT flags)
{
    DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    LPVOID rv = MemoryManagerInternal::AllocateUnprotected(bytes, flags);
    MemoryManagerInternal::allocation_lock.clear();
    return rv;
}

LPVOID MemoryManager::Reallocate(LPVOID address, UINT bytes, UINT flags)
{
    DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    LPVOID rv = MemoryManagerInternal::ReallocateUnprotected(address, bytes, flags);
    MemoryManagerInternal::allocation_lock.clear();
    return rv;
}

void MemoryManager::Deallocate(LPVOID address)
{
    DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    MemoryManagerInternal::DeallocateUnprotected(address);
    MemoryManagerInternal::allocation_lock.clear();
}

SIZE_T MemoryManager::GetSizeOfAllocation(LPCVOID address)
{
    DLL_ASSERT(MemoryManagerInternal::memory_manager_inited);
    ENTER(address);
    if (address == nullptr)
    {
        return 0;
    }

    while (MemoryManagerInternal::allocation_lock.test_and_set() == true) {}
    SIZE_T rv = 0;
    MemoryManagerInternal::MemoryBlockDescription* mbd =
        MemoryManagerInternal::FindBlock(address, 0, 0, MemoryManagerInternal::MEMORY_BLOCK_USED);
    
    if (mbd != nullptr)
    {
        rv = mbd->m_bytes;
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
        mbd = mod->m_blocks;
        debugprintf("MOD prev=0x%p this=0x%p next=0x%p *head=0x%p addr=0x%p bytes=0x%X flags=0x%X blocks=0x%p\n",
                    mod->m_prev, mod, mod->m_next, *mod->m_head, mod->m_address, mod->m_bytes, mod->m_flags, mod->m_blocks);
        while (mbd != nullptr)
        {
            debugprintf("MBD prev=0x%p this=0x%p next=0x%p *head=0x%p addr=0x%p bytes=0x%X flags=0x%X\n",
                        mbd->m_prev, mbd, mbd->m_next, *mbd->m_head, mbd->m_address, mbd->m_bytes, mbd->m_flags);
            mbd = static_cast<MemoryBlockDescription*>(mbd->m_next);
        }
        mod = static_cast<MemoryObjectDescription*>(mod->m_next);
    }
}
