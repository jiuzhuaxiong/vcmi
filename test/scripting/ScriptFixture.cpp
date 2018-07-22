/*
 * ScriptFixture.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"

#include "ScriptFixture.h"

namespace test
{
using namespace ::testing;
using namespace ::scripting;

ScriptFixture::BattleFake::BattleFake(std::shared_ptr<PoolMock> pool_)
	: CBattleInfoCallback(),
	BattleStateMock(),
	pool(pool_)
{
}

void ScriptFixture::BattleFake::setUp()
{
	CBattleInfoCallback::setBattle(this);
}

Pool * ScriptFixture::BattleFake::getContextPool() const
{
	return pool.get();
}

ScriptFixture::ScriptFixture() =  default;

ScriptFixture::~ScriptFixture() = default;

void ScriptFixture::loadScript(const JsonNode & scriptConfig)
{
	subject = VLC->scriptHandler->loadFromJson(scriptConfig, "test");

	GTEST_ASSERT_NE(subject, nullptr);

	context = subject->createContext(&environmentMock);

	EXPECT_CALL(*pool, getContext(Eq(subject.get()))).WillRepeatedly(Return(context));
}

void ScriptFixture::loadScript(ModulePtr module, const std::string & scriptSource)
{
	subject = std::make_shared<ScriptImpl>(VLC->scriptHandler);

	subject->host = module;
	subject->sourceText = scriptSource;
	subject->identifier = "test";
	subject->compile();

	context = subject->createContext(&environmentMock);

	EXPECT_CALL(*pool, getContext(Eq(subject.get()))).WillRepeatedly(Return(context));
}

void ScriptFixture::setUp()
{
	pool = std::make_shared<PoolMock>();

	battleFake = std::make_shared<BattleFake>(pool);
	battleFake->setUp();

	EXPECT_CALL(environmentMock, battle()).WillRepeatedly(Return(battleFake.get()));
	EXPECT_CALL(environmentMock, game()).WillRepeatedly(Return(&infoMock));
	EXPECT_CALL(environmentMock, logger()).WillRepeatedly(Return(&loggerMock));
}

void ScriptFixture::run(const JsonNode & scriptState)
{
	context->run(scriptState);
}

}
