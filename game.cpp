#include <stdlib.h>
#include <iostream>
#include "agl/game.h"
#include "agl/drawer.h"

#include "agl/color.h"
#include "agl/gameobject.h"
#include "agl/intvector2.h"
#include "agl/sound.h"
#include "agl/music.h"
#include "agl/filemanipulator.h"
#include "agl/format.h"

#include "enemy.h"
#include "stage.h"

#define MAXNAME 20
#define MAXLIVES 10
#define LIFEVALUE 10000
#define RANKING_SIZE 10

#define GS_PRESENTATION 0
#define GS_TITLE 1
#define GS_OPTIONS 2
#define GS_STAGE 3
#define GS_CLEAR 4
#define GS_DEAD 5
#define GS_OVER 6
#define GS_ENTERNAME 7
#define GS_RANKING 8
#define GS_CREDITS 9

#define LANG_ENGLISH 0
#define LANG_PORTUGUESE 1

#define RANKING_FILE "ranking.txt"
#define OPTIONS_FILE "options.txt"

bool debug = false;
const string VERSION = "1.5.2";

AGL::Game* game = NULL;
AGL::Drawer* drawer = NULL;

AGL::Mouse* mouse = NULL;
AGL::Keyboard* keyboard = NULL;
string keys = "abcdefghijklmnopqrstuvwxyz0123456789 ";

string playerName;
unsigned short gameState;
unsigned short lives;
unsigned int score;
bool congratulations;
vector<string> playerNames;
vector<unsigned int> playerScores;
short lastRank;
short curMenu;
unsigned short lastLives;

bool musicOn;
bool soundOn;
short language;

vector<AGL::Image*> images;
vector<AGL::Sound*> sounds;
vector<AGL::Music*> musics;
vector<AGL::Font*> fonts;
AGL::GameObject* thief = NULL;
Stage* stage = NULL;

string getString(unsigned short stringId);

void initialize()
{
	playerName = "Jwltief";
	game = new AGL::Game;
	drawer = new AGL::Drawer;
	mouse = new AGL::Mouse;
	keyboard = new AGL::Keyboard(true, true, true, true, false, true);
	keyboard->heldDelay = 45;
	keyboard->heldRepeat = 3;

	game->setWindowTitle(AGL::Format::stringToChar(&("Jwltief " + VERSION)));
	game->showMouse(true);
}

void loadOptions()
{
	if(!AGL::FileManipulator::exists(OPTIONS_FILE))
	{
		AGL::FileManipulator fm(OPTIONS_FILE);
		fm.openWriter();
		fm.writeLine("1");
		fm.writeLine("1");
		fm.writeLine("0");
		fm.closeWriter();
	}

	AGL::FileManipulator fm2(OPTIONS_FILE);
	fm2.openReader();
	musicOn = AGL::Format::stringToInt(fm2.readLine());
	soundOn = AGL::Format::stringToInt(fm2.readLine());
	language = AGL::Format::stringToInt(fm2.readLine());
}

void saveOptions()
{
	AGL::FileManipulator fm(OPTIONS_FILE);
	fm.openWriter();
	fm.writeLine(AGL::Format::intToString(musicOn));
	fm.writeLine(AGL::Format::intToString(soundOn));
	fm.writeLine(AGL::Format::intToString(language));
	fm.closeWriter();
}

void loadContent()
{
	thief = new AGL::GameObject(new AGL::Image(AGL_AGR, "content/misc/thief.agr"));
	sounds.push_back(new AGL::Sound(AGL_AGR, "content/sound/damage.agr"));
	sounds.push_back(new AGL::Sound(AGL_AGR, "content/sound/jewel.agr"));
	musics.push_back(new AGL::Music(AGL_AGR, "content/music/default.agr"));
	fonts.push_back(new AGL::Font("content/font/default.ttf", 20));
	fonts.push_back(new AGL::Font("content/font/menu.ttf", 30));
	images.push_back(new AGL::Image(AGL_AGR, "content/misc/aleva.agr"));
	images.push_back(new AGL::Image(AGL_AGR, "content/misc/title.agr"));
	images.push_back(new AGL::Image(AGL_AGR, "content/misc/cursor.agr"));
	images.push_back(new AGL::Image(AGL_AGR, "content/misc/menu.agr"));

	gameState = GS_PRESENTATION;
	loadOptions();
	if(musicOn) musics[0]->play(0.5);
}

void initializeStages()
{
	lives = MAXLIVES;
	lastLives = lives;
	score = 0;
	congratulations = false;
	if(stage != NULL) delete stage;
	stage = new Stage(game, drawer, thief, 1);
	game->showMouse(false);
}

int getRank(int _score)
{
	playerNames.clear();
	playerScores.clear();

	if(AGL::FileManipulator::exists(RANKING_FILE))
	{
		AGL::FileManipulator fm(RANKING_FILE);
		fm.openReader();

		while(!fm.end())
		{
			string line = fm.readLine();
			if(line == "") break;
			playerNames.push_back(line);
			playerScores.push_back(AGL::Format::stringToInt(fm.readLine()));
		}
		
		fm.closeReader();

		for(int i = 0; i <= playerScores.size(); i++)
			if(i == playerScores.size()) return i + 1;
			else if(_score >= playerScores[i]) return i + 1;

	}
	
	return 1;
}

void updateRanking(bool insertEntry)
{
	lastRank = -1;

	playerNames.clear();
	playerScores.clear();

	if(AGL::FileManipulator::exists(RANKING_FILE))
	{
		AGL::FileManipulator fm(RANKING_FILE);
		fm.openReader();

		while(!fm.end())
		{
			string line = fm.readLine();
			if(line == "") break;
			playerNames.push_back(line);
			playerScores.push_back(AGL::Format::stringToInt(fm.readLine()));
		}
		
		fm.closeReader();
	}

	if(insertEntry)
	{
		for(int i = 0; i <= playerScores.size(); i++)
			if(i == playerScores.size())
			{
				lastRank = i + 1;
				playerNames.push_back(playerName);
				playerScores.push_back(score);
				break;
			}
			else if(score >= playerScores[i])
			{
				lastRank = i + 1;
				playerNames.insert(playerNames.begin() + i, playerName);
				playerScores.insert(playerScores.begin() + i, score);
				break;
			}
	}

	AGL::FileManipulator fm2(RANKING_FILE);
	fm2.openWriter();

	for(int i = 0; i < RANKING_SIZE && i < playerScores.size(); i++)
	{
		fm2.writeLine(playerNames[i]);
		fm2.writeLine(AGL::Format::intToString(playerScores[i]));
	}

	fm2.closeWriter();
}

void clearRanking()
{
	if(AGL::FileManipulator::exists(RANKING_FILE))
	{
		AGL::FileManipulator fm(RANKING_FILE);
		fm.openWriter();
		fm.closeWriter();
	}
}

void update()
{
	game->update(keyboard, mouse);
	if(keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_ESC))) game->setQuit(true);

	AGL::Vector2 mpos(*(mouse->position));
	
	short lastMenu = curMenu;
	switch(gameState)
	{
		case GS_PRESENTATION:
			if(mouse->isLeftButtonPressed())
			{
				gameState = GS_TITLE;
			}
			break;

		case GS_TITLE:
			if(!AGL::Music::isPlaying() && musicOn) musics[0]->play(0.5);

			if(curMenu > 0 && (keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_UP))
				|| keyboard->isKeyHeld(keyboard->getIndex(AGL::KEY_UP)))) curMenu--;
			if(curMenu < 4 && (keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_DOWN))
				|| keyboard->isKeyHeld(keyboard->getIndex(AGL::KEY_DOWN)))) curMenu++;

			if(mouse->position->x > 185 && mouse->position->x < 385)
			{
				if(mouse->position->y > 280 && mouse->position->y < 330) curMenu = 0;
				else if(mouse->position->y > 330 && mouse->position->y < 380) curMenu = 1;
				else if(mouse->position->y > 380 && mouse->position->y < 430) curMenu = 2;
				else if(mouse->position->y > 430 && mouse->position->y < 480) curMenu = 3;
				else if(mouse->position->y > 480 && mouse->position->y < 530) curMenu = 4;
			}
			
			if(lastMenu != curMenu && soundOn) sounds[1]->play();

			if(mouse->isLeftButtonPressed() || keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_ENTER)))
			{
				switch(curMenu)
				{
					case 0:
						initializeStages();
						gameState = GS_STAGE;
						break;

					case 1:
						curMenu = 0;
						gameState = GS_OPTIONS;
						break;

					case 2:
						updateRanking(false);
						gameState = GS_RANKING;
						break;

					case 3:
						gameState = GS_CREDITS;
						break;

					case 4:
						game->setQuit(true);
						break;
				}
			}
			break;

		case GS_OPTIONS:
			if(curMenu > 0 && (keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_UP))
				|| keyboard->isKeyHeld(keyboard->getIndex(AGL::KEY_UP)))) curMenu--;
			if(curMenu < 4 && (keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_DOWN))
				|| keyboard->isKeyHeld(keyboard->getIndex(AGL::KEY_DOWN)))) curMenu++;

			if(mouse->position->x > 185 && mouse->position->x < 385)
			{
				if(mouse->position->y > 230 && mouse->position->y < 280) curMenu = 0;
				else if(mouse->position->y > 280 && mouse->position->y < 330) curMenu = 1;
				else if(mouse->position->y > 330 && mouse->position->y < 380) curMenu = 2;
				else if(mouse->position->y > 380 && mouse->position->y < 430) curMenu = 3;
				else if(mouse->position->y > 430 && mouse->position->y < 480) curMenu = 4;
			}
			
			if(lastMenu != curMenu && soundOn) sounds[1]->play();

			if(mouse->isLeftButtonPressed() || keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_ENTER)))
			{
				switch(curMenu)
				{
					case 0:
						musicOn = !musicOn;
						break;

					case 1:
						soundOn = !soundOn;
						break;

					case 2:
						if(language == LANG_ENGLISH) language = LANG_PORTUGUESE;
						else language = LANG_ENGLISH;
						break;
					
					case 3:
						clearRanking();
						if(soundOn) sounds[1]->play();
						break;

					case 4:
						saveOptions();
						loadOptions();
						if(AGL::Music::isPlaying() && !musicOn) musics[0]->stop();
						curMenu = 0;
						gameState = GS_TITLE;
						break;
				}
			}
			break;

		case GS_STAGE:
			musics[0]->stop();
			*(thief->position) = mpos;

			if(mouse->position->x > drawer->screenSize->x - thief->size->x) thief->position->x = drawer->screenSize->x - thief->size->x;
			if(mouse->position->y > drawer->screenSize->y - thief->size->y) thief->position->y = drawer->screenSize->y - thief->size->y;

			stage->update();
			if(stage->isColliding()) 
			{
				lives--;
				if(soundOn) sounds[0]->play();
			}

			if(stage->gotJewel())
			{
				score += stage->jewelValue;
				if(soundOn) sounds[1]->play(0.5);
			} 

			if(lives == 0) gameState = GS_DEAD;
			if(stage->isClear()) gameState = GS_CLEAR;
			break;

		case GS_CLEAR:
			lastLives = lives;
			if(Stage::exists(stage->number + 1)) 
			{
				delete stage;
				stage = new Stage(game, drawer, thief, stage->number + 1);
				gameState = GS_STAGE;
			}
			else 
				gameState = GS_OVER;
			break;

		case GS_DEAD:
			congratulations = false;
			if(getRank(score) <= RANKING_SIZE) gameState = GS_ENTERNAME;
			else 
			{
				updateRanking(false);
				gameState = GS_RANKING;
				game->showMouse(true);
			}
			break;

		case GS_OVER:
			score += lives * LIFEVALUE;
			congratulations = true;
			if(getRank(score) <= RANKING_SIZE) gameState = GS_ENTERNAME;
			else 
			{
				delete stage;
				updateRanking(false);
				gameState = GS_RANKING;
				game->showMouse(true);
			}
			break;	

		case GS_ENTERNAME:
			stage->update();
			if((keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_BACKSPACE)) 
				|| keyboard->isKeyHeld(keyboard->getIndex(AGL::KEY_BACKSPACE))) && playerName.length() > 0) 
				playerName = playerName.substr(0, playerName.length() - 1);

			if(playerName.length() < MAXNAME)
			{
				for(int i = 0; i < 26; i++)
					if(keyboard->isKeyPressed(i) || keyboard->isKeyHeld(i)) 
					{
						if(keyboard->isKeyDown(keyboard->getIndex(AGL::KEY_RIGHTSHIFT))
							|| keyboard->isKeyDown(keyboard->getIndex(AGL::KEY_LEFTSHIFT)))
							playerName += toupper(keys[i]);
						else
							playerName += keys[i];
					}

				for(int i = 26; i < 37; i++)
					if(keyboard->isKeyPressed(i) || keyboard->isKeyHeld(i)) playerName += keys[i];

				if(keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_SPACE)) || keyboard->isKeyHeld(keyboard->getIndex(AGL::KEY_SPACE)))
					playerName += " ";
			}

			if(keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_ENTER)) || mouse->isLeftButtonPressed()) 
			{
				updateRanking(true);
				gameState = GS_RANKING;
				game->showMouse(true);
			}
			break;

		case GS_RANKING:
			if(keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_ENTER)) || mouse->isLeftButtonPressed()) gameState = GS_TITLE;
			break;

		case GS_CREDITS:
			if(keyboard->isKeyPressed(keyboard->getIndex(AGL::KEY_ENTER)) || mouse->isLeftButtonPressed()) gameState = GS_TITLE;
			break;			
	}

	//if(game->gameCounter == 0) cout << "Tempo do ciclo: " << game->cycleTime << endl;
	//if(game->gameCounter == 0) cout << "Tempo do update: " << game->getUpdateTime() << endl;
	if(debug && game->gameCounter == 0) cout << "FPS: " << game->getFps(1) << endl;
}

void drawShadowedText(char* text, AGL::IntVector2* position, AGL::Color color)
{
	AGL::IntVector2 pos(position->x - 2, position->y - 2);
	drawer->drawText(fonts[0], text, pos, AGL_COLOR_WHITE);
	drawer->drawText(fonts[0], text, *position, color);
}

void drawShadowedText(char* text, AGL::IntVector2 &position)
{
	AGL::IntVector2 pos(position.x - 2, position.y - 2);
	drawer->drawText(fonts[0], text, pos, AGL_COLOR_WHITE);
	drawer->drawText(fonts[0], text, position, AGL_COLOR_BLACK);
}

void drawPanel()
{
	AGL::IntVector2 p1(700, 570);
	AGL::IntVector2 p2(10, 570);
	drawShadowedText(AGL::Format::stringToChar(&(getString(0) + ": " + AGL::Format::intToString(lives))), p1);
	drawShadowedText(AGL::Format::stringToChar(&(getString(1) + ": " + AGL::Format::intToString(score))), p2);
}

void draw()
{
	string strLanguage = getString(2);
	string strSound = getString(4);
	string strMusic = getString(4);

	AGL::IntVector2 tpos = thief->position->toIntVector2();
	AGL::IntVector2 p1(200, 60);
	AGL::IntVector2 p2(330, 350);
	AGL::IntVector2 p3(275, 560);
	AGL::IntVector2 zero(0, 0);
	AGL::IntVector2 p4(385, 280 + curMenu * 50);
	AGL::IntVector2 p5(185, 280);
	AGL::IntVector2 p6(190, 330);
	AGL::IntVector2 p7(190, 380);
	AGL::IntVector2 p8(195, 430);
	AGL::IntVector2 p9(200, 480);
	
	AGL::IntVector2 p10(185, 230);
	AGL::IntVector2 p11(185, 280);
	AGL::IntVector2 p12(185, 330);
	AGL::IntVector2 p13(185, 380);
	AGL::IntVector2 p14(185, 430);
	AGL::IntVector2 p15(140, 230 + curMenu * 50);
	AGL::IntVector2 p16(20, 20);
	
	AGL::IntVector2 p17(150, 250);
	AGL::IntVector2 p18(230, 250);
	AGL::IntVector2 p19(150, 280);
	AGL::IntVector2 p20(230, 320);
	AGL::IntVector2 p21(245, 320);
	
	AGL::IntVector2 p26(10, 550);
	
	AGL::IntVector2 p27(100, 130);
	AGL::IntVector2 p28(150, 180);
	AGL::IntVector2 p29(100, 250);
	AGL::IntVector2 p30(150, 300);
	AGL::IntVector2 p31(100, 370);
	AGL::IntVector2 p32(150, 420);
	
	AGL::IntVector2 p33(20, 20);

	switch(gameState)
	{
		case GS_PRESENTATION:
			drawer->clearScreen(AGL_COLOR_DARKRED); //AGL_COLOR_DARKVIOLET
			drawer->draw(images[0], p1);
			drawer->drawText(fonts[0], AGL::Format::stringToChar(&(getString(6) + "...")), p2, AGL_COLOR_WHITE);
			drawer->drawText(fonts[0], "http://alevagames.com", p3, AGL_COLOR_WHITE);
			curMenu = 0;
			break;

		case GS_TITLE:
			drawer->draw(images[1], zero);
			drawer->draw(images[2], p4);

			AGL::Color colors[5];
			colors[0] = AGL_COLOR_WHITE;
			colors[1] = AGL_COLOR_WHITE;
			colors[2] = AGL_COLOR_WHITE;
			colors[3] = AGL_COLOR_WHITE;
			colors[4] = AGL_COLOR_WHITE;
			colors[curMenu] = AGL_COLOR_ORANGE;

			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(7)), p5, colors[0]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(8)), p6, colors[1]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(9)), p7, colors[2]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(10)), p8, colors[3]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(11)), p9, colors[4]);
			break;

		case GS_OPTIONS:
			drawer->draw(images[3], zero);
			drawer->draw(images[2], p15);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(8)), p16, AGL_COLOR_WHITE);

			AGL::Color colors2[5];
			colors2[0] = AGL_COLOR_WHITE;
			colors2[1] = AGL_COLOR_WHITE;
			colors2[2] = AGL_COLOR_WHITE;
			colors2[3] = AGL_COLOR_WHITE;
			colors2[4] = AGL_COLOR_WHITE;
			colors2[curMenu] = AGL_COLOR_ORANGE;

			if(language == LANG_PORTUGUESE) strLanguage = getString(3);
			if(!soundOn) strSound = getString(5);
			if(!musicOn) strMusic = getString(5);

			drawer->drawText(fonts[1], AGL::Format::stringToChar(&(getString(12) + " " + strMusic)), p10, colors2[0]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&(getString(13) + " " + strSound)), p11, colors2[1]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&(getString(14) + ": " + strLanguage)), p12, colors2[2]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(24)), p13, colors2[3]);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(15)), p14, colors2[4]);
			break;

		case GS_STAGE:
			stage->draw();	
			drawer->draw(thief->image, tpos);
			drawPanel();
			break;

		case GS_CLEAR:
			stage->draw();	
			drawer->draw(thief->image, tpos);
			break;

		case GS_DEAD:
			break;

		case GS_OVER:
			break;	

		case GS_ENTERNAME:
			stage->draw();	
			drawer->draw(thief->image, tpos);
			drawPanel();
			if(congratulations) drawShadowedText(AGL::Format::stringToChar(&getString(16)), p17);
			else drawShadowedText(AGL::Format::stringToChar(&getString(17)), p18);
			drawShadowedText(AGL::Format::stringToChar(&getString(18)), p19);
			if(game->gameCounter < 125) drawShadowedText("_", p20);
			drawShadowedText(AGL::Format::stringToChar(&playerName), p21);
			break;

		case GS_RANKING:
			drawer->draw(images[3], zero);
			drawer->drawText(fonts[1], "###   R A N K I N G   ###", p33, AGL_COLOR_WHITE);

			for(int i = 0; i < RANKING_SIZE && i < playerScores.size(); i++) 
			{
				if(lastRank == i + 1)
				{
					AGL::IntVector2 p22(80, 110 + 35 * i);
					AGL::IntVector2 p23(500, 110 + 35 * i);
					drawer->drawText(fonts[1], AGL::Format::stringToChar(&playerNames[i]), p22, AGL_COLOR_ORANGE);
					drawer->drawText(fonts[1], AGL::Format::stringToChar(&AGL::Format::intToString(playerScores[i])), p23, AGL_COLOR_ORANGE);
				}
				else
				{
					AGL::IntVector2 p24(80, 110 + 35 * i);
					AGL::IntVector2 p25(500, 110 + 35 * i);
					drawer->drawText(fonts[1], AGL::Format::stringToChar(&playerNames[i]), p24, AGL_COLOR_WHITE);
					drawer->drawText(fonts[1], AGL::Format::stringToChar(&AGL::Format::intToString(playerScores[i])), p25, AGL_COLOR_WHITE);
				}
			}

			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(19)), p26, AGL_COLOR_WHITE);
			break;

		case GS_CREDITS:
			drawer->draw(images[3], zero);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&(getString(20) + ":")), p27, AGL_COLOR_WHITE);
			drawer->drawText(fonts[1], "YURI DAVID SANTOS", p28, AGL_COLOR_WHITE);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&(getString(21) + ":")), p29, AGL_COLOR_WHITE);
			drawer->drawText(fonts[1], "VICTOR, NATHALIA", p30, AGL_COLOR_WHITE);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&getString(22)), p31, AGL_COLOR_WHITE);
			drawer->drawText(fonts[1], AGL::Format::stringToChar(&("Jewel Thief " + getString(23) + " PAUL LIGESKI, 1991")), p32, AGL_COLOR_WHITE);
			break;	
	}

	drawer->updateScreen();
}

void unloadContent()
{
	game->end();
}

int main(int argc, char *argv[])
{
	if(argc > 1 && AGL::Format::compareCharString(argv[1], "-d")) debug = true;

	initialize();
	loadContent();

	game->runFixedFps(update, draw, 60);

	unloadContent();

   return 0;
}

string getString(unsigned short stringId)
{
	switch(language)
	{
		case LANG_ENGLISH:
			switch(stringId)
			{
				case 0: return "Lives";
				case 1: return "Score";
				case 2: return "english";
				case 3: return "portuguese";
				case 4:	return "ON";
				case 5:	return "OFF";
				case 6:	return "Presents";
				case 7:	return "START GAME";
				case 8:	return "   OPTIONS";
				case 9:	return "   RANKING";
				case 10: return "   CREDITS";
				case 11: return "      QUIT";
				case 12: return "Music";
				case 13: return "Sound Effects";
				case 14: return "Language";
				case 15: return "Save Options";
				case 16: return "Congratulations! You are really skilled!!!";
				case 17: return "Too bad... You are dead!";
				case 18: return "Type your name and press <ENTER>:";
				case 19: return "Press <ENTER> or click to back to menu";
				case 20: return "Production and Development";
				case 21: return "Special thanks to";
				case 22: return "Based on the classic";
				case 23: return "by";
				case 24: return "Clear Ranking";
			}
			break;

		case LANG_PORTUGUESE:
			switch(stringId)
			{
				case 0:	return "Vidas";
				case 1:	return "Pontos";
				case 2:	return "ingles";
				case 3:	return "portugues";
				case 4:	return "SIM";
				case 5:	return "NAO";
				case 6:	return "Apresenta";
				case 7:	return "   INICIAR";
				case 8:	return "    OPCOES";
				case 9:	return "   RANKING";
				case 10: return "  CREDITOS";
				case 11: return "      SAIR";
				case 12: return "Musica";
				case 13: return "Som";
				case 14: return "Idioma";
				case 15: return "Salvar Opcoes";
				case 16: return "Parabens! Voce e realmente habilidoso!!!";
				case 17: return "Que pena... Voce morreu!";
				case 18: return "Digite seu nome e aperte <ENTER>:";
				case 19: return "Aperte <ENTER> ou clique para ir ao menu";
				case 20: return "Producao e Desenvolvimento";
				case 21: return "Agradecimentos especiais";
				case 22: return "Baseado no classico";
				case 23: return "de";
				case 24: return "Limpar Ranking";
			}
			break;
	}
}

