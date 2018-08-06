/*
 * ERM_BU.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "../scripting/ScriptFixture.h"

#include "../../lib/NetPacks.h"

namespace test
{
namespace scripting
{

using namespace ::testing;

class ERM_BU : public Test, public ScriptFixture
{
protected:
	void SetUp() override
	{
		ScriptFixture::setUp();
	}
};

class ERM_BU_C : public ERM_BU
{
protected:
	void doTest(boost::optional<int> input, double output)
	{
		EXPECT_CALL(binfoMock, battleIsFinished()).WillOnce(Return(input));

		std::stringstream source;
		source << "VERM" << std::endl;
		source << "!?PI;" << std::endl;
		source << "!!BU:C?v851;" << std::endl;

		JsonNode actualState = runScript(VLC->scriptHandler->erm, source.str());

		SCOPED_TRACE("\n" + subject->code);

		EXPECT_EQ(actualState["ERM"]["v"]["851"], JsonUtils::floatNode(output)) << actualState.toJson(true);
	}
};

TEST_F(ERM_BU_C, Finished)
{
	doTest(boost::make_optional(1), 1);
}

TEST_F(ERM_BU_C, NotFinished)
{
	doTest(boost::none, 0);
}

class ERM_BU_D : public ERM_BU
{
protected:
	void doTest(double output)
	{

		std::stringstream source;
		source << "VERM" << std::endl;
		source << "!?PI;" << std::endl;
		source << "!!BU:D75/?v1;" << std::endl;

		JsonNode actualState = runScript(VLC->scriptHandler->erm, source.str());

		SCOPED_TRACE("\n" + subject->code);

		EXPECT_EQ(actualState["ERM"]["v"]["1"], JsonUtils::floatNode(output)) << actualState.toJson(true);
	}
};

TEST_F(ERM_BU_D, EmptyField)
{
	EXPECT_CALL(binfoMock, battleGetUnitByPos(_,_)).WillOnce(Return(nullptr));
	doTest(-1);
}

TEST_F(ERM_BU_D, Alive)
{
	auto & unit = unitsFake.add(BattleSide::ATTACKER);
	EXPECT_CALL(binfoMock, battleGetUnitByPos(Eq(BattleHex(75)), Eq(false))).WillOnce(Return(&unit));
	EXPECT_CALL(unit, alive()).WillOnce(Return(true));
	doTest(-2);
}

TEST_F(ERM_BU_D, Dead)
{
	auto & unit = unitsFake.add(BattleSide::ATTACKER);
	EXPECT_CALL(binfoMock, battleGetUnitByPos(Eq(BattleHex(75)), Eq(false))).WillOnce(Return(&unit));
	EXPECT_CALL(unit, alive()).WillOnce(Return(false));
	EXPECT_CALL(unit, unitId()).WillOnce(Return(47));
	doTest(47);
}


class ERM_BU_E : public ERM_BU
{
protected:
	void doTest(double output)
	{
		std::stringstream source;
		source << "VERM" << std::endl;
		source << "!?PI;" << std::endl;
		source << "!!BU:E75/?v1;" << std::endl;

		JsonNode actualState = runScript(VLC->scriptHandler->erm, source.str());

		SCOPED_TRACE("\n" + subject->code);

		EXPECT_EQ(actualState["ERM"]["v"]["1"], JsonUtils::floatNode(output)) << actualState.toJson(true);
	}
};

TEST_F(ERM_BU_E, EmptyField)
{
	EXPECT_CALL(binfoMock, battleGetUnitByPos(_,_)).WillOnce(Return(nullptr));
	doTest(-1);
}

TEST_F(ERM_BU_E, Alive)
{
	auto & unit = unitsFake.add(BattleSide::ATTACKER);
	EXPECT_CALL(binfoMock, battleGetUnitByPos(Eq(BattleHex(75)), Eq(false))).WillOnce(Return(&unit));
	EXPECT_CALL(unit, alive()).WillOnce(Return(true));
	EXPECT_CALL(unit, unitId()).WillOnce(Return(47));
	doTest(47);
}

TEST_F(ERM_BU_E, Dead)
{
	auto & unit = unitsFake.add(BattleSide::ATTACKER);
	EXPECT_CALL(binfoMock, battleGetUnitByPos(Eq(BattleHex(75)), Eq(false))).WillOnce(Return(&unit));
	EXPECT_CALL(unit, alive()).WillOnce(Return(false));
	doTest(-1);
}


class ERM_BU_G : public ERM_BU
{

};

TEST_F(ERM_BU_G, Get)
{
	std::stringstream source;
	source << "VERM" << std::endl;
	source << "!?FU1;" << std::endl;
	source << "!!BU:G?v1;" << std::endl;
	source << "!?FU2;" << std::endl;
	source << "!!BU:G?v2;" << std::endl;

	loadScript(VLC->scriptHandler->erm, source.str());
	run();

	EXPECT_CALL(binfoMock, battleGetBattlefieldType()).WillOnce(Return(BFieldType::SNOW_TREES));
	context->callGlobal("FU1", JsonNode());

	EXPECT_CALL(binfoMock, battleGetBattlefieldType()).WillOnce(Return(BFieldType::EVIL_FOG));
	context->callGlobal("FU2", JsonNode());

	JsonNode actualState = context->saveState();

	EXPECT_EQ(actualState["ERM"]["v"]["1"], JsonUtils::floatNode(-1)) << actualState.toJson(true);
	EXPECT_EQ(actualState["ERM"]["v"]["2"], JsonUtils::floatNode(4)) << actualState.toJson(true);
}

//TODO: ERM_BU_G Set

class ERM_BU_M : public ERM_BU
{

};

TEST_F(ERM_BU_M, Simple)
{
	std::stringstream source;
	source << "VERM" << std::endl;
	source << "!?FU1;" << std::endl;
	source << "!!BU:M^Test 1^;" << std::endl;
	source << "!?FU2;" << std::endl;
	source << "!!VRz3:S^Test 2^;" << std::endl;
	source << "!!BU:Mz3;" << std::endl;

	loadScript(VLC->scriptHandler->erm, source.str());
	run();

	auto checkApply1 = [&](BattleLogMessage * pack)
	{
		EXPECT_EQ(pack->lines.size(), 1);

		if(!pack->lines.empty())
			EXPECT_EQ(pack->lines[0].toString(), "Test 1");
	};

	EXPECT_CALL(battleApplierMock, apply(Matcher<BattleLogMessage *>(_))).WillOnce(Invoke(checkApply1));

	context->callGlobal(&battleApplierMock, "FU1", JsonNode());

	auto checkApply2 = [&](BattleLogMessage * pack)
	{
		EXPECT_EQ(pack->lines.size(), 1);

		if(!pack->lines.empty())
			EXPECT_EQ(pack->lines[0].toString(), "Test 2");
	};

	EXPECT_CALL(battleApplierMock, apply(Matcher<BattleLogMessage *>(_))).WillOnce(Invoke(checkApply2));

	context->callGlobal(&battleApplierMock, "FU2", JsonNode());
}

class ERM_BU_O : public ERM_BU
{

};

class ERM_BU_R : public ERM_BU
{

};

class ERM_BU_S : public ERM_BU
{

};

class ERM_BU_T : public ERM_BU
{

};

class ERM_BU_V : public ERM_BU
{

};

}
}
