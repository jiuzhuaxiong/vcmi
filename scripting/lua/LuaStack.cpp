/*
 * LuaStack.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "LuaStack.h"

namespace scripting
{

LuaStack::LuaStack(lua_State * L_)
	: L(L_)
{
	initialTop = lua_gettop(L);
}

void LuaStack::balance()
{
	lua_settop(L, initialTop);
}

void LuaStack::clear()
{
	lua_settop(L, 0);
}

void LuaStack::pushNil()
{
	lua_pushnil(L);
}

void LuaStack::pushInteger(lua_Integer value)
{
	lua_pushinteger(L, value);
}

bool LuaStack::tryGetBool(int position, bool & value)
{
	if(!lua_isboolean(L, position))
		return false;
	value = lua_toboolean(L, position);
	return true;
}

bool LuaStack::tryGetFloat(int position, double & value)
{
	if(!lua_isnumber(L, position))
		return false;
	value = lua_tonumber(L, position);
	return true;
}

bool LuaStack::tryGetInteger(int position, lua_Integer & value)
{
	if(!lua_isnumber(L, position))
		return false;

	value = lua_tointeger(L, position);
	return true;
}

bool LuaStack::tryGetString(int position, std::string & value)
{
	if(!lua_isstring(L, position))
		return false;

	size_t len = 0;
	auto raw = lua_tolstring(L, position, &len);
	value = std::string(raw, len);

	return true;
}

int LuaStack::retNil()
{
	clear();
	pushNil();
	return 1;
}

int LuaStack::retVoid()
{
	clear();
	return 0;
}

}
