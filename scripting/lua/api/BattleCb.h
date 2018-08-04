/*
 * BattleCb.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "../../../lib/ScriptingService.h"
#include "../../../lib/battle/IBattleInfoCallback.h"

#include "../LuaWrapper.h"

namespace scripting
{
namespace api
{

class BattleCbProxy : public OpaqueWrapper<const BattleCb, BattleCbProxy>
{
public:
	using Wrapper = OpaqueWrapper<const BattleCb, BattleCbProxy>;

	static const std::vector<typename Wrapper::RegType> REGISTER;
	static const std::string CLASSNAME;

	static int battleIsFinished(lua_State * L, const BattleCb * object);
};

}
}
