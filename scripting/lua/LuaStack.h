/*
 * LuaStack.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include <lua.hpp>

namespace scripting
{

class LuaStack
{
public:
	LuaStack(lua_State * L_);
	void balance();
	void clear();

	void pushNil();
	void pushInteger(lua_Integer value);

	template<typename T, typename std::enable_if< std::is_integral<T>::value, int>::type = 0>
	void push(const T value)
	{
		pushInteger(static_cast<lua_Integer>(value));
	}

	template<typename T>
	void push(const boost::optional<T> & value)
	{
		if(value.is_initialized())
			push(value.get());
		else
			pushNil();
	}

	bool tryGetBool(int position, bool & value);
	bool tryGetInteger(int position, lua_Integer & value);
	bool tryGetFloat(int position, double & value);
	bool tryGetString(int position, std::string & value);

	template<typename Proxy>
	bool tryGetShared(int position, typename Proxy::Object & value)
	{
		void * raw = luaL_checkudata(L, position, Proxy::CLASSNAME.c_str());

		if(!raw)
			return false;

		value = *(static_cast<typename Proxy::Object *>(raw));
		return true;
	}

	int retNil();
	int retVoid();

private:
	lua_State * L;
	int initialTop;
};

}
