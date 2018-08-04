/*
 * BattleCb.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "api/BattleCb.h"

#include "../LuaStack.h"
#include "../LuaCallWrapper.h"

namespace scripting
{
namespace api
{

const std::string BattleCbProxy::CLASSNAME = "Battle";

const std::vector<BattleCbProxy::RegType> BattleCbProxy::REGISTER =
{
	{
		"battleIsFinished",
		&BattleCbProxy::battleIsFinished
	}
};

int BattleCbProxy::battleIsFinished(lua_State * L, const BattleCb * object)
{
	LuaStack S(L);
	auto ret = object->battleIsFinished();
	S.push(ret);
	return 1;
}

}
}
