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

#include "malloc.h"
#include "../logging.h"
#include "MemoryManager.h"
#include "../dlhook.h"
#include "../ThreadState.h"
#include "../backtrace.h"

bool custom_mm = true; // TODO: Make this an option

namespace orig {
    static void *(*malloc) (size_t size) throw();
    static void *(*calloc) (size_t nmemb, size_t size) throw();
    static void *(*realloc) (void *ptr, size_t size) throw();
    static void *(*valloc) (size_t size) throw();
    static void *(*pvalloc) (size_t size) throw();
    static int (*posix_memalign) (void **memptr, size_t alignment, size_t size) throw();
    static void *(*aligned_alloc) (size_t alignment, size_t size) throw();
    static void *(*memalign) (size_t alignment, size_t size) throw();
    static void (*free) (void *ptr) throw();
}

void *malloc (size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);
    //printBacktrace();
    void* addr;
    if (custom_mm && !threadState.isNative())
        addr = memorymanager.allocate(size, MemoryManager::ALLOC_WRITE, 0);
    else {
        LINK_NAMESPACE(malloc, nullptr);
        addr = orig::malloc(size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

static void *null_calloc (size_t nmemb, size_t size) throw()
{
    return NULL;
}

void *calloc (size_t nmemb, size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);
    void* addr;
    if (custom_mm && !threadState.isNative())
        addr = memorymanager.allocate(nmemb * size, MemoryManager::ALLOC_WRITE | MemoryManager::ALLOC_ZEROINIT, 0);
    else {
        /*
         * We must add a hack here, because to access to the original
         * calloc function, we must call dlsym which calls calloc internally
         * (actually, dlerror() is calling calloc).
         * However, looking at the dlerror code, it seems we can safely return
         * NULL for that specific call.
         *
         * So we set calloc with a NULL returning function before calling dlsym.
         */
        if (!orig::calloc) {
            /* Next time calloc is called, it will return NULL */
            orig::calloc = null_calloc;

            /* Code from link_function() */
            dlhook_init();
            dlenter();
            /* calloc is called below, and returns NULL */
            orig::calloc = reinterpret_cast<void*(*)(size_t, size_t)>(dlsym(RTLD_NEXT, "calloc"));
            dlleave();

        }
        addr = orig::calloc(nmemb, size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

void *realloc (void *ptr, size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with ptr %p and size %d", __func__, ptr, size);
    void* addr;
    if (custom_mm)
        addr = memorymanager.reallocate(ptr, size, MemoryManager::ALLOC_WRITE);
    if (!custom_mm || !addr) {
        LINK_NAMESPACE(realloc, nullptr);
        addr = orig::realloc(ptr, size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

void *valloc (size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);
    void* addr;
    if (custom_mm && !threadState.isNative()) {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Alignment not supported!");
        addr = memorymanager.allocate(size, MemoryManager::ALLOC_WRITE, 0);
    }
    else {
        LINK_NAMESPACE(valloc, nullptr);
        addr = orig::valloc(size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

void *pvalloc (size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);
    void* addr;
    if (custom_mm && !threadState.isNative()) {
        debuglogstdio(LCF_MEMORY | LCF_ERROR, "Alignment not supported!");
        addr = memorymanager.allocate(size, MemoryManager::ALLOC_WRITE, 0);
    }
    else {
        LINK_NAMESPACE(pvalloc, nullptr);
        addr = orig::pvalloc(size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

int posix_memalign (void **memptr, size_t alignment, size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with alignment %d and size %d", __func__, alignment, size);
    void* addr;
    int ret = 0;
    if (custom_mm && !threadState.isNative()) {
        addr = memorymanager.allocate(size, MemoryManager::ALLOC_WRITE, alignment);
        if (!addr)
            ret = ENOMEM;
        *memptr = addr;
    }
    else {
        LINK_NAMESPACE(posix_memalign, nullptr);
        ret = orig::posix_memalign (memptr, alignment, size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", *memptr);
    return ret;
}

void *aligned_alloc (size_t alignment, size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with alignment %d and size %d", __func__, alignment, size);
    void* addr;
    if (custom_mm && !threadState.isNative()) {
        addr = memorymanager.allocate(size, MemoryManager::ALLOC_WRITE, alignment);
    }
    else {
        LINK_NAMESPACE(aligned_alloc, nullptr);
        addr = orig::aligned_alloc(alignment, size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

void *memalign (size_t alignment, size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with alignment %d and size %d", __func__, alignment, size);
    void* addr;
    if (custom_mm && !threadState.isNative()) {
        addr = memorymanager.allocate(size, MemoryManager::ALLOC_WRITE, alignment);
    }
    else {
        LINK_NAMESPACE(memalign, nullptr);
        addr = orig::memalign(alignment, size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

void free (void *ptr) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with ptr %p", __func__, ptr);
    //if (custom_mm && !threadState.isNative())
    bool res = true;
    if (custom_mm)
        res = memorymanager.deallocate(ptr);
    if (!custom_mm || !res) {
        LINK_NAMESPACE(free, nullptr);
        orig::free(ptr);
    }
    debuglogstdio(LCF_MEMORY, "  returns");
}

