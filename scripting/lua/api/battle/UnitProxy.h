/*
 * UnitProxy.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "../../../../lib/ScriptingService.h"
#include "../../../../lib/battle/Unit.h"

#include "../../LuaWrapper.h"

namespace scripting
{
namespace api
{
namespace battle
{
using ::battle::Unit;

class UnitProxy : public OpaqueWrapper<const Unit, UnitProxy>
{
public:
	using Wrapper = OpaqueWrapper<const Unit, UnitProxy>;

	static const std::vector<typename Wrapper::RegType> REGISTER;

	static int isAlive(lua_State * L, const Unit * object);
	static int unitId(lua_State * L, const Unit * object);
};

}
}
}
