#include "stdafx.h"
#include "CPreGame.h"
#include "SDL.h"
#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations
#include "boost/algorithm/string.hpp"
//#include "boost/foreach.hpp"
#include "zlib.h"
#include "timeHandler.h"
#include <sstream>
#include "SDL_Extensions.h"
#include "CGameInfo.h"
#include <cmath>
#define CGI (CGameInfo::mainObj)

extern SDL_Surface * ekran;
extern SDL_Color tytulowy, tlo, zwykly ;
extern TTF_Font * TNRB16, *TNR, *GEOR13, *GEORXX;

SDL_Rect genRect(int hh, int ww, int xx, int yy);
SDL_Color genRGB(int r, int g, int b, int a=0);
void blitAt(SDL_Surface * src, int x, int y, SDL_Surface * dst=ekran);
//void printAt(std::string text, int x, int y, TTF_Font * font, SDL_Color kolor=tytulowy, SDL_Surface * dst=ekran);
CPreGame * CPG;
void updateRect (SDL_Rect * rect, SDL_Surface * scr = ekran);
bool isItIn(const SDL_Rect * rect, int x, int y);

namespace fs = boost::filesystem;
namespace s = CSDL_Ext;


void OverButton::show()
{
	blitAt(imgs->ourImages[state].bitmap,pos.x,pos.y);
	updateRect(&pos);
}
void OverButton::press(bool down)
{
	int i;
	if (down) state=i=1;
	else state=i=0;
	SDL_BlitSurface(imgs->ourImages[i].bitmap,NULL,ekran,&pos);
	updateRect(&pos);
}

template <class T> void SetrButton<T>::press(bool down=true)
{
	if (!down && state==1)
		*poin=key;
	OverButton::press(down);
}

template <class T> void Button<T>::hover(bool on=true)
{
	if (!highlightable) return;
	int i;
	if (on) state=i=2;
	else state=i=0;
	SDL_BlitSurface(imgs->ourImages[i].bitmap,NULL,ekran,&pos);
	updateRect(&pos);
}
template <class T> void Button<T>::select(bool on)
{
	int i;
	if (on) state=i=3;
	else state=i=0;
	SDL_BlitSurface(imgs->ourImages[i].bitmap,NULL,ekran,&pos);
	updateRect(&pos);
	if (ourGroup && on && ourGroup->type==1)
	{
		if (ourGroup->selected && ourGroup->selected!=this)
			ourGroup->selected->select(false);
		ourGroup->selected =this;
	}
}

//void Slider::clickDown(int x, int y, bool bzgl=true);
//void Slider::clickUp(int x, int y, bool bzgl=true);
//void Slider::mMove(int x, int y, bool bzgl=true);

void Slider<>::updateSlid()
{
	float perc = ((float)whereAreWe)/((float)positionsAmnt-capacity);
	float myh;
	if (vertical)
	{
		myh=perc*((float)pos.h-48)+pos.y+16;
		SDL_FillRect(ekran,&genRect(pos.h-32,pos.w,pos.x,pos.y+16),0);
		blitAt(slider.imgs->ourImages[0].bitmap,pos.x,(int)myh);
		slider.pos.y=(int)myh;
	}
	else
	{
		myh=perc*((float)pos.w-48)+pos.x+16;
		SDL_FillRect(ekran,&genRect(pos.h,pos.w-32,pos.x+16,pos.y),0);
		blitAt(slider.imgs->ourImages[0].bitmap,(int)myh,pos.y);
		slider.pos.x=(int)myh;
	}
	updateRect(&pos);
}

void Slider<>::moveDown()
{
	if (whereAreWe<positionsAmnt-capacity)
		(CPG->*fun)(++whereAreWe);
	updateSlid();
}
void Slider<>::moveUp()
{
	if (whereAreWe>0)
		(CPG->*fun)(--whereAreWe);
	updateSlid();
}
//void Slider::moveByOne(bool up);
Slider<>::Slider(int x, int y, int h, int amnt, int cap, bool ver)
{
	vertical=ver;
	positionsAmnt = amnt;
	capacity = cap;
	if (ver)
	{
		pos = genRect(h,16,x,y);
		down = Button<void(Slider::*)()>(genRect(16,16,x,y+h-16),&Slider::moveDown,CGI->spriteh->giveDef("SCNRBDN.DEF"),false);
		up = Button<void(Slider::*)()>(genRect(16,16,x,y),&Slider::moveUp,CGI->spriteh->giveDef("SCNRBUP.DEF"),false);
		slider = Button<void(Slider::*)()>(genRect(16,16,x,y+16),NULL,CGI->spriteh->giveDef("SCNRBSL.DEF"),false);
	}
	else
	{
		pos = genRect(16,h,x,y);
		down = Button<void(Slider::*)()>(genRect(16,16,x+h-16,y),&Slider::moveDown,CGI->spriteh->giveDef("SCNRBRT.DEF"),false);
		up = Button<void(Slider::*)()>(genRect(16,16,x,y),&Slider::moveUp,CGI->spriteh->giveDef("SCNRBLF.DEF"),false);
		slider = Button<void(Slider::*)()>(genRect(16,16,x+16,y),NULL,CGI->spriteh->giveDef("SCNRBSL.DEF"),false);
	}
	moving = false;
	whereAreWe=0;
}
void Slider<>::deactivate()
{
	CPG->interested.erase(std::find(CPG->interested.begin(),CPG->interested.end(),this));
}	

void Slider<>::activate()
{
	SDL_FillRect(ekran,&pos,0);
	up.show();
	down.show();
	slider.show();
	//SDL_Flip(ekran);
	CSDL_Ext::update(ekran);
	CPG->interested.push_back(this);
}	

void Slider<>::handleIt(SDL_Event sEvent)
{
	if ((sEvent.type==SDL_MOUSEBUTTONDOWN) && (sEvent.button.button == SDL_BUTTON_LEFT))
	{
		if (isItIn(&down.pos,sEvent.motion.x,sEvent.motion.y))
		{
			down.press();
		}
		else if (isItIn(&up.pos,sEvent.motion.x,sEvent.motion.y))
		{
			up.press();
		}
		else if (isItIn(&slider.pos,sEvent.motion.x,sEvent.motion.y))
		{
			//slider.press();
			moving=true;
		}
		else if (isItIn(&pos,sEvent.motion.x,sEvent.motion.y))
		{
			float dy;
			float pe;
			if (vertical)
			{
				dy = sEvent.motion.y-pos.y-16;
				pe = dy/((float)(pos.h-32));
				if (pe>1) pe=1;
				if (pe<0) pe=0;
			}
			else
			{
				dy = sEvent.motion.x-pos.x-16;
				pe = dy/((float)(pos.w-32));
				if (pe>1) pe=1;
				if (pe<0) pe=0;
			}
			whereAreWe = pe*(positionsAmnt-capacity);
			if (whereAreWe<0)whereAreWe=0;
			updateSlid();
			(CPG->*fun)(whereAreWe);
		}
	}
	else if ((sEvent.type==SDL_MOUSEBUTTONUP) && (sEvent.button.button == SDL_BUTTON_LEFT))
	{

		if ((down.state==1) && isItIn(&down.pos,sEvent.motion.x,sEvent.motion.y))
		{
			(this->*down.fun)();
		}
		if ((up.state==1) && isItIn(&up.pos,sEvent.motion.x,sEvent.motion.y))
		{
			(this->*up.fun)();
		}
		if (down.state==1) down.press(false);
		if (up.state==1) up.press(false);
		if (moving)
		{
			//slider.press();
			moving=false;
		}
	}
	else if (sEvent.type==SDL_KEYDOWN)
	{
		switch (sEvent.key.keysym.sym)
		{
		case (SDLK_UP):
			CPG->ourScenSel->mapsel.moveByOne(true);
			break;
		case (SDLK_DOWN):
			CPG->ourScenSel->mapsel.moveByOne(false);
			break;
		}
	}
	else if (moving && sEvent.type==SDL_MOUSEMOTION)
	{
		if (isItIn(&genRect(pos.h,pos.w+64,pos.x-32,pos.y),sEvent.motion.x,sEvent.motion.y))
		{
			int my;
			int all;
			float ile;
			if (vertical)
			{
				my = sEvent.motion.y-(pos.y+16);
				all =pos.h-48;
				ile = (float)my / (float)all;
				if (ile>1) ile=1;
				if (ile<0) ile=0;
			}
			else
			{
				my = sEvent.motion.x-(pos.x+16);
				all =pos.w-48;
				ile = (float)my / (float)all;
				if (ile>1) ile=1;
				if (ile<0) ile=0;
			}
			int ktory = ile*(positionsAmnt-capacity);
			if (ktory!=whereAreWe)
			{
				whereAreWe=ktory;
				updateSlid();
			}
			(CPG->*fun)(whereAreWe);
		}
	}
	
	/*else if ((sEvent.type==SDL_MOUSEBUTTONUP) && (sEvent.button.button == SDL_BUTTON_LEFT))
	{
		if (ourScenSel->pressed)
		{
			ourScenSel->pressed->press(false);
			ourScenSel->pressed=NULL;
		}
		for (int i=0;i<btns.size(); i++)
		{
			if (isItIn(&btns[i]->pos,sEvent.motion.x,sEvent.motion.y))
			{
				if (btns[i]->selectable)
					btns[i]->select(true);
				if (btns[i]->fun)
					(this->*(btns[i]->fun))();
				return;
			}
		}


		if (isItIn(&down.pos,sEvent.motion.x,sEvent.motion.y))
		{
			(this->*down.fun)();
		}
		if (isItIn(&up.pos,sEvent.motion.x,sEvent.motion.y))
		{
			(this->*up.fun)();
		}
		if (isItIn(&slider.pos,sEvent.motion.x,sEvent.motion.y))
		{
			(this->*slider.fun)();
		}

	}*/
}

/********************************************************************************************/
void PreGameTab::show()
{
	if (CPG->currentTab)
		CPG->currentTab->hide();
	showed=true;
	CPG->currentTab=this;
}
void PreGameTab::hide()
{
	showed=false;
	CPG->currentTab=NULL;
}
PreGameTab::PreGameTab()
{
	showed=false;
}


Options::PlayerOptions::PlayerOptions(int serial, int player)
	:left(genRect(24,11,163,133+serial*50),CPG->ourOptions->left,true,-1), //left castle arrow
	right(genRect(24,11,224,133+serial*50),CPG->ourOptions->right,false,-1) //right castle arrow
{
	left.playerID=right.playerID=player;
	left.serialID=right.serialID=serial;
}
void Options::OptionSwitch::press(bool down)
{
	OverButton::press(down);
	StartInfo::PlayerSettings * ourOpt = &CPG->ret.playerInfos[serialID];
	PlayerInfo * ourInf = &CPG->ourScenSel->mapsel.ourMaps[CPG->ourScenSel->mapsel.selected].players[playerID];
	int dir = (left) ? (-1) : (1);
	if (down) return;
	switch (which)
	{
	case -1:
		{
			if (ourOpt->castle==-2) //no castle - no change
				return;
			else if (ourOpt->castle==-1) //random => first/last available
			{
				int pom = (left) ? (F_NUMBER-1) : (0); // last or first
				for (;pom>=0 && pom<F_NUMBER;pom+=dir)
				{
					if (((int)pow((double)2,pom))&ourInf->allowedFactions)
					{
						ourOpt->castle=pom;
						break;
					}
					else continue;
				}
			}
			else 
			{
				ourOpt->castle+=dir;
				if (ourOpt->castle>=F_NUMBER || ourOpt->castle<0)
					ourOpt->castle=-1;
				break;
			}
			break;
		}
	}
	CPG->ourOptions->showIcon(which,serialID,false);
}
void Options::showIcon (int what, int nr, bool abs) //what: -1=castle, 0=hero, 1=bonus, 2=all; abs=true -> nr is absolute
{
	if (what==-2)
	{
		showIcon(-1,nr,abs);
	}
	int ab, se;
	if (!abs)
	{
		ab = CPG->ret.playerInfos[nr].color;
		se = nr;
	}
	else
	{
		ab = nr;
		for (int i=0; i<CPG->ret.playerInfos.size();i++)
		{
			if (CPG->ret.playerInfos[i].color==nr)
			{
				se=i;
				break;
			}
		}
	}
	StartInfo::PlayerSettings * ourOpt = &CPG->ret.playerInfos[se];
	int pom=ourOpt->castle;
	switch (what)
	{
	case -1:
		{
			if (ourOpt->castle<F_NUMBER && ourOpt->castle>=0)
			{
				blitAtWR(CGI->townh->getPic(ourOpt->castle,true,false),176,130+50*se);
			}
			else if (ourOpt->castle==-1)
			{
				blitAtWR(CPG->ourOptions->rCastle,176,130+50*se);
			}
			else if (ourOpt->castle==-2)
			{
				blitAtWR(CPG->ourOptions->nCastle,176,130+50*se);
			}
			break;
		}
	}
}
Options::~Options()
{
	if (!inited) return;
	for (int i=0; i<bgs.size();i++)
		SDL_FreeSurface(bgs[i]);
	SDL_FreeSurface(bg);
	SDL_FreeSurface(rHero);
	SDL_FreeSurface(rCastle);
	SDL_FreeSurface(nHero);
	SDL_FreeSurface(nCastle);
	delete turnLength;
	delete left;
	delete right;

}
void Options::init()
{
	inited=true;
	bg = CGI->bitmaph->loadBitmap("ADVOPTBK.bmp");
	SDL_SetColorKey(bg,SDL_SRCCOLORKEY,SDL_MapRGB(bg->format,0,255,255));
	left = CGI->spriteh->giveDef("ADOPLFA.DEF");
	right = CGI->spriteh->giveDef("ADOPRTA.DEF");
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPRPNL.bmp"));
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPBPNL.bmp"));
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPYPNL.bmp"));
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPGPNL.bmp"));
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPOPNL.bmp")); 
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPPPNL.bmp"));
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPTPNL.bmp"));
	bgs.push_back(CGI->bitmaph->loadBitmap("ADOPSPNL.bmp"));
	rHero = CGI->bitmaph->loadBitmap("HPSRAND1.bmp");
	rCastle = CGI->bitmaph->loadBitmap("HPSRAND0.bmp");
	nHero = CGI->bitmaph->loadBitmap("HPSRAND6.bmp");
	nCastle = CGI->bitmaph->loadBitmap("HPSRAND5.bmp");
	turnLength = new Slider<>(57,558,195,11,1,false);
	turnLength->fun=&CPreGame::setTurnLength;
}
void Options::show()
{
	if (showed)return;
	PreGameTab::show();
	MapSel & ms = CPG->ourScenSel->mapsel;
	blitAt(bg,2,6);
	CPG->ourScenSel->listShowed=false;
	for (int i=0;i<CPG->btns.size();i++)
	{
		if (CPG->btns[i]->ID!=10) //leave only right panel buttons
		{
			CPG->btns.erase(CPG->btns.begin()+i);
			i--;
		}
	}
	CPG->interested.clear();
	CSDL_Ext::printAtMiddle("Advanced Options",225,35,GEORXX);
	int playersSoFar=0;
	for (int i=0;i<PLAYER_LIMIT;i++)
	{
		if (!(ms.ourMaps[ms.selected].players[i].canComputerPlay || ms.ourMaps[ms.selected].players[i].canComputerPlay))
			continue;
		blitAt(bgs[i],56,128+playersSoFar*50);
		poptions.push_back(new PlayerOptions(playersSoFar,i));
		poptions[poptions.size()-1]->nr=playersSoFar;
		poptions[poptions.size()-1]->color=(Ecolor)i;
		poptions[poptions.size()-1]->left.show();
		poptions[poptions.size()-1]->right.show();
		CPG->btns.push_back(&poptions[poptions.size()-1]->left);
		CPG->btns.push_back(&poptions[poptions.size()-1]->right);
		playersSoFar++;
	}
	turnLength->activate();
	for (int i=0;i<poptions.size();i++)
		showIcon(-2,i,false);
	SDL_Flip(ekran);
}
void Options::hide()
{
	if (!showed) return;
	PreGameTab::hide();
	for (int i=0; i<CPG->btns.size();i++)
		if (CPG->btns[i]->ID==7)
			CPG->btns.erase(CPG->btns.begin()+i--);
	for (int i=0;i<poptions.size();i++)
		delete poptions[i];
	poptions.clear();
	turnLength->deactivate();
}
MapSel::~MapSel()
{
	SDL_FreeSurface(bg);
	for (int i=0;i<scenImgs.size();i++)
		SDL_FreeSurface(scenImgs[i]);
	for (int i=0;i<scenList.size();i++)
		delete scenList[i];
}
int MapSel::countWL()
{
	int ret=0;
	for (int i=0;i<ourMaps.size();i++)
	{
		if (sizeFilter && ((ourMaps[i].width) != sizeFilter))
			continue;
		else ret++;
	}
	return ret;
}
void MapSel::printMaps(int from, int to, int at, bool abs)
{
	if (true)//
	{
		int help=-1;
		for (int i=0;i<ourMaps.size();i++)
		{
			if (sizeFilter && ((ourMaps[i].width) != sizeFilter))
				continue;
			else help++;
			if (help==from)
			{
				from=i;
				break;
			}
		}
	}
	SDL_Surface * scenin = SDL_CreateRGBSurface(ekran->flags,351,25,ekran->format->BitsPerPixel,ekran->format->Rmask,ekran->format->Gmask,ekran->format->Bmask,ekran->format->Amask);
	SDL_Color nasz;
	for (int i=at;i<to;i++)
	{
		if ((i-at+from) > ourMaps.size()-1)
		{
			SDL_Surface * scenin = SDL_CreateRGBSurface(ekran->flags,351,25,ekran->format->BitsPerPixel,ekran->format->Rmask,ekran->format->Gmask,ekran->format->Bmask,ekran->format->Amask);
			SDL_BlitSurface(bg,&genRect(25,351,22,(i-at)*25+115),scenin,NULL);
			blitAt(scenin,24,121+(i-at)*25);
			//SDL_Flip(ekran);
			CSDL_Ext::update(ekran);
			SDL_FreeSurface(scenin);
			continue;
		}
		if (sizeFilter && ((ourMaps[(i-at)+from].width) != sizeFilter))
		{
			to++;
			at++;
			from++;
			if (((i-at)+from)>ourMaps.size()-1) break;
			else continue;
		}
		if ((i-at+from) == selected)
			nasz=tytulowy;
		else nasz=zwykly;
		//SDL_Rect pier = genRect(25,351,24,126+(i*25));
		SDL_BlitSurface(bg,&genRect(25,351,22,(i-at)*25+115),scenin,NULL);
		int temp=-1;
		std::ostringstream ostr(std::ostringstream::out); ostr << ourMaps[(i-at)+from].playerAmnt << "/" << ourMaps[(i-at)+from].humenPlayers;
		CSDL_Ext::printAt(ostr.str(),6,4,GEOR13,nasz,scenin, 2); 
		std::string temp2;
		switch (ourMaps[(i-at)+from].width)
		{
		case 36:
			temp2="S";
			break;
		case 72:
			temp2="M";
			break;
		case 108:
			temp2="L";
			break;
		case 144:
			temp2="XL";
			break;
		default:
			temp2="C";
			break;
		}
		CSDL_Ext::printAtMiddle(temp2,50,13,GEOR13,nasz,scenin, 2);
		switch (ourMaps[(i-at)+from].version)
		{
		case Eformat::SoD:
			temp=2;
			break;
		case Eformat::WoG:
			temp=3;
			break;
		}
		blitAt(Dtypes->ourImages[temp].bitmap,67,2,scenin);
		if (!(ourMaps[(i-at)+from].name.length()))
			ourMaps[(i-at)+from].name = "Unnamed";
		CSDL_Ext::printAtMiddle(ourMaps[(i-at)+from].name,192,13,GEOR13,nasz,scenin, 2);
		if (ourMaps[(i-at)+from].victoryCondition==EvictoryConditions::winStandard)
			temp=11;
		else temp=ourMaps[(i-at)+from].victoryCondition;
		blitAt(Dvic->ourImages[temp].bitmap,285,2,scenin);
		if (ourMaps[(i-at)+from].lossCondition.typeOfLossCon == ElossCon::lossStandard)
			temp=3;
		else temp=ourMaps[(i-at)+from].lossCondition.typeOfLossCon;
		blitAt(Dloss->ourImages[temp].bitmap,318,2,scenin);

		blitAt(scenin,24,121+(i-at)*25);
		SDL_UpdateRect(ekran,24,121+(i-at)*25,scenin->w,scenin->h);
	}
	SDL_FreeSurface(scenin);
}
int MapSel::whichWL(int nr)
{
	int help=-1;
	for (int i=0;i<ourMaps.size();i++)
	{
		if (sizeFilter && ((ourMaps[i].width) != sizeFilter))
			continue;
		else help++;
		if (help==nr)
		{
			help=i;
			break;
		}
	}
	return help;
}
void MapSel::hide()
{
	if (!showed)return;
	PreGameTab::hide();
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&small));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&medium));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&large));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&xlarge));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&all));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&nrplayer));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&mapsize));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&type));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&name));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&viccon));
	CPG->btns.erase(std::find(CPG->btns.begin(),CPG->btns.end(),&loscon));
	slid->deactivate();
};
void MapSel::show()
{
	if (showed)return;
	PreGameTab::show();
	//blit bg
	blitAt(bg,2,6);
	CSDL_Ext::printAt("Map Sizes",55,60,GEOR13);
	CSDL_Ext::printAt("Select a Scenario to Play",110,25,TNRB16);
	//size buttons
	small.show();
	medium.show();
	large.show();
	xlarge.show();
	all.show();
	CPG->btns.push_back(&small);
	CPG->btns.push_back(&medium);
	CPG->btns.push_back(&large);
	CPG->btns.push_back(&xlarge);
	CPG->btns.push_back(&all);
	//sort by buttons
	nrplayer.show();
	mapsize.show();
	type.show();
	name.show();
	viccon.show();
	loscon.show();
	CPG->btns.push_back(&nrplayer);
	CPG->btns.push_back(&mapsize);
	CPG->btns.push_back(&type);
	CPG->btns.push_back(&name);
	CPG->btns.push_back(&viccon);
	CPG->btns.push_back(&loscon);

	//print scenario list
	printMaps(0,18);

	slid->activate();

	//SDL_Flip(ekran);
	CSDL_Ext::update(ekran);
}
void MapSel::init()
{
	bg = CGI->bitmaph->loadBitmap("SCSELBCK.bmp");
	SDL_SetColorKey(bg,SDL_SRCCOLORKEY,SDL_MapRGB(bg->format,0,255,255));
	small.imgs = CGI->spriteh->giveDef("SCSMBUT.DEF");
	small.fun = NULL;
	small.pos = genRect(small.imgs->ourImages[0].bitmap->h,small.imgs->ourImages[0].bitmap->w,161,52);
	small.ourGroup=NULL;
	medium.imgs = CGI->spriteh->giveDef("SCMDBUT.DEF");
	medium.fun = NULL;
	medium.pos = genRect(medium.imgs->ourImages[0].bitmap->h,medium.imgs->ourImages[0].bitmap->w,208,52);
	medium.ourGroup=NULL;
	large.imgs = CGI->spriteh->giveDef("SCLGBUT.DEF");
	large.fun = NULL;
	large.pos = genRect(large.imgs->ourImages[0].bitmap->h,large.imgs->ourImages[0].bitmap->w,255,52);
	large.ourGroup=NULL;
	xlarge.imgs = CGI->spriteh->giveDef("SCXLBUT.DEF");
	xlarge.fun = NULL;
	xlarge.pos = genRect(xlarge.imgs->ourImages[0].bitmap->h,xlarge.imgs->ourImages[0].bitmap->w,302,52);
	xlarge.ourGroup=NULL;
	all.imgs = CGI->spriteh->giveDef("SCALBUT.DEF");
	all.fun = NULL;
	all.pos = genRect(all.imgs->ourImages[0].bitmap->h,all.imgs->ourImages[0].bitmap->w,349,52);
	all.ourGroup=NULL;
	all.selectable=xlarge.selectable=large.selectable=medium.selectable=small.selectable=false;
	small.what=medium.what=large.what=xlarge.what=all.what=&sizeFilter;
	small.key=36;medium.key=72;large.key=108;xlarge.key=144;all.key=0;
//Button<> nrplayer, mapsize, type, name, viccon, loscon;
	nrplayer.imgs = CGI->spriteh->giveDef("SCBUTT1.DEF");
	nrplayer.fun = NULL;
	nrplayer.pos = genRect(nrplayer.imgs->ourImages[0].bitmap->h,nrplayer.imgs->ourImages[0].bitmap->w,26,92);
	nrplayer.key=ESortBy::playerAm;

	mapsize.imgs = CGI->spriteh->giveDef("SCBUTT2.DEF");
	mapsize.fun = NULL;
	mapsize.pos = genRect(mapsize.imgs->ourImages[0].bitmap->h,mapsize.imgs->ourImages[0].bitmap->w,58,92);
	mapsize.key=ESortBy::size;

	type.imgs = CGI->spriteh->giveDef("SCBUTCP.DEF");
	type.fun = NULL;
	type.pos = genRect(type.imgs->ourImages[0].bitmap->h,type.imgs->ourImages[0].bitmap->w,91,92);
	type.key=ESortBy::format;

	name.imgs = CGI->spriteh->giveDef("SCBUTT3.DEF");
	name.fun = NULL;
	name.pos = genRect(name.imgs->ourImages[0].bitmap->h,name.imgs->ourImages[0].bitmap->w,124,92);
	name.key=ESortBy::name;

	viccon.imgs = CGI->spriteh->giveDef("SCBUTT4.DEF");
	viccon.fun = NULL;
	viccon.pos = genRect(viccon.imgs->ourImages[0].bitmap->h,viccon.imgs->ourImages[0].bitmap->w,309,92);
	viccon.key=ESortBy::viccon;

	loscon.imgs = CGI->spriteh->giveDef("SCBUTT5.DEF");
	loscon.fun = NULL;
	loscon.pos = genRect(loscon.imgs->ourImages[0].bitmap->h,loscon.imgs->ourImages[0].bitmap->w,342,92);
	loscon.key=ESortBy::loscon;

	nrplayer.poin=mapsize.poin=type.poin=name.poin=viccon.poin=loscon.poin=(int*)(&sortBy);
	nrplayer.fun=mapsize.fun=type.fun=name.fun=viccon.fun=loscon.fun=&CPreGame::sortMaps;

	Dtypes = CGI->spriteh->giveDef("SCSELC.DEF");
	Dvic = CGI->spriteh->giveDef("SCNRVICT.DEF");
	Dloss = CGI->spriteh->giveDef("SCNRLOSS.DEF");
	//Dsizes = CPG->slh->giveDef("SCNRMPSZ.DEF");
	Dsizes = CGI->spriteh->giveDef("SCNRMPSZ.DEF");
	//get map files names
	std::vector<std::string> pliczkiTemp;
	fs::path tie( (fs::initial_path<fs::path>())/"\maps" );
	fs::directory_iterator end_iter;
	for ( fs::directory_iterator dir (tie); dir!=end_iter; ++dir )
	{
		if (fs::is_regular(dir->status()));
		{
			if (boost::ends_with(dir->path().leaf(),std::string(".h3m")))
				pliczkiTemp.push_back("Maps/"+(dir->path().leaf()));
		}
	}
	for (int i=0; i<pliczkiTemp.size();i++)
	{
		gzFile tempf = gzopen(pliczkiTemp[i].c_str(),"rb");
		std::string sss;
		int iii=0;
		while(++iii)
		{
			if (iii>3300) break;
			int z = gzgetc (tempf);
			if (z>=0) 
			{
				sss+=unsigned char(z);
			}
			else break;
		}
		gzclose(tempf);
		if (sss[0]<28) continue; //zly format
		if (!sss[4]) continue; //nie ma graczy? mapa niegrywalna //już to kiedyś komentowałem- - to bzdura //tu calkiem pasuje...
		unsigned char* file2 = new unsigned char[sss.length()];
		for (int j=0;j<sss.length();j++)
			file2[j]=sss[j];
		ourMaps.push_back(CMapInfo(pliczkiTemp[i],file2));
		delete file2;
	}
	std::sort(ourMaps.begin(),ourMaps.end(),mapSorter(ESortBy::name));
	slid = new Slider<>(375,92,480,ourMaps.size(),18,true);
	slid->fun = &CPreGame::printMapsFrom;
}
void MapSel::moveByOne(bool up)
{
	int help=selected;
	if (up) selected--;
	else selected ++;
	for (int i=selected;i<ourMaps.size() && i>=0;)
	{
		help=i;
		if (!(sizeFilter && ((ourMaps[i].width) != sizeFilter)))		
			break;
		if (up)
		{
			i--;
		}
		else
		{
			i++;
			if (i<0) break;
		}
	}
	select(help);
	slid->updateSlid();
}
void MapSel::select(int which)
{
	selected = which;
	printMaps(slid->whereAreWe,18,0,true);
	printSelectedInfo();

	CPG->ret.playerInfos.clear();
	for (int i=0;i<8;i++)
	{
		StartInfo::PlayerSettings temp;
		if (ourMaps[which].players[i].canHumanPlay)
		{
			temp.name="Player";
		}
		else if (ourMaps[which].players[i].canComputerPlay)
		{
			temp.name="AI";
		}
		else
			continue;
		int pom=0;
		for (int j=0;j<F_NUMBER; j++)
		{
			if(((int)pow(2.0f,j))&ourMaps[which].players[i].allowedFactions)
			{
				temp.castle=j;
				pom++;
			}
			if (pom>1) temp.castle=-1;
			else if (!pom) temp.castle=-2;
		}
		temp.bonus=(Ebonus)-1;
		if (	(ourMaps[which].players[i].heroesNames.size()>1)      ||
			(((ourMaps[which].players[i].heroesNames.size()>1)==1) && ourMaps[which].players[i].generateHeroAtMainTown==false)    )
			temp.hero=-2;
		else if (ourMaps[which].players[i].heroesNames.size()==1)
		{
			temp.hero=ourMaps[which].players[i].heroesNames[0].heroID;
		}
		else
			temp.hero=-1;
		CPG->ret.playerInfos.push_back(temp);
	}

	for (int i=0;i<PLAYER_LIMIT;i++)
	{
		if (!(ourMaps[selected].players[i].canComputerPlay || ourMaps[selected].players[i].canComputerPlay))
			continue;
		StartInfo::PlayerSettings pset;
		pset.color=(Ecolor)i;
		pset.bonus=Ebonus::brandom;
		pset.castle=-2;
		for (int j=0;j<F_NUMBER;j++)
		{
			if (((int)pow((double)2,j))&ourMaps[selected].players[i].allowedFactions)
			{
				if (pset.castle>=0)
					pset.castle=-1;
				if (pset.castle==-2)
					pset.castle=j;
			}
		}
		if (ourMaps[selected].players[i].generateHeroAtMainTown || ourMaps[selected].players[i].generateHero)
			pset.hero=-1;
		else if (ourMaps[selected].players[i].mainHeroName.length())
		{
			pset.heroName=ourMaps[selected].players[i].mainHeroName;
			pset.heroPortrait=ourMaps[selected].players[i].mainHeroPortrait;
		}
		else
			pset.hero=-2;
		CPG->ret.playerInfos.push_back(pset);
	}

}
MapSel::MapSel():selected(0),sizeFilter(0)
{
}
void MapSel::printSelectedInfo()
{
	SDL_BlitSurface(CPG->ourScenSel->scenInf,&genRect(399,337,17,23),ekran,&genRect(399,337,412,29));
	SDL_BlitSurface(CPG->ourScenSel->scenInf,&genRect(50,91,18,447),ekran,&genRect(50,91,413,453));
	SDL_BlitSurface(CPG->ourScenSel->bScens.imgs->ourImages[0].bitmap,NULL,ekran,&CPG->ourScenSel->bScens.pos);
	SDL_BlitSurface(CPG->ourScenSel->bOptions.imgs->ourImages[0].bitmap,NULL,ekran,&CPG->ourScenSel->bOptions.pos);
	SDL_BlitSurface(CPG->ourScenSel->bRandom.imgs->ourImages[0].bitmap,NULL,ekran,&CPG->ourScenSel->bRandom.pos);
	//blit texts
	CSDL_Ext::printAt(CPG->preth->singleScenarioName,420,25,GEOR13);
	CSDL_Ext::printAt("Scenario Description:",420,135,GEOR13);
	CSDL_Ext::printAt("Victory Condition:",420,285,GEOR13);
	CSDL_Ext::printAt("Loss Condition:",420,340,GEOR13);
	CSDL_Ext::printAt("Allies:",420,406,GEOR13,zwykly);
	CSDL_Ext::printAt("Enemies:",585,406,GEOR13,zwykly);

	int temp = ourMaps[selected].victoryCondition+1;
	if (temp>20) temp=0;
	std::string sss = CPG->preth->victoryConditions[temp];
	if (temp && ourMaps[selected].vicConDetails->allowNormalVictory) sss+= "/" + CPG->preth->victoryConditions[0];
	CSDL_Ext::printAt(sss,452,310,GEOR13,zwykly);


	temp = ourMaps[selected].lossCondition.typeOfLossCon+1;
	if (temp>20) temp=0;
	sss = CPG->preth->lossCondtions[temp];
	CSDL_Ext::printAt(sss,452,370,GEOR13,zwykly);

	//blit descrption
	std::vector<std::string> desc = *CMessage::breakText(ourMaps[selected].description,50);
	for (int i=0;i<desc.size();i++)
		CSDL_Ext::printAt(desc[i],417,152+i*13,GEOR13,zwykly);

	if ((selected < 0) || (selected >= ourMaps.size()))
		return; 
	if (ourMaps[selected].name.length())
		CSDL_Ext::printAt(ourMaps[selected].name,420,41,GEORXX);
	else CSDL_Ext::printAt("Unnamed",420,41,GEORXX);
	std::string diff;
	switch (ourMaps[selected].difficulty)
	{
	case 0:
		diff=gdiff(CPG->preth->singleEasy);
		break;
	case 1:
		diff=gdiff(CPG->preth->singleNormal);
		break;
	case 2:
		diff=gdiff(CPG->preth->singleHard);
		break;
	case 3:
		diff=gdiff(CPG->preth->singleExpert);
		break;
	case 4:
		diff=gdiff(CPG->preth->singleImpossible);
		break;
	}
	temp=-1;
	switch (ourMaps[selected].width)
	{
	case 36:
		temp=0;
		break;
	case 72:
		temp=1;
		break;
	case 108:
		temp=2;
		break;
	case 144:
		temp=3;
		break;
	default:
		temp=4;
		break;
	}
	blitAt(Dsizes->ourImages[temp].bitmap,714,28);
	temp=ourMaps[selected].victoryCondition;
	if (temp>12) temp=11;
	blitAt(Dvic->ourImages[temp].bitmap,420,308); //v
	temp=ourMaps[selected].lossCondition.typeOfLossCon;
	if (temp>12) temp=3;
	blitAt(Dloss->ourImages[temp].bitmap,420,366); //l

	CSDL_Ext::printAtMiddle(diff,458,477,GEOR13,zwykly);
	//SDL_Flip(ekran);
	CSDL_Ext::update(ekran);
}
std::string MapSel::gdiff(std::string ss)
{
	std::string ret;
	for (int i=2;i<ss.length();i++)
	{
		if (ss[i]==' ')
			break;
		ret+=ss[i];
	}
	return ret;
}
void CPreGame::printMapsFrom(int from)
{
	ourScenSel->mapsel.printMaps(from);
}
void CPreGame::showScenList()
{
	if (currentTab!=&ourScenSel->mapsel)
	{
		ourScenSel->listShowed=true;
		ourScenSel->mapsel.show();
	}
	else currentTab->hide();
}
CPreGame::CPreGame()
{
	currentTab=NULL;
	run=true;
	timeHandler tmh;tmh.getDif();
	tytulowy.r=229;tytulowy.g=215;tytulowy.b=123;tytulowy.unused=0;
	zwykly.r=255;zwykly.g=255;zwykly.b=255;zwykly.unused=0; //gbr
	tlo.r=66;tlo.g=44;tlo.b=24;tlo.unused=0;
	preth = new CPreGameTextHandler;
	preth->loadTexts();
	THC std::cout<<"\tCPreGame: loading txts: "<<tmh.getDif()<<std::endl;
	currentMessage=NULL;
	behindCurMes=NULL;
	initMainMenu();
	THC std::cout<<"\tCPreGame: main menu initialization: "<<tmh.getDif()<<std::endl;
	initNewMenu();
	THC std::cout<<"\tCPreGame: newgame menu initialization: "<<tmh.getDif()<<std::endl;
	initScenSel();
	THC std::cout<<"\tCPreGame: scenario choice initialization: "<<tmh.getDif()<<std::endl;
	initOptions();
	THC std::cout<<"\tCPreGame: scenario options initialization: "<<tmh.getDif()<<std::endl;
	showMainMenu();
	THC std::cout<<"\tCPreGame: displaying main menu: "<<tmh.getDif()<<std::endl;
	CPG=this;
}
void CPreGame::initOptions()
{
	ourOptions = new Options();
	ourOptions->init();
}
void CPreGame::initScenSel()
{
	ourScenSel = new ScenSel();
	ourScenSel->listShowed=false;
	if (rand()%2) ourScenSel->background=CGI->bitmaph->loadBitmap("ZPIC1000.bmp");
	else ourScenSel->background=CGI->bitmaph->loadBitmap("ZPIC1001.bmp");

	ourScenSel->pressed=NULL;

	ourScenSel->scenInf=CGI->bitmaph->loadBitmap("GSELPOP1.bmp");//SDL_LoadBMP("h3bitmap.lod\\GSELPOP1.bmp");
	ourScenSel->randMap=CGI->bitmaph->loadBitmap("RANMAPBK.bmp");
	ourScenSel->options=CGI->bitmaph->loadBitmap("ADVOPTBK.bmp");
	SDL_SetColorKey(ourScenSel->scenInf,SDL_SRCCOLORKEY,SDL_MapRGB(ourScenSel->scenInf->format,0,255,255));
	//SDL_SetColorKey(ourScenSel->scenList,SDL_SRCCOLORKEY,SDL_MapRGB(ourScenSel->scenList->format,0,255,255));
	SDL_SetColorKey(ourScenSel->randMap,SDL_SRCCOLORKEY,SDL_MapRGB(ourScenSel->randMap->format,0,255,255));
	SDL_SetColorKey(ourScenSel->options,SDL_SRCCOLORKEY,SDL_MapRGB(ourScenSel->options->format,0,255,255));

	ourScenSel->difficulty = new CPoinGroup<>();
	ourScenSel->difficulty->type=1;
	ourScenSel->selectedDiff=-77;
	ourScenSel->difficulty->gdzie = &ourScenSel->selectedDiff;
	ourScenSel->bEasy = IntSelBut<>(genRect(0,0,506,456),NULL,CGI->spriteh->giveDef("GSPBUT3.DEF"),true,ourScenSel->difficulty,1);
	ourScenSel->bNormal = IntSelBut<>(genRect(0,0,538,456),NULL,CGI->spriteh->giveDef("GSPBUT4.DEF"),true,ourScenSel->difficulty,2);
	ourScenSel->bHard = IntSelBut<>(genRect(0,0,570,456),NULL,CGI->spriteh->giveDef("GSPBUT5.DEF"),true,ourScenSel->difficulty,3);
	ourScenSel->bExpert = IntSelBut<>(genRect(0,0,602,456),NULL,CGI->spriteh->giveDef("GSPBUT6.DEF"),true,ourScenSel->difficulty,4);
	ourScenSel->bImpossible = IntSelBut<>(genRect(0,0,634,456),NULL,CGI->spriteh->giveDef("GSPBUT7.DEF"),true,ourScenSel->difficulty,5);

	ourScenSel->bBack = Button<>(genRect(0,0,584,535),&CPreGame::showNewMenu,CGI->spriteh->giveDef("SCNRBACK.DEF"));
	ourScenSel->bBegin = Button<>(genRect(0,0,414,535),&CPreGame::begin,CGI->spriteh->giveDef("SCNRBEG.DEF"));

	ourScenSel->bScens = Button<>(genRect(0,0,414,81),&CPreGame::showScenList,CGI->spriteh->giveDef("GSPBUTT.DEF"));
	for (int i=0; i<ourScenSel->bScens.imgs->ourImages.size(); i++)
		CSDL_Ext::printAt("Show Available Scenarios",25+i,2+i,GEOR13,zwykly,ourScenSel->bScens.imgs->ourImages[i].bitmap);
	ourScenSel->bRandom = Button<>(genRect(0,0,414,105),&CPreGame::showScenList,CGI->spriteh->giveDef("GSPBUTT.DEF"));
	for (int i=0; i<ourScenSel->bRandom.imgs->ourImages.size(); i++)
		CSDL_Ext::printAt("Random Map",25+i,2+i,GEOR13,zwykly,ourScenSel->bRandom.imgs->ourImages[i].bitmap);
	ourScenSel->bOptions = Button<>(genRect(0,0,414,509),&CPreGame::showOptions,CGI->spriteh->giveDef("GSPBUTT.DEF"));
	for (int i=0; i<ourScenSel->bOptions.imgs->ourImages.size(); i++)
		CSDL_Ext::printAt("Show Advanced Options",25+i,2+i,GEOR13,zwykly,ourScenSel->bOptions.imgs->ourImages[i].bitmap);

	CPG=this;
	ourScenSel->mapsel.init();
}

void CPreGame::showScenSel()
{
	state=EState::ScenarioList;
	SDL_BlitSurface(ourScenSel->background,NULL,ekran,NULL);
	SDL_BlitSurface(ourScenSel->scenInf,NULL,ekran,&genRect(ourScenSel->scenInf->h,ourScenSel->scenInf->w,395,6));
	CSDL_Ext::printAt("Map Diff:",427,438,GEOR13);
	CSDL_Ext::printAt("Player Difficulty:",527,438,GEOR13);
	CSDL_Ext::printAt("Rating:",685,438,GEOR13);
	//blit buttons
	SDL_BlitSurface(ourScenSel->bEasy.imgs->ourImages[0].bitmap,NULL,ekran,&ourScenSel->bEasy.pos);
	SDL_BlitSurface(ourScenSel->bNormal.imgs->ourImages[0].bitmap,NULL,ekran,&ourScenSel->bNormal.pos);
	SDL_BlitSurface(ourScenSel->bHard.imgs->ourImages[0].bitmap,NULL,ekran,&ourScenSel->bHard.pos);
	SDL_BlitSurface(ourScenSel->bExpert.imgs->ourImages[0].bitmap,NULL,ekran,&ourScenSel->bExpert.pos);
	SDL_BlitSurface(ourScenSel->bImpossible.imgs->ourImages[0].bitmap,NULL,ekran,&ourScenSel->bImpossible.pos);
	SDL_BlitSurface(ourScenSel->bBegin.imgs->ourImages[0].bitmap,NULL,ekran,&ourScenSel->bBegin.pos);
	SDL_BlitSurface(ourScenSel->bBack.imgs->ourImages[0].bitmap,NULL,ekran,&ourScenSel->bBack.pos);
	//blitAt(ourScenSel->bScens.imgs->ourImages[0].bitmap,ourScenSel->bScens.pos.x,ourScenSel->bScens.pos.y);
	//blitAt(ourScenSel->bRandom.imgs->ourImages[0].bitmap,414,105);
	//blitAt(ourScenSel->bOptions.imgs->ourImages[0].bitmap,414,509);
	//blitAt(ourScenSel->bBegin.imgs->ourImages[0].bitmap,414,535);
	//blitAt(ourScenSel->bBack.imgs->ourImages[0].bitmap,584,535);

	//add buttons info
	btns.push_back(&ourScenSel->bEasy);
	btns.push_back(&ourScenSel->bNormal);
	btns.push_back(&ourScenSel->bHard);
	btns.push_back(&ourScenSel->bExpert);
	btns.push_back(&ourScenSel->bImpossible);
	btns.push_back(&ourScenSel->bScens);
	btns.push_back(&ourScenSel->bRandom);
	btns.push_back(&ourScenSel->bOptions);
	btns.push_back(&ourScenSel->bBegin);
	btns.push_back(&ourScenSel->bBack);

	ourScenSel->selectedDiff=1;
	ourScenSel->bNormal.select(true);

	for (int i=0;i<btns.size();i++)
	{
		btns[i]->pos.w=btns[i]->imgs->ourImages[0].bitmap->w;
		btns[i]->pos.h=btns[i]->imgs->ourImages[0].bitmap->h;
		btns[i]->ID=10;
	}
	handleOther = &CPreGame::scenHandleEv;

	ourScenSel->mapsel.printSelectedInfo();
	//SDL_Flip(ekran);
	CSDL_Ext::update(ekran);

}
void CPreGame::showOptions()
{
	ourOptions->show();
}
void CPreGame::initNewMenu()
{
	ourNewMenu = new menuItems();
	ourNewMenu->bgAd = CGI->bitmaph->loadBitmap("ZNEWGAM.bmp");
	ourNewMenu->background = CGI->bitmaph->loadBitmap("ZPIC1005.bmp");
	blitAt(ourNewMenu->bgAd,114,312,ourNewMenu->background);
	slh = new CSemiLodHandler();
	slh->openLod("H3sprite.lod");
	 //loading menu buttons
	ourNewMenu->newGame = CGI->spriteh->giveDef("ZTSINGL.DEF");
	ourNewMenu->loadGame = CGI->spriteh->giveDef("ZTMULTI.DEF");
	ourNewMenu->highScores = CGI->spriteh->giveDef("ZTCAMPN.DEF");
	ourNewMenu->credits = CGI->spriteh->giveDef("ZTTUTOR.DEF");
	ourNewMenu->quit = CGI->spriteh->giveDef("ZTBACK.DEF");
	ok = CGI->spriteh->giveDef("IOKAY.DEF");
	cancel = CGI->spriteh->giveDef("ICANCEL.DEF");
	// single scenario
	ourNewMenu->lNewGame.h=ourNewMenu->newGame->ourImages[0].bitmap->h;
	ourNewMenu->lNewGame.w=ourNewMenu->newGame->ourImages[0].bitmap->w;
	ourNewMenu->lNewGame.x=545;
	ourNewMenu->lNewGame.y=4;
	ourNewMenu->fNewGame=&CPreGame::showScenSel;
	//multiplayer
	ourNewMenu->lLoadGame.h=ourNewMenu->loadGame->ourImages[0].bitmap->h;
	ourNewMenu->lLoadGame.w=ourNewMenu->loadGame->ourImages[0].bitmap->w;
	ourNewMenu->lLoadGame.x=568;
	ourNewMenu->lLoadGame.y=120;
	//campaign
	ourNewMenu->lHighScores.h=ourNewMenu->highScores->ourImages[0].bitmap->h;
	ourNewMenu->lHighScores.w=ourNewMenu->highScores->ourImages[0].bitmap->w;
	ourNewMenu->lHighScores.x=541;
	ourNewMenu->lHighScores.y=233;
	//tutorial
	ourNewMenu->lCredits.h=ourNewMenu->credits->ourImages[0].bitmap->h;
	ourNewMenu->lCredits.w=ourNewMenu->credits->ourImages[0].bitmap->w;
	ourNewMenu->lCredits.x=545;
	ourNewMenu->lCredits.y=358;
	//back
	ourNewMenu->lQuit.h=ourNewMenu->quit->ourImages[0].bitmap->h;
	ourNewMenu->lQuit.w=ourNewMenu->quit->ourImages[0].bitmap->w;
	ourNewMenu->lQuit.x=582;
	ourNewMenu->lQuit.y=464;
	ourNewMenu->fQuit=&CPreGame::showMainMenu;
	
	ourNewMenu->highlighted=0;
}
void CPreGame::showNewMenu()
{
	btns.clear();
	interested.clear();
	handleOther=NULL;
	state = EState::newGame;
	SDL_BlitSurface(ourNewMenu->background,NULL,ekran,NULL);
	SDL_BlitSurface(ourNewMenu->newGame->ourImages[0].bitmap,NULL,ekran,&ourNewMenu->lNewGame);
	SDL_BlitSurface(ourNewMenu->loadGame->ourImages[0].bitmap,NULL,ekran,&ourNewMenu->lLoadGame);
	SDL_BlitSurface(ourNewMenu->highScores->ourImages[0].bitmap,NULL,ekran,&ourNewMenu->lHighScores);
	SDL_BlitSurface(ourNewMenu->credits->ourImages[0].bitmap,NULL,ekran,&ourNewMenu->lCredits);
	SDL_BlitSurface(ourNewMenu->quit->ourImages[0].bitmap,NULL,ekran,&ourNewMenu->lQuit);
	//SDL_Flip(ekran);
	CSDL_Ext::update(ekran);
}
void CPreGame::initMainMenu()
{
	ourMainMenu = new menuItems();
	ourMainMenu->background = CGI->bitmaph->loadBitmap("ZPIC1005.bmp"); //SDL_LoadBMP("h3bitmap.lod\\ZPIC1005.bmp");
	CSemiLodHandler * slh = new CSemiLodHandler();
	slh->openLod("H3sprite.lod");
	 //loading menu buttons
	ourMainMenu->newGame = CGI->spriteh->giveDef("ZMENUNG.DEF");
	ourMainMenu->loadGame = CGI->spriteh->giveDef("ZMENULG.DEF");
	ourMainMenu->highScores = CGI->spriteh->giveDef("ZMENUHS.DEF");
	ourMainMenu->credits = CGI->spriteh->giveDef("ZMENUCR.DEF");
	ourMainMenu->quit = CGI->spriteh->giveDef("ZMENUQT.DEF");
	ok = CGI->spriteh->giveDef("IOKAY.DEF");
	cancel = CGI->spriteh->giveDef("ICANCEL.DEF");
	// new game button location
	ourMainMenu->lNewGame.h=ourMainMenu->newGame->ourImages[0].bitmap->h;
	ourMainMenu->lNewGame.w=ourMainMenu->newGame->ourImages[0].bitmap->w;
	ourMainMenu->lNewGame.x=540;
	ourMainMenu->lNewGame.y=10;
	ourMainMenu->fNewGame=&CPreGame::showNewMenu;
	//load game location
	ourMainMenu->lLoadGame.h=ourMainMenu->loadGame->ourImages[0].bitmap->h;
	ourMainMenu->lLoadGame.w=ourMainMenu->loadGame->ourImages[0].bitmap->w;
	ourMainMenu->lLoadGame.x=532;
	ourMainMenu->lLoadGame.y=132;
	//high scores
	ourMainMenu->lHighScores.h=ourMainMenu->highScores->ourImages[0].bitmap->h;
	ourMainMenu->lHighScores.w=ourMainMenu->highScores->ourImages[0].bitmap->w;
	ourMainMenu->lHighScores.x=524;
	ourMainMenu->lHighScores.y=251;
	//credits
	ourMainMenu->lCredits.h=ourMainMenu->credits->ourImages[0].bitmap->h;
	ourMainMenu->lCredits.w=ourMainMenu->credits->ourImages[0].bitmap->w;
	ourMainMenu->lCredits.x=557;
	ourMainMenu->lCredits.y=359;
	//quit
	ourMainMenu->lQuit.h=ourMainMenu->quit->ourImages[0].bitmap->h;
	ourMainMenu->lQuit.w=ourMainMenu->quit->ourImages[0].bitmap->w;
	ourMainMenu->lQuit.x=586;
	ourMainMenu->lQuit.y=468;
	ourMainMenu->fQuit=&CPreGame::quitAskBox;
	
	ourMainMenu->highlighted=0;
	handledLods.push_back(slh);
	delete slh;
}
void CPreGame::showMainMenu()
{
	state = EState::mainMenu;
	SDL_BlitSurface(ourMainMenu->background,NULL,ekran,NULL);
	SDL_BlitSurface(ourMainMenu->newGame->ourImages[0].bitmap,NULL,ekran,&ourMainMenu->lNewGame);
	SDL_BlitSurface(ourMainMenu->loadGame->ourImages[0].bitmap,NULL,ekran,&ourMainMenu->lLoadGame);
	SDL_BlitSurface(ourMainMenu->highScores->ourImages[0].bitmap,NULL,ekran,&ourMainMenu->lHighScores);
	SDL_BlitSurface(ourMainMenu->credits->ourImages[0].bitmap,NULL,ekran,&ourMainMenu->lCredits);
	SDL_BlitSurface(ourMainMenu->quit->ourImages[0].bitmap,NULL,ekran,&ourMainMenu->lQuit);
	//SDL_Flip(ekran);
	CSDL_Ext::update(ekran);
}
void CPreGame::highlightButton(int which, int on)
{

	menuItems * current = currentItems();
	switch (which)
	{
	case 1:
		{
			SDL_BlitSurface(current->newGame->ourImages[on].bitmap,NULL,ekran,&current->lNewGame);
			break;
		}
	case 2:
		{
			SDL_BlitSurface(current->loadGame->ourImages[on].bitmap,NULL,ekran,&current->lLoadGame);
			break;
		}
	case 3:
		{
			SDL_BlitSurface(current->highScores->ourImages[on].bitmap,NULL,ekran,&current->lHighScores);
			break;
		}
	case 4:
		{
			SDL_BlitSurface(current->credits->ourImages[on].bitmap,NULL,ekran,&current->lCredits);
			break;
		}
	case 5:
		{
			SDL_BlitSurface(current->quit->ourImages[on].bitmap,NULL,ekran,&current->lQuit);
			break;
		}
	}
	//SDL_Flip(ekran);
	CSDL_Ext::update(ekran);
}
void CPreGame::showCenBox (std::string data)
{
	CMessage * cmh = new CMessage();
	SDL_Surface * infoBox = cmh->genMessage(preth->getTitle(data), preth->getDescr(data));
	behindCurMes = SDL_CreateRGBSurface(ekran->flags,infoBox->w,infoBox->h,ekran->format->BitsPerPixel,ekran->format->Rmask,ekran->format->Gmask,ekran->format->Bmask,ekran->format->Amask);
	SDL_Rect pos = genRect(infoBox->h,infoBox->w,
		(ekran->w/2)-(infoBox->w/2),(ekran->h/2)-(infoBox->h/2));
	SDL_BlitSurface(ekran,&pos,behindCurMes,NULL);
	SDL_BlitSurface(infoBox,NULL,ekran,&pos);
	SDL_UpdateRect(ekran,pos.x,pos.y,pos.w,pos.h);
	SDL_FreeSurface(infoBox);
	currentMessage = new SDL_Rect(pos);
	delete cmh;
}
void CPreGame::showAskBox (std::string data, void(*f1)(),void(*f2)())
{
	CMessage * cmh = new CMessage();
	std::vector<CDefHandler*> * przyciski = new std::vector<CDefHandler*>(0);
	std::vector<SDL_Rect> * btnspos= new std::vector<SDL_Rect>(0);
	przyciski->push_back(ok);
	przyciski->push_back(cancel);
	SDL_Surface * infoBox = cmh->genMessage(preth->getTitle(data), preth->getDescr(data), EWindowType::yesOrNO, przyciski, btnspos);
	behindCurMes = SDL_CreateRGBSurface(ekran->flags,infoBox->w,infoBox->h,ekran->format->BitsPerPixel,ekran->format->Rmask,ekran->format->Gmask,ekran->format->Bmask,ekran->format->Amask);
	SDL_Rect pos = genRect(infoBox->h,infoBox->w,
		(ekran->w/2)-(infoBox->w/2),(ekran->h/2)-(infoBox->h/2));
	SDL_BlitSurface(ekran,&pos,behindCurMes,NULL);
	SDL_BlitSurface(infoBox,NULL,ekran,&pos);
	SDL_UpdateRect(ekran,pos.x,pos.y,pos.w,pos.h);
	SDL_FreeSurface(infoBox);
	currentMessage = new SDL_Rect(pos);
	(*btnspos)[0].x+=pos.x;
	(*btnspos)[0].y+=pos.y;
	(*btnspos)[1].x+=pos.x;
	(*btnspos)[1].y+=pos.y;
	btns.push_back(new Button<>((*btnspos)[0],&CPreGame::quit,ok,false, NULL,2));
	btns.push_back(new Button<>((*btnspos)[1],(&CPreGame::hideBox),cancel,false, NULL,2));
	delete cmh;
	delete przyciski;
	delete btnspos;

}
void CPreGame::hideBox ()
{
	SDL_BlitSurface(behindCurMes,NULL,ekran,currentMessage);
	SDL_UpdateRect
		(ekran,currentMessage->x,currentMessage->y,currentMessage->w,currentMessage->h);
	for (int i=0;i<btns.size();i++)
	{
		if (btns[i]->ID==2)
		{
			delete btns[i];
			btns.erase(btns.begin()+i);
			i--;
		}
	}
	SDL_FreeSurface(behindCurMes);
	delete currentMessage;
	currentMessage = NULL;
	behindCurMes=NULL;
}
CPreGame::menuItems * CPreGame::currentItems()
{
	switch (state)
	{
	case EState::mainMenu:
		return ourMainMenu;
	case EState::newGame:
		return ourNewMenu;
	default:
		return NULL;
	}
}

void CPreGame::scenHandleEv(SDL_Event& sEvent)
{
	if ((sEvent.type==SDL_MOUSEBUTTONDOWN) && (sEvent.button.button == SDL_BUTTON_LEFT))
	{
		for (int i=0;i<btns.size(); i++)
		{
			if (isItIn(&btns[i]->pos,sEvent.motion.x,sEvent.motion.y))
			{
				btns[i]->press(true);
				ourScenSel->pressed=(Button<>*)btns[i];
			}
		}
		if (ourScenSel->listShowed && (sEvent.button.y>121) &&(sEvent.button.y<570) 
									&& (sEvent.button.x>55) && (sEvent.button.x<372))
		{
			int py = ((sEvent.button.y-121)/25)+ourScenSel->mapsel.slid->whereAreWe;
			ourScenSel->mapsel.select(ourScenSel->mapsel.whichWL(py));
		}

	}
	else if ((sEvent.type==SDL_MOUSEBUTTONUP) && (sEvent.button.button == SDL_BUTTON_LEFT))
	{
		Button<> * prnr=ourScenSel->pressed;
		if (ourScenSel->pressed && ourScenSel->pressed->state==1)
		{
			ourScenSel->pressed->press(false);
			ourScenSel->pressed=NULL;
		}
		for (int i=0;i<btns.size(); i++)
		{
			if (isItIn(&btns[i]->pos,sEvent.motion.x,sEvent.motion.y))
			{
				if (btns[i]->selectable)
					btns[i]->select(true);
				if (btns[i]->type==1 && ((Button<>*)btns[i])->fun)
					(this->*(((Button<>*)btns[i])->fun))();
				int zz = btns.size();
				if (i>=zz) 
					break;
				if  (btns[i]==prnr && btns[i]->type==2)
				{
					((IntBut<> *)(btns[i]))->set();
					ourScenSel->mapsel.slid->whereAreWe=0;
					ourScenSel->mapsel.slid->updateSlid();
					ourScenSel->mapsel.slid->positionsAmnt=ourScenSel->mapsel.countWL();
					ourScenSel->mapsel.printMaps(0);
				}
			}
		}
	}
}
StartInfo CPreGame::runLoop()
{
	SDL_Event sEvent;
	while(run)
	{
		try
		{
			if(SDL_PollEvent(&sEvent))  //wait for event...
			{
				menuItems * current = currentItems();
				if(sEvent.type==SDL_QUIT) 
					return ret;
				for (int i=0;i<interested.size();i++)
					interested[i]->handleIt(sEvent);
				if (!current)
				{
					(this->*handleOther)(sEvent);
				}
				else if (sEvent.type==SDL_KEYDOWN)
				{
					if (sEvent.key.keysym.sym==SDLK_q)
						{
							return ret;
							break;
						}
					/*if (state==EState::newGame)
					{
						switch (sEvent.key.keysym.sym)
						{
						case SDLK_LEFT:
							{
								if(currentItems()->lNewGame.x>0)
									currentItems()->lNewGame.x--;
								break;
							}
						case (SDLK_RIGHT):
							{
									currentItems()->lNewGame.x++;
								break;
							}
						case (SDLK_UP):
							{
								if(currentItems()->lNewGame.y>0)
									currentItems()->lNewGame.y--;
								break;
							}
						case (SDLK_DOWN):
							{
									currentItems()->lNewGame.y++;
								break;
							}
						}
						showNewMenu();
					}*/
				}
				else if (sEvent.type==SDL_MOUSEMOTION)
				{
					if (currentMessage) continue;
					if (current->highlighted)
					{
						switch (current->highlighted)
						{
							case 1:
								{
									if(isItIn(&current->lNewGame,sEvent.motion.x,sEvent.motion.y))
										continue;
									else
									{
										current->highlighted=0;
										highlightButton(1,0);
									}
									break;
								}
							case 2:
								{
									if(isItIn(&current->lLoadGame,sEvent.motion.x,sEvent.motion.y))
										continue;
									else
									{
										current->highlighted=0;
										highlightButton(2,0);
									}
									break;
								}
							case 3:
								{
									if(isItIn(&current->lHighScores,sEvent.motion.x,sEvent.motion.y))
										continue;
									else
									{
										current->highlighted=0;
										highlightButton(3,0);
									}
									break;
								}
							case 4:
								{
									if(isItIn(&current->lCredits,sEvent.motion.x,sEvent.motion.y))
										continue;
									else
									{
										current->highlighted=0;
										highlightButton(4,0);
									}
									break;
								}
							case 5:
								{
									if(isItIn(&current->lQuit,sEvent.motion.x,sEvent.motion.y))
										continue;
									else
									{
										current->highlighted=0;
										highlightButton(5,0);
									}
									break;
								}
						} //switch (current->highlighted)
					} // if (current->highlighted)
					if (isItIn(&current->lNewGame,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(1,2);
						current->highlighted=1;
					}
					else if (isItIn(&current->lLoadGame,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(2,2);
						current->highlighted=2;
					}
					else if (isItIn(&current->lHighScores,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(3,2);
						current->highlighted=3;
					}
					else if (isItIn(&current->lCredits,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(4,2);
						current->highlighted=4;
					}
					else if (isItIn(&current->lQuit,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(5,2);
						current->highlighted=5;
					}
				}
				else if ((sEvent.type==SDL_MOUSEBUTTONDOWN) && (sEvent.button.button == SDL_BUTTON_LEFT))
				{
					mush->playClick();
					for (int i=0;i<btns.size(); i++)
					{
						if (isItIn(&btns[i]->pos,sEvent.motion.x,sEvent.motion.y))
						{
							btns[i]->press(true);
							//SDL_BlitSurface((btns[i].imgs)->ourImages[1].bitmap,NULL,ekran,&btns[i].pos);
							//updateRect(&btns[i].pos);
						}
					}
					if (currentMessage) continue;
					if (isItIn(&current->lNewGame,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(1,1);
						current->highlighted=1;
					}
					else if (isItIn(&current->lLoadGame,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(2,1);
						current->highlighted=2;
					}
					else if (isItIn(&current->lHighScores,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(3,1);
						current->highlighted=3;
					}
					else if (isItIn(&current->lCredits,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(4,1);
						current->highlighted=4;
					}
					else if (isItIn(&current->lQuit,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(5,1);
						current->highlighted=5;
					}
				}
				else if ((sEvent.type==SDL_MOUSEBUTTONUP) && (sEvent.button.button == SDL_BUTTON_LEFT))
				{
					for (int i=0;i<btns.size(); i++)
					{
						if (isItIn(&btns[i]->pos,sEvent.motion.x,sEvent.motion.y))
							(this->*(((Button<>*)btns[i])->fun))();
						else
						{
							btns[i]->press(false);
							//SDL_BlitSurface((btns[i].imgs)->ourImages[0].bitmap,NULL,ekran,&btns[i].pos);
							//updateRect(&btns[i].pos);
						}
					}
					if (currentMessage) continue;
					if (isItIn(&current->lNewGame,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(1,2);
						current->highlighted=1;
						(this->*(current->fNewGame))();
					}
					else if (isItIn(&current->lLoadGame,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(2,2);
						current->highlighted=2;
					}
					else if (isItIn(&current->lHighScores,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(3,2);
						current->highlighted=3;
					}
					else if (isItIn(&current->lCredits,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(4,2);
						current->highlighted=4;
					}
					else if (isItIn(&current->lQuit,sEvent.motion.x,sEvent.motion.y))
					{
						highlightButton(5,2);
						current->highlighted=5;
						(this->*(current->fQuit))();
					}
				}
				else if ((sEvent.type==SDL_MOUSEBUTTONDOWN) && (sEvent.button.button == SDL_BUTTON_RIGHT))
				{
					if (currentMessage) continue;
					if (isItIn(&current->lNewGame,sEvent.motion.x,sEvent.motion.y))
					{
						showCenBox(buttonText(0));
					}
					else if (isItIn(&current->lLoadGame,sEvent.motion.x,sEvent.motion.y))
					{
						showCenBox(buttonText(1));
					}
					else if (isItIn(&current->lHighScores,sEvent.motion.x,sEvent.motion.y))
					{
						showCenBox(buttonText(2));
					}
					else if (isItIn(&current->lCredits,sEvent.motion.x,sEvent.motion.y))
					{
						showCenBox(buttonText(3));
					}
					else if (isItIn(&ourMainMenu->lQuit,sEvent.motion.x,sEvent.motion.y))
					{
						showCenBox(buttonText(4));
					}
				}
				else if ((sEvent.type==SDL_MOUSEBUTTONUP) && (sEvent.button.button == SDL_BUTTON_RIGHT) && currentMessage)
				{
					hideBox();
				}
			}
		}
		catch(...)
		{ continue; }

		SDL_Delay(1); //give time for other apps
	}
	return ret;
}
std::string CPreGame::buttonText(int which)
{
	if (state==EState::mainMenu)
	{
		switch (which)
		{
		case 0:
			return preth->mainNewGame;
		case 1:
			return preth->mainLoadGame;
		case 2:
			return preth->mainHighScores;
		case 3:
			return preth->mainCredits;
		case 4:
			return preth->mainQuit;
		}
	}
	else if (state==EState::newGame)
	{
		switch (which)
		{
		case 0:
			return preth->ngSingleScenario;
		case 1:
			return preth->ngMultiplayer;
		case 2:
			return preth->ngCampain;
		case 3:
			return preth->ngTutorial;
		case 4:
			return preth->ngBack;
		}
	}
}
void CPreGame::quitAskBox()
{
	showAskBox("\"{}  Are you sure you want to quit?\"",NULL,NULL);
}
void CPreGame::sortMaps()
{
	std::sort(ourScenSel->mapsel.ourMaps.begin(),ourScenSel->mapsel.ourMaps.end(),mapSorter(ourScenSel->mapsel.sortBy));
	ourScenSel->mapsel.select(0);
	printMapsFrom(0);
}
void CPreGame::setTurnLength(int on)
{
	int min;
	switch (on)
	{
	case 0:
		min=1;
		break;
	case 1:
		min=2;
		break;
	case 2:
		min=4;
		break;
	case 3:
		min=6;
		break;
	case 4:
		min=8;
		break;
	case 5:
		min=10;
		break;
	case 6:
		min=15;
		break;
	case 7:
		min=20;
		break;
	case 8:
		min=25;
		break;
	case 9:
		min=30;
		break;
	case 10:
		min=0;
		break;
	}
	SDL_BlitSurface(ourOptions->bg,&genRect(23,134,256,547),ekran,&genRect(23,134,258,553));
	updateRect(&genRect(23,134,258,553));
	if (min)
	{
		std::ostringstream os;
		os<<min<<" Minutes";
		CSDL_Ext::printAtMiddle(os.str(),323,563,GEOR13);
	}
	else CSDL_Ext::printAtMiddle("Unlimited",323,563,GEOR13);
}