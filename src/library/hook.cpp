/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "hook.h"
#include "dlhook.h"
#include "logging.h"
#include "GlobalState.h"
#include <string>
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/task.h>
#include <mach/mach.h>
#include <mach-o/dyld_images.h>
#endif

namespace libtas {

bool link_function(void** function, const char* source, const char* library, const char *version /*= nullptr*/)
{
    /* Test if function is already linked */
    if (*function != nullptr)
        return true;

    /* First try to link it from the global namespace */
#ifdef __linux__
    if (version)
        *function = dlvsym(RTLD_NEXT, source, version);
    if (*function == nullptr)
#endif
        NATIVECALL(*function = dlsym(RTLD_NEXT, source));

    if (*function != nullptr) {
        debuglogstdio(LCF_HOOK, "Imported symbol %s function : %p", source, *function);
        return true;
    }

    if (library != nullptr) {

        /* If it did not succeed, try to link using a matching library
         * loaded by the game.
         */
        std::string libpath = find_lib(library);

        void* handle;
        if (! libpath.empty()) {

            /* Try to link again using a matching library */
            NATIVECALL(handle = dlopen(libpath.c_str(), RTLD_LAZY));

            if (handle != NULL) {
                NATIVECALL(*function = dlsym(handle, source));

                if (*function != nullptr) {
                    debuglogstdio(LCF_HOOK, "Imported from lib %s symbol %s function : %p", libpath.c_str(), source, *function);
                    return true;
                }
            }
        }

        /* If it did not succeed, try to link using the given library */
        NATIVECALL(handle = dlopen(library, RTLD_LAZY));

        if (handle != NULL) {
            NATIVECALL(*function = dlsym(handle, source));

            if (*function != nullptr) {
                debuglogstdio(LCF_HOOK, "Imported from lib %s symbol %s function : %p", library, source, *function);

                /* Add the library to our set of libraries */
                add_lib(library);
                return true;
            }
        }
    }
    
#if defined(__APPLE__) && defined(__MACH__)
    /* Look at all loaded libraries and find a matched library */
    /* From: https://blog.lse.epita.fr/2017/03/14/playing-with-mach-os-and-dyld.html */
    if (library != nullptr) {
        /* Get DYLD task infos */
        struct task_dyld_info dyld_info;
        mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
        kern_return_t ret;
        ret = task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&dyld_info, &count);
        if (ret != KERN_SUCCESS) {
            debuglogstdio(LCF_HOOK | LCF_ERROR, "Could not get task info");
            return false;
        }
        
        /* Get image array's size and address */
        mach_vm_address_t image_infos = dyld_info.all_image_info_addr;
        struct dyld_all_image_infos *infos;
        infos = reinterpret_cast<struct dyld_all_image_infos*>(image_infos);
        uint32_t image_count = infos->infoArrayCount;
        const struct dyld_image_info *image_array = infos->infoArray;
        
        /* Find the library among loaded libraries */
        for (int i = 0; i < image_count; ++i) {
            struct dyld_image_info image = image_array[i];
            
            if (strstr(image.imageFilePath, library)) {
                /* We found a matching library. Load the library and look at symbol */
                void* handle;
                NATIVECALL(handle = dlopen(image.imageFilePath, RTLD_NOLOAD | RTLD_LAZY));
                    
                if (handle != NULL) {
                    NATIVECALL(*function = dlsym(handle, source));
                    dlclose(handle);
                    if (*function != nullptr) {
                        debuglogstdio(LCF_HOOK, "Imported from mach lib %s symbol %s function: %p", image.imageFilePath, source, *function);
                        return true;
                    }
                }
            }
        }
    }
#endif

    debuglogstdio(LCF_ERROR | LCF_HOOK, "Could not import symbol %s", source);

    *function = nullptr;
    return false;
}

}
