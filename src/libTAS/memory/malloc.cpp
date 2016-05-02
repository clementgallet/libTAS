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

bool custom_mm = false; // TODO: Make this an option

namespace orig {
    static void *(*malloc) (size_t size) throw();
    static void *(*calloc) (size_t nmemb, size_t size) throw();
    static void *(*realloc) (void *ptr, size_t size) throw();
    static void (*free) (void *ptr) throw();
}

void *malloc (size_t size) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with size %d", __func__, size);
    void* addr;
    if (custom_mm)
        addr = memorymanager.allocate(size, MemoryManager::ALLOC_WRITE);
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
    if (custom_mm)
        addr = memorymanager.allocate(nmemb * size, MemoryManager::ALLOC_WRITE | MemoryManager::ALLOC_ZEROINIT);
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
    else {
        LINK_NAMESPACE(realloc, nullptr);
        addr = orig::realloc(ptr, size);
    }
    debuglogstdio(LCF_MEMORY, "  returns addr %p", addr);
    return addr;
}

void free (void *ptr) throw()
{
    debuglogstdio(LCF_MEMORY, "%s call with ptr %p", __func__, ptr);
    if (custom_mm)
        return memorymanager.deallocate(ptr);
    else {
        LINK_NAMESPACE(free, nullptr);
        return orig::free(ptr);
    }
}

