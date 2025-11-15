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

#include "Input.h"

#include "../shared/inputs/SingleInput.h"
#include "../shared/inputs/AllInputs.h"

#include <iostream>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static AllInputs* ai;
static bool* modified;

/* List of functions to register */
static const luaL_Reg input_functions[] =
{
    { "clear", Lua::Input::clear},
    { "setKey", Lua::Input::setKey},
    { "getKey", Lua::Input::getKey},
    { "setMouseCoords", Lua::Input::setMouseCoords},
    { "getMouseCoords", Lua::Input::getMouseCoords},
    { "setMouseButtons", Lua::Input::setMouseButtons},
    { "getMouseButtons", Lua::Input::getMouseButtons},
    { "setControllerButton", Lua::Input::setControllerButton},
    { "getControllerButton", Lua::Input::getControllerButton},
    { "setControllerAxis", Lua::Input::setControllerAxis},
    { "getControllerAxis", Lua::Input::getControllerAxis},
    { "setFlag", Lua::Input::setFlag},
    { "getFlag", Lua::Input::getFlag},
    { "setFramerate", Lua::Input::setFramerate},
    { "getFramerate", Lua::Input::getFramerate},
    { "setRealtime", Lua::Input::setRealtime},
    { "getRealtime", Lua::Input::getRealtime},
    { NULL, NULL }
};

void Lua::Input::registerFunctions(lua_State *L)
{
    luaL_newlib(L, input_functions);
    lua_setglobal(L, "input");
}

void Lua::Input::registerInputs(AllInputs* frame_ai, bool* frame_modified)
{
    ai = frame_ai;
    modified = frame_modified;
    *modified = false;
}

int Lua::Input::clear(lua_State *L)
{
    ai->clear();
    *modified = true;
    return 0;
}

int Lua::Input::setKey(lua_State *L)
{
    unsigned int keysym = static_cast<unsigned int>(lua_tointeger(L, 1));
    int state = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_KEYBOARD, keysym, ""};
    if (ai->getInput(si) != state) {
        ai->setInput(si, state);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getKey(lua_State *L)
{
    unsigned int keysym = static_cast<unsigned int>(lua_tointeger(L, 1));

    SingleInput si = {SingleInput::IT_KEYBOARD, keysym, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setMouseCoords(lua_State *L)
{
    int x = static_cast<int>(lua_tointeger(L, 1));
    int y = static_cast<int>(lua_tointeger(L, 2));
    int mode = static_cast<int>(lua_tointeger(L, 3));
    
    SingleInput si = {SingleInput::IT_POINTER_X, 0, ""};
    if (ai->getInput(si) != x) {
        ai->setInput(si, x);
        *modified = true;
    }
    si = {SingleInput::IT_POINTER_Y, 0, ""};
    if (ai->getInput(si) != y) {
        ai->setInput(si, y);
        *modified = true;
    }
    si = {SingleInput::IT_POINTER_MODE, 0, ""};
    if (ai->getInput(si) != mode) {
        ai->setInput(si, mode);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getMouseCoords(lua_State *L)
{
    SingleInput si = {SingleInput::IT_POINTER_X, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    si = {SingleInput::IT_POINTER_Y, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    si = {SingleInput::IT_POINTER_MODE, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 3;
}

int Lua::Input::setMouseButtons(lua_State *L)
{
    unsigned int button = static_cast<unsigned int>(lua_tointeger(L, 1));
    int state = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_POINTER_BUTTON, button, ""};
    if (ai->getInput(si) != state) {
        ai->setInput(si, state);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getMouseButtons(lua_State *L)
{
    unsigned int button = static_cast<unsigned int>(lua_tointeger(L, 1));

    SingleInput si = {SingleInput::IT_POINTER_BUTTON, button, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setControllerButton(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    unsigned int button = static_cast<unsigned int>(lua_tointeger(L, 2));
    int state = static_cast<int>(lua_tointeger(L, 3));
    
    SingleInput si = {2*(controller-1)+SingleInput::IT_CONTROLLER1_BUTTON, button, ""};
    if (ai->getInput(si) != state) {
        ai->setInput(si, state);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getControllerButton(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    unsigned int button = static_cast<unsigned int>(lua_tointeger(L, 2));
    
    SingleInput si = {2*(controller-1)+SingleInput::IT_CONTROLLER1_BUTTON, button, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setControllerAxis(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    unsigned int axis = static_cast<unsigned int>(lua_tointeger(L, 2));
    short value = static_cast<short>(lua_tointeger(L, 3));
    
    SingleInput si = {2*(controller-1)+SingleInput::IT_CONTROLLER1_AXIS, axis, ""};
    if (ai->getInput(si) != value) {
        ai->setInput(si, value);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getControllerAxis(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    unsigned int axis = static_cast<unsigned int>(lua_tointeger(L, 2));
    
    SingleInput si = {2*(controller-1)+SingleInput::IT_CONTROLLER1_AXIS, axis, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setFlag(lua_State *L)
{
    unsigned int flag = static_cast<unsigned int>(lua_tointeger(L, 1));
    int state = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_FLAG, flag, ""};
    if (ai->getInput(si) != state) {
        ai->setInput(si, state);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getFlag(lua_State *L)
{
    unsigned int flag = static_cast<unsigned int>(lua_tointeger(L, 1));

    SingleInput si = {SingleInput::IT_FLAG, flag, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setFramerate(lua_State *L)
{
    int num = static_cast<int>(lua_tointeger(L, 1));
    int den = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_FRAMERATE_NUM, 0, ""};
    if (ai->getInput(si) != num) {
        ai->setInput(si, num);
        *modified = true;
    }
    si = {SingleInput::IT_FRAMERATE_DEN, 0, ""};
    if (ai->getInput(si) != den) {
        ai->setInput(si, den);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getFramerate(lua_State *L)
{
    SingleInput si = {SingleInput::IT_FRAMERATE_NUM, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    si = {SingleInput::IT_FRAMERATE_DEN, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 2;
}

int Lua::Input::setRealtime(lua_State *L)
{
    int sec = static_cast<int>(lua_tointeger(L, 1));
    int nsec = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_REALTIME_SEC, 0, ""};
    if (ai->getInput(si) != sec) {
        ai->setInput(si, sec);
        *modified = true;
    }
    si = {SingleInput::IT_REALTIME_NSEC, 0, ""};
    if (ai->getInput(si) != nsec) {
        ai->setInput(si, nsec);
        *modified = true;
    }
    return 0;
}

int Lua::Input::getRealtime(lua_State *L)
{
    SingleInput si = {SingleInput::IT_REALTIME_SEC, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    si = {SingleInput::IT_REALTIME_NSEC, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 2;
}
