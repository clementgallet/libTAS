/* Usage:
 *     ./symbol_signature symbol lib_path
 * This program looks at every library file inside `lib_path`, finds the function
 * identified as `symbol`, looks at the assembly and tracks every byte that is
 * shared among every library. It prints the result with the common bytes, and
 * `??` if the byte differed between the different libraries. It can help
 * identifying functions by their signature, when we don't have access to the
 * function symbol.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <ctype.h>

#define SIGNATURE_LEN 300

unsigned char signature_bytes[SIGNATURE_LEN];
unsigned char signature_mask[SIGNATURE_LEN];

int main(int argc, char * argv[])
{
    if (argc < 2)
        return 0;

    memset(signature_bytes, 0, SIGNATURE_LEN);
    memset(signature_mask, 1, SIGNATURE_LEN);

    char* symbol = argv[1];
    char* path = argv[2];
    
    char* abspath = realpath(path, NULL);
    if (!abspath) {
        printf("Could not resolve path of libraries %s\n", path);
        return 1;
    }

    struct dirent *dp;

    DIR *dir = opendir(abspath);
    if (!dir) {
        printf("Could not open directory %s\n", abspath);
        return 1;        
    }
    
    int first_library = 1;
    
    while ((dp = readdir(dir))) {
        if (dp->d_type != DT_REG)
            continue;

        printf("Process file %s\n", dp->d_name);

        char filepath[1024] = {};
        sprintf(filepath, "%s/%s", abspath, dp->d_name);
        
        /* Find if symbol is present in library file */
        char cmd[1024] = {};
        sprintf(cmd, "readelf -Ws \"%s\" | grep %s | awk '{print $2}'", filepath, symbol);

        char outputstr[256];
        FILE *output = popen(cmd, "r");
        if (output != NULL) {
            char* c = fgets(outputstr, 256, output);
            pclose(output);
            if (!c)
                continue;
        }

        unsigned long long addr_offset = strtoull(outputstr, NULL, 16);    
        printf("Found address offset %llx for symbol\n", addr_offset);

        /* dlopen */
        void* h = dlopen(filepath, RTLD_LAZY);
        if (!h) {
            continue;
        }

        unsigned long long base_addr = (unsigned long long)*(size_t const*)(h);
        printf("Library loaded in %llx\n", base_addr);
        
        unsigned long long symbol_addr = base_addr + addr_offset;
        unsigned char* symbol_addr_ptr = (char*) symbol_addr;

        /* Compare bytes from saved signature */
        if (first_library) {
            memcpy(signature_bytes, symbol_addr_ptr, SIGNATURE_LEN);
            first_library = 0;
        }
        else {
            for (int i = 0; i < SIGNATURE_LEN; i++) {
                if (signature_mask[i] && (signature_bytes[i] != symbol_addr_ptr[i]))
                    signature_mask[i] = 0;
            }
        }
        
        dlclose(h);
    }
    
    /* Print signature */
    printf("Signature is:\n");

    for (int i = 0; i < SIGNATURE_LEN; i++) {
        if (signature_mask[i])
            printf("%02x ", signature_bytes[i]);
        else
            printf("?? ");
    }
    printf("\n");
    
    closedir(dir);
    free(abspath);
    return 0;
}
