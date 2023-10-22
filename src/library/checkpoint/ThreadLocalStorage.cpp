/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

    Most of the code taken from DMTCP <http://dmtcp.sourceforge.net/>
*/

#include "ThreadLocalStorage.h"
#include "../logging.h"
#include <unistd.h>
#include <sys/syscall.h> // SYS_get_thread_area, SYS_set_thread_area
#include <cstring> // memset

#if defined(__APPLE__) && defined(__MACH__)
#else
#ifdef __x86_64__
#include <asm/prctl.h> // ARCH_GET_FS, ARCH_GET_GS, etc.
#include <sys/prctl.h>
#endif
#endif

namespace libtas {

void ThreadLocalStorage::saveTLSState(ThreadTLSInfo *tlsInfo)
{
#if defined(__APPLE__) && defined(__MACH__)
#else
#ifdef __i386__
    asm volatile ("movw %%fs,%0" : "=m" (tlsInfo->fs));
    asm volatile ("movw %%gs,%0" : "=m" (tlsInfo->gs));
    memset(tlsInfo->gdtentrytls, 0, sizeof tlsInfo->gdtentrytls);

    int i = tlsInfo->gs / 8;
    tlsInfo->gdtentrytls[0].entry_number = i;
    MYASSERT(syscall(SYS_get_thread_area, &(tlsInfo->gdtentrytls[0])) == 0)
#elif __x86_64__
    MYASSERT(syscall(SYS_arch_prctl, ARCH_GET_FS, &tlsInfo->fs) == 0)
    MYASSERT(syscall(SYS_arch_prctl, ARCH_GET_GS, &tlsInfo->gs) == 0)
#endif
#endif
}

void ThreadLocalStorage::restoreTLSState(ThreadTLSInfo *tlsInfo)
{
  /* Every architecture needs a register to point to the current
   * TLS (thread-local storage).  This is where we set it up.
   */

#if defined(__APPLE__) && defined(__MACH__)
#else
#ifdef __i386__
   MYASSERT(syscall(SYS_set_thread_area, &(tlsInfo->gdtentrytls[0])) == 0)

   /* Finally, if this is i386, we need to set %gs to refer to the segment
    * descriptor that we're using above.  We restore the original pointer.
    * For the other architectures (not i386), the kernel call above
    * already did the equivalent work of setting up thread registers.
    */

   asm volatile ("movw %0,%%fs" : : "m" (tlsInfo->fs));
   asm volatile ("movw %0,%%gs" : : "m" (tlsInfo->gs));
#elif __x86_64__
    MYASSERT(syscall(SYS_arch_prctl, ARCH_SET_FS, tlsInfo->fs) == 0)
    MYASSERT(syscall(SYS_arch_prctl, ARCH_SET_GS, tlsInfo->gs) == 0)
#endif
#endif
}

}
