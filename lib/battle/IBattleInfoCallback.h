/*
 * IBattleInfoCallback.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "BattleHex.h"

struct CObstacleInstance;
class BFieldType;
class ETerrainType;

namespace battle
{
	class IUnitInfo;
	class Unit;
	using Units = std::vector<const Unit *>;
	using UnitFilter = std::function<bool(const Unit *)>;
}

class DLL_LINKAGE IBattleInfoCallback
{
public:
	virtual ETerrainType battleTerrainType() const = 0;
	virtual BFieldType battleGetBattlefieldType() const = 0;

	///return none if battle is ongoing; otherwise the victorious side (0/1) or 2 if it is a draw
	virtual boost::optional<int> battleIsFinished() const = 0;

	//blocking obstacles makes tile inaccessible, others cause special effects (like Land Mines, Moat, Quicksands)
	virtual std::vector<std::shared_ptr<const CObstacleInstance>> battleGetAllObstaclesOnPos(BattleHex tile, bool onlyBlocking = true) const = 0;
	virtual std::vector<std::shared_ptr<const CObstacleInstance>> getAllAffectedObstaclesByStack(const battle::Unit * unit) const = 0;
	virtual const battle::Unit * battleGetUnitByPos(BattleHex pos, bool onlyAlive = true) const = 0;
};


