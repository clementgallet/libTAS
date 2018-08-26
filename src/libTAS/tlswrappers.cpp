/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "tlswrappers.h"
#include "logging.h"
// #include <errno.h>
// #include <unistd.h>
// #include <cstring>
// #include <atomic>
#include <map>
// #include <exception>
// #include "checkpoint/ThreadInfo.h"
// #include "checkpoint/ThreadManager.h"
// #include "checkpoint/ThreadSync.h"
// #include "DeterministicTimer.h"
// #include "backtrace.h"

namespace libtas {

DEFINE_ORIG_POINTER(pthread_key_create);
DEFINE_ORIG_POINTER(pthread_key_delete);
DEFINE_ORIG_POINTER(pthread_getspecific);
DEFINE_ORIG_POINTER(pthread_setspecific);

static std::map<pthread_key_t, void(*)(void*)> *pthread_keys;

void clear_pthread_keys()
{
    LINK_NAMESPACE(pthread_getspecific, "pthread");
    LINK_NAMESPACE(pthread_setspecific, "pthread");

    for( const auto& pair : *pthread_keys ) {
        if (orig::pthread_getspecific(pair.first)) {
            debuglog(LCF_THREAD, "  removing value from key ", pair.first);
            orig::pthread_setspecific(pair.first, nullptr);
            if (orig::pthread_getspecific(pair.first)) {
                debuglog(LCF_THREAD, "  calling destructor for key ", pair.first);
            }
        }
    }
}



int pthread_key_create (pthread_key_t *key, void (*destr_function) (void *)) throw()
{
    DEBUGLOGCALL(LCF_THREAD);
    LINK_NAMESPACE(pthread_key_create, "pthread");

    int ret = orig::pthread_key_create(key, destr_function);

    debuglog(LCF_THREAD, "   returning ", *key);

    /* Using initialization on first use idiom because this object could be
     * used before static object have a chance to initialize.
     */
    if (!pthread_keys) {
        pthread_keys = new std::map<pthread_key_t, void(*)(void *)>;
    }

    pthread_keys->insert(std::pair<pthread_key_t, void(*)(void*)>(*key,destr_function));

    return ret;
}

int pthread_key_delete (pthread_key_t key) throw()
{
    debuglog(LCF_THREAD, __func__, " called on key ", key);
    LINK_NAMESPACE(pthread_key_create, "pthread");

    int ret = orig::pthread_key_delete(key);

    auto it = pthread_keys->find(key);
    if (it != pthread_keys->end()) {
        pthread_keys->erase (it);
    }

    return ret;
}

void *pthread_getspecific (pthread_key_t key) throw()
{
    debuglog(LCF_THREAD | LCF_FREQUENT, __func__, " called on key ", key);
    LINK_NAMESPACE(pthread_getspecific, "pthread");

    return orig::pthread_getspecific(key);
}

int pthread_setspecific (pthread_key_t key, const void *pointer) throw()
{
    debuglog(LCF_THREAD | LCF_FREQUENT, __func__, " called on key ", key, " and pointer ", pointer);
    LINK_NAMESPACE(pthread_setspecific, "pthread");

    return orig::pthread_setspecific(key, pointer);
}

}
