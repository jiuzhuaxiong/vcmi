/*
 * ServerCb.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include <vcmi/scripting/Service.h>
#include "../../../lib/CGameInfoCallback.h"

#include "../LuaWrapper.h"

namespace scripting
{
namespace api
{

class ServerCbProxy : public OpaqueWrapper<ServerCb, ServerCbProxy>
{
public:
	using Wrapper = OpaqueWrapper<ServerCb, ServerCbProxy>;

	static int commitPackage(lua_State * L, ServerCb * object);

	static const std::vector<typename Wrapper::RegType> REGISTER;
};

}
}
