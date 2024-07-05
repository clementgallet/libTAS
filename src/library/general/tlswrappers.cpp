/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "GlobalState.h"

#include <map>

namespace libtas {

DEFINE_ORIG_POINTER(pthread_key_create)
DEFINE_ORIG_POINTER(pthread_key_delete)
DEFINE_ORIG_POINTER(pthread_getspecific)
DEFINE_ORIG_POINTER(pthread_setspecific)

/* We declare our map static inside a function, so that we ensure it is
 * constructed when we call the function. */
static std::map<pthread_key_t, void(*)(void*)>& getPthreadKeys() {
    static std::map<pthread_key_t, void(*)(void*)> pthread_keys;
    return pthread_keys;
}

void clear_pthread_keys()
{
    LINK_NAMESPACE(pthread_getspecific, "pthread");
    LINK_NAMESPACE(pthread_setspecific, "pthread");

    std::map<pthread_key_t, void(*)(void*)> pthread_keys = getPthreadKeys();
    for( const auto& pair : pthread_keys ) {
        if (orig::pthread_getspecific(pair.first)) {
            LOG(LL_DEBUG, LCF_THREAD, "  removing value from key %d", pair.first);
            orig::pthread_setspecific(pair.first, nullptr);
            if (orig::pthread_getspecific(pair.first)) {
                LOG(LL_DEBUG, LCF_THREAD, "  calling destructor for key %d", pair.first);
            }
        }
    }
}

int pthread_key_create (pthread_key_t *key, void (*destr_function) (void *)) __THROW
{
    LINK_NAMESPACE(pthread_key_create, "pthread");
    if (GlobalState::isNative()) {
        return orig::pthread_key_create(key, destr_function);
    }

    LOGTRACE(LCF_THREAD);
    int ret = orig::pthread_key_create(key, destr_function);

    LOG(LL_DEBUG, LCF_THREAD, "   returning %d", *key);

    std::map<pthread_key_t, void(*)(void*)> pthread_keys = getPthreadKeys();
    pthread_keys.insert(std::pair<pthread_key_t, void(*)(void*)>(*key,destr_function));

    return ret;
}

int pthread_key_delete (pthread_key_t key) __THROW
{
    LINK_NAMESPACE(pthread_key_delete, "pthread");
    if (GlobalState::isNative()) {
        return orig::pthread_key_delete(key);
    }

    LOG(LL_TRACE, LCF_THREAD, "%s called on key %d", __func__, key);
    int ret = orig::pthread_key_delete(key);

    std::map<pthread_key_t, void(*)(void*)> pthread_keys = getPthreadKeys();
    auto it = pthread_keys.find(key);
    if (it != pthread_keys.end()) {
        pthread_keys.erase (it);
    }

    return ret;
}

}
