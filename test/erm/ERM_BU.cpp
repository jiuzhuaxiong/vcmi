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

		std::stringstream builder;
		builder << "VERM" << std::endl;
		builder << "!?PI;" << std::endl;
		builder << "!!BU:C?v851;" << std::endl;

		loadScript(VLC->scriptHandler->erm, builder.str());

		run();

		JsonNode actualState = context->saveState();

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

};

class ERM_BU_E : public ERM_BU
{

};

class ERM_BU_G : public ERM_BU
{

};

class ERM_BU_M : public ERM_BU
{

};

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
