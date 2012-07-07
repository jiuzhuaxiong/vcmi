#include "UIFramework/CIntObject.h"
#include "AdventureMapClasses.h"
#include "CAdvmapInterface.h"
#include "GUIClasses.h"

#include "../lib/CGameState.h"

/*
 * CCreatureWindow.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class CCreature;
class CStackInstance;
class CAdventureMapButton;
class CGHeroInstance;
class CComponent;
class LRClickableAreaWText;
class CAdventureMapButton;
class CPicture;
class CCreaturePic;
class LRClickableAreaWTextComp;
class CSlider;
class CLabel;
struct QuestInfo;

extern CAdvMapInt *adventureInt;

const int QUEST_COUNT = 9;

class CQuestLabel : public LRClickableAreaWText, public CLabel
{
public:
	boost::function<void()> callback;

	CQuestLabel (int x=0, int y=0, EFonts Font = FONT_SMALL, EAlignment Align = TOPLEFT, const SDL_Color &Color = Colors::Cornsilk, const std::string &Text =  "")
		: CLabel (x, y, FONT_SMALL, TOPLEFT, Colors::Cornsilk, Text){};
	void clickLeft(tribool down, bool previousState);
};

class CQuestMinimap : public CMinimap
{
	void clickLeft(tribool down, bool previousState);
	void mouseMoved (const SDL_MouseMotionEvent & sEvent){};

public:

	const QuestInfo * currentQuest;

	CQuestMinimap (const Rect & position) : CMinimap (position){};
	//should be called to invalidate whole map - different player or level
	void update();
	void setLevel(int level);
	void addQuestMarks (const QuestInfo * q){};

	//void showAll(SDL_Surface * to){};
};

class CQuestLog : public CWindowObject
{
	int questIndex;
	const QuestInfo * currentQuest;

	const std::vector<QuestInfo> quests;
	std::vector<CQuestLabel *> labels;
	CTextBox * description;
	CQuestMinimap * minimap;
	CSlider * slider; //scrolls quests
	CAdventureMapButton *ok;

public:

	CQuestLog (const std::vector<QuestInfo> & Quests);

	~CQuestLog(){};

	void init ();
	void selectQuest (int which);
	void updateMinimap (int which){};
	void printDescription (int which){};
	void sliderMoved (int newpos);
	void recreateQuestList (int pos);
	void showAll (SDL_Surface * to);
};