/*
 * mock_IBattleInfoCallback.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#pragma once

#include "../../lib/battle/IBattleInfoCallback.h"

class IBattleInfoCallbackMock : public IBattleInfoCallback
{
public:

	MOCK_CONST_METHOD0(battleIsFinished, boost::optional<int>());

	MOCK_CONST_METHOD2(battleGetAllObstaclesOnPos, std::vector<std::shared_ptr<const CObstacleInstance>>(BattleHex, bool));
	MOCK_CONST_METHOD1(getAllAffectedObstaclesByStack, std::vector<std::shared_ptr<const CObstacleInstance>>(const battle::Unit *));

	MOCK_CONST_METHOD2(battleGetUnitByPos, const battle::Unit *(BattleHex, bool));
};




