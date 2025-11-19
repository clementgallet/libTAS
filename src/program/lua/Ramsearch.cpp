#include "Ramsearch.h"

#include "ramsearch/MemAccess.h"
#include "ramsearch/MemScanner.h"
#include "ramsearch/MemScannerThread.h"
#include "ui/RamSearchModel.h"


#include <iostream>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static Context* context;
static RamSearchModel *ramSearchModel = nullptr;

/* search variables definitions with default values */
static int memflags = 0 | MemSection::MemNoSpecial | MemSection::MemNoRO | MemSection::MemNoExec; // 7
static int value_type = 0;
static int alignment = 0;
static int compare_type_int = 0; // CompareType::Previous;
static CompareType compare_type;
static CompareOperator compare_operator = CompareOperator::Equal;
static int compare_value_int = 0;
static MemValueType compare_value;
static int different_value_int = 0;
static MemValueType different_value;
static const char* begin_address_default = "0000000000000000";
static uintptr_t begin_address;
static const char* end_address_default = "00007fffffffffff";
static uintptr_t end_address;


/* List of functions to register */
static const luaL_Reg ramsearch_functions[] =
{
    { "newsearch", Lua::Ramsearch::newsearch},
    { "search", Lua::Ramsearch::search},
    { "get_current_value", Lua::Ramsearch::get_current_value},
    { "set_comparison_operator", Lua::Ramsearch::set_compare_operator},
    { "set_comparison_type", Lua::Ramsearch::set_comparison_type},
    { "get_address", Lua::Ramsearch::get_address},
    { NULL, NULL }
};

void Lua::Ramsearch::registerFunctions(lua_State *L, Context* c)
{
    context = c;
    luaL_newlib(L, ramsearch_functions);
    lua_setglobal(L, "ramsearch");
}

int Lua::Ramsearch::newsearch(lua_State *L)
{
    ramSearchModel = new RamSearchModel(context);

    value_type = luaL_optinteger(L, 1, value_type);
    alignment = luaL_optinteger(L, 2, alignment);
    compare_type_int = luaL_optinteger(L, 3, compare_type_int);
    compare_value_int = luaL_optinteger(L, 4, compare_value_int);
    _set_compare_to(compare_type_int, compare_value_int);
    const char* compare_operator_str = luaL_optstring(L, 5, nullptr);
    if (compare_operator_str != nullptr) {
        different_value_int = luaL_optinteger(L, 6, different_value_int);
        _set_compareop(compare_operator_str, different_value_int);
    }
    memflags = luaL_optinteger(L, 7, memflags);
    begin_address = std::strtoul(luaL_optstring(L, 8, begin_address_default), nullptr, 16);
    end_address = std::strtoul(luaL_optstring(L, 9, end_address_default), nullptr, 16);

    int err = ramSearchModel->memscanner.first_scan(memflags, value_type, alignment, compare_type, compare_operator, compare_value, different_value, begin_address, end_address);

    switch (err) {
        case MemScannerThread::ESTOPPED:
            std::cerr << "The search was interupted by the user";
            break;
        case MemScannerThread::EOUTPUT:
            std::cerr << "The search results could not be written to disk";
            break;
        case MemScannerThread::EINPUT:
            std::cerr << "The previous search results could not be read correctly";
            break;
        case MemScannerThread::EPROCESS:
            std::cerr << "There was an error in the search process";
            break;
    }

    // return number of results
    lua_pushinteger(L, static_cast<lua_Integer>(ramSearchModel->scanCount()));
    return 1;
}

int Lua::Ramsearch::search(lua_State *L)
{
    if (ramSearchModel == nullptr) {
        std::cerr << "Lua ramsearch: Error: attempting to perforum a search without first creating a newsearch";
        lua_pushinteger(L, static_cast<lua_Integer>(0));
        return 1;
    }

    if (ramSearchModel->scanCount() == 0) {
        lua_pushinteger(L, static_cast<lua_Integer>(0));
        return 1;
    }

    compare_type_int = luaL_optinteger(L, 1, compare_type_int);
    compare_value_int = luaL_optinteger(L, 2, compare_value_int);
    _set_compare_to(compare_type_int, compare_value_int);
    const char* compare_operator_str = luaL_optstring(L, 3, nullptr);
    if (compare_operator_str != nullptr) {
        int value = luaL_optinteger(L, 4, different_value_int);
        _set_compareop(compare_operator_str, value);
    }

    int err = ramSearchModel->memscanner.scan(false, compare_type, compare_operator, compare_value, different_value);

    switch (err) {
        case MemScannerThread::ESTOPPED:
            std::cerr << "The search was interupted by the user";
            break;
        case MemScannerThread::EOUTPUT:
            std::cerr << "The search results could not be written to disk";
            break;
        case MemScannerThread::EINPUT:
            std::cerr << "The previous search results could not be read correctly";
            break;
        case MemScannerThread::EPROCESS:
            std::cerr << "There was an error in the search process";
            break;
    }

    // return number of results
    lua_pushinteger(L, static_cast<lua_Integer>(ramSearchModel->scanCount()));
    return 1;
}

int Lua::Ramsearch::get_current_value(lua_State *L)
{
    if (ramSearchModel == nullptr) {
        std::cerr << "Lua ramsearch: Error: attempting to access search results first creating a newsearch";
        lua_pushinteger(L, static_cast<lua_Integer>(0));
        return 1;
    }

    int index = static_cast<int>(lua_tointeger(L, 1));
    MemValueType current_value = ramSearchModel->memscanner.get_current_value(index);

    switch (value_type) {
    case RamChar:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_int8_t));
        break;
    case RamUnsignedChar:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_uint8_t));
        break;
    case RamShort:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_int16_t));
        break;
    case RamUnsignedShort:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_uint16_t));
        break;
    case RamInt:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_int32_t));
        break;
    case RamUnsignedInt:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_uint32_t));
        break;
    case RamLong:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_int64_t));
        break;
    case RamUnsignedLong:
        lua_pushinteger(L, static_cast<lua_Integer>(current_value.v_uint64_t));
        break;
    case RamFloat:
        lua_pushnumber(L, static_cast<lua_Number>(current_value.v_float));
        break;
    case RamDouble:
        lua_pushnumber(L, static_cast<lua_Number>(current_value.v_double));
        break;
    case RamCString:
        lua_pushstring(L, current_value.v_cstr);
        break;
    case RamArray: {
        lua_newtable(L);
        int size = current_value.v_array[RAM_ARRAY_MAX_SIZE];
        for (int i = 0; i < size; i++) {
            lua_pushinteger(L, current_value.v_array[i]);
            lua_rawseti(L, -2, i + 1);
        }
        break;
    }
    default:
        std::cerr << "Lua ramsearch: Error: value type not recognized\n";
        break;
    }
    return 1;
}

void Lua::Ramsearch::_set_compareop(const char *op, int value)
{
    if (strcmp(op, "==") == 0)
        compare_operator = CompareOperator::Equal;
    else if (strcmp(op, "!=") == 0)
        compare_operator = CompareOperator::NotEqual;
    else if (strcmp(op, "<") == 0)
        compare_operator = CompareOperator::Less;
    else if (strcmp(op, ">") == 0)
        compare_operator = CompareOperator::Greater;
    else if (strcmp(op, "<=") == 0)
        compare_operator = CompareOperator::LessEqual;
    else if (strcmp(op, ">=") == 0)
        compare_operator = CompareOperator::GreaterEqual;
    else if (strcmp(op, "!") == 0) {
        compare_operator = CompareOperator::Different;
        // we convert the int to string, then to MemValueType to avoid
        // having to implement the int->MemValueType converter
        different_value = MemValue::from_string(std::to_string(value).c_str(), RamInt, false);
    }
}

int Lua::Ramsearch::set_compare_operator(lua_State *L)
{
    const char* op = lua_tostring(L, 1);
    different_value_int = static_cast<int>(luaL_optinteger(L, 2, different_value_int));
    _set_compareop(op, different_value_int);
    return 0;
}

void Lua::Ramsearch::_set_compare_to(int type, int value)
{
    if (type == 0) {
        compare_type = CompareType::Previous;
    } else if (type == 1) {
        compare_type = CompareType::Value;
        compare_value = MemValue::from_string(std::to_string(value).c_str(), value_type, false);
    }
}

int Lua::Ramsearch::set_comparison_type(lua_State *L)
{
    compare_type_int = static_cast<int>(lua_tointeger(L, 1));
    compare_value_int = luaL_optinteger(L, 2, compare_value_int);
    _set_compare_to(compare_type_int, compare_value_int);
    return 0;
}

int Lua::Ramsearch::get_address(lua_State *L)
{
    if (ramSearchModel == nullptr) {
        std::cerr << "Lua ramsearch: Error: attempting to access search results first creating a newsearch";
        lua_pushinteger(L, static_cast<lua_Integer>(0));
        return 1;
    }

    int index = static_cast<int>(lua_tointeger(L, 1));
    uintptr_t address = ramSearchModel->memscanner.get_address(index);
    std::ostringstream addr_hex;
    addr_hex << std::hex << address;
    lua_pushstring(L, addr_hex.str().c_str());
    return 1;
}
