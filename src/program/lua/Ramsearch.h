#ifndef LIBTAS_LUARAMSEARCH_H_INCLUDED
#define LIBTAS_LUARAMSEARCH_H_INCLUDED

#include <stdint.h>
extern "C" {
#include <lua.h>
}

struct Context;

namespace Lua {

namespace Ramsearch {

    /* Register all functions */
    void registerFunctions(lua_State *L, Context* c);

    /* Start a new ramsearch */
    int newsearch(lua_State *L);

    /* Perform a new step on previously started search */
    int search(lua_State *L);

    /* Get current value of address at a given index of the search results */
    int get_current_value(lua_State *L);

    /* Get address at a given index of the search results */
    int get_address(lua_State *L);

    /* Set comparison operator for next search */
    int set_compare_operator(lua_State *L);

    /* Set comparison type for next search */
    int set_comparison_type(lua_State *L);

    /* Set comparison operator, and value in the "different" operator
     * case */
    void _set_compareop(const char *op, const char* value);

    /* Set "compare to" case, and value in the "specific value" case */
    void _set_compare_to(int type, const char* value);

}
}

#endif
