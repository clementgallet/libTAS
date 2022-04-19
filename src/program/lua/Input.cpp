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

#include "Input.h"
#include "../../shared/SingleInput.h"

#include <iostream>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static AllInputs* ai;

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

void Lua::Input::registerFunctions(Context* context)
{
    luaL_newlib(context->lua_state, input_functions);
    lua_setglobal(context->lua_state, "input");
}

void Lua::Input::registerInputs(AllInputs* frame_ai)
{
    ai = frame_ai;
}

int Lua::Input::clear(lua_State *L)
{
    ai->emptyInputs();
    return 0;
}

int Lua::Input::setKey(lua_State *L)
{
    unsigned int keysym = static_cast<unsigned int>(lua_tointeger(L, 1));
    int state = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_KEYBOARD, keysym, ""};
    ai->setInput(si, state);
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
    ai->setInput(si, x);
    si = {SingleInput::IT_POINTER_Y, 0, ""};
    ai->setInput(si, y);
    si = {SingleInput::IT_POINTER_MODE, 0, ""};
    ai->setInput(si, mode);
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
    int button = static_cast<int>(lua_tointeger(L, 1));
    int state = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_POINTER_B1 + button, 0, ""};
    ai->setInput(si, state);
    return 0;
}

int Lua::Input::getMouseButtons(lua_State *L)
{
    int button = static_cast<int>(lua_tointeger(L, 1));

    SingleInput si = {SingleInput::IT_POINTER_B1 + button, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setControllerButton(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    int button = static_cast<int>(lua_tointeger(L, 2));
    int state = static_cast<int>(lua_tointeger(L, 3));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | button, 0, ""};
    ai->setInput(si, state);
    return 0;
}

int Lua::Input::getControllerButton(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    int button = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | button, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setControllerAxis(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    int axis = static_cast<int>(lua_tointeger(L, 2));
    short value = static_cast<short>(lua_tointeger(L, 3));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | SingleInput::IT_CONTROLLER_AXIS_MASK | axis, 0, ""};
    ai->setInput(si, value);
    return 0;
}

int Lua::Input::getControllerAxis(lua_State *L)
{
    int controller = static_cast<int>(lua_tointeger(L, 1));
    int axis = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {(controller << SingleInput::IT_CONTROLLER_ID_SHIFT) | SingleInput::IT_CONTROLLER_AXIS_MASK | axis, 0, ""};
    lua_pushinteger(L, static_cast<lua_Integer>(ai->getInput(si)));
    return 1;
}

int Lua::Input::setFlag(lua_State *L)
{
    unsigned int flag = static_cast<unsigned int>(lua_tointeger(L, 1));
    int state = static_cast<int>(lua_tointeger(L, 2));
    
    SingleInput si = {SingleInput::IT_FLAG, flag, ""};
    ai->setInput(si, state);
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
    ai->setInput(si, num);
    si = {SingleInput::IT_FRAMERATE_DEN, 0, ""};
    ai->setInput(si, den);
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
    ai->setInput(si, sec);
    si = {SingleInput::IT_REALTIME_NSEC, 0, ""};
    ai->setInput(si, nsec);
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
