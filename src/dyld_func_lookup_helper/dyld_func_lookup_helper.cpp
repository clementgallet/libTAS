#include "dyld_func_lookup_helper.h"

extern "C" int _dyld_func_lookup(const char *name, void **address);

int dyld_func_lookup_helper(const char *name, void **address) {
    return _dyld_func_lookup(name, address);
}
