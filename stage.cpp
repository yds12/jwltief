#include "agl/filemanipulator.h"
#include "agl/format.h"

#include "stage.h"

#include <iostream>
using namespace std;

Stage::Stage(AGL::Game* _game, AGL::Drawer* _drawer, AGL::GameObject* _thief, unsigned short _number)
{
	enemies.clear();
	jewels.clear();
	initialProtectionOver = false;
	number = _number;
	game = _game;
	drawer = _drawer;
	thief = _thief;
	collisionTimer = 0;
	_isColliding = false;

	string fileName = "content/stage/stage" + AGL::Format::formatInt(_number, 2);
	AGL::FileManipulator fm(fileName);
	fm.openReader();

	if(!fm.ready()) 
	{
		cout << "Não pode abrir arquivo " << fileName << endl;
		return;
	}

	if(!fm.end())
		background = new AGL::Image(AGL_AGR, AGL::Format::stringToChar(&("content/bg/" + fm.readLine())));

	string enemyTexPath;
	string jewelTexPath;

	if(!fm.end())
		enemyTexPath = "content/sprite/" + fm.readLine();

	if(!fm.end())
		jewelTexPath = "content/jewel/" + fm.readLine();

	if(!fm.end())
		jewelValue = AGL::Format::stringToInt(fm.readLine());

	AGL::Rectangle moveBounds;

	if(!fm.end())
	{
		string line = fm.readLine();
		moveBounds.x = AGL::Format::stringToInt(line.substr(0, 4));
		moveBounds.y = AGL::Format::stringToInt(line.substr(5, 4));
		moveBounds.width = AGL::Format::stringToInt(line.substr(10, 4));
		moveBounds.height = AGL::Format::stringToInt(line.substr(15, 4));
	}

	AGL::Image* jewelImage = new AGL::Image(AGL_AGR, AGL::Format::stringToChar(&jewelTexPath));
	AGL::Image* enemyImage = new AGL::Image(AGL_AGR, AGL::Format::stringToChar(&enemyTexPath));

	while(!fm.end())
	{
		string line = fm.readLine();
		if(line.length() == 9) // jewel
		{
			AGL::GameObject* jewel = new AGL::GameObject(jewelImage);
			jewel->position = new AGL::Vector2(AGL::Format::stringToInt(line.substr(0, 4)), AGL::Format::stringToInt(line.substr(5, 4)));
			jewels.push_back(jewel);
		}
		else if(line.length() == 21) // enemy
		{
			Enemy* enemy = new Enemy(game, drawer, moveBounds);
			enemy->image = enemyImage;
			enemy->position = new AGL::Vector2(AGL::Format::stringToInt(line.substr(0, 4)), AGL::Format::stringToInt(line.substr(5, 4)));
			enemy->speed = new AGL::Vector2(AGL::Format::stringToFloat(line.substr(10, 5)), AGL::Format::stringToFloat(line.substr(16, 5)));
			enemy->size = new AGL::Vector2(enemy->image->size->x, enemy->image->size->y);
			enemies.push_back(enemy);
		}
	}

	fm.closeReader();
}

Stage::~Stage()
{
	// deleta os inimigos
	for(int i = 0; i < enemies.size(); i++)
	{
		delete enemies[i]->position;
		delete enemies[i]->speed;
		delete enemies[i]->imageGap;
		delete enemies[i]->size;
	}
	delete enemies[0]->image;
	
	// deleta as jóias
	for(int i = 0; i < jewels.size(); i++)
	{
		delete jewels[i]->position;
		delete jewels[i]->imageGap;
		delete jewels[i]->size;
	}
	delete jewels[0]->image;	
			
	// deleta o bg
	delete background;
}

void Stage::update()
{
	for(int i = 0; i < enemies.size(); i++)
		enemies[i]->update();

	checkCollision();
}

void Stage::draw()
{
	AGL::IntVector2 zero(0, 0);
	drawer->clearScreen();
	drawer->draw(background, zero);

	for(int i = 0; i < jewels.size(); i++)
	{
		AGL::IntVector2 pos = jewels[i]->position->toIntVector2();
		drawer->draw(jewels[i]->image, pos);
	}

	for(int i = 0; i < enemies.size(); i++)
		enemies[i]->draw();
}

void Stage::checkCollision()
{
	if(initialProtectionOver)
	{
		if(!_isColliding)
		{
			for(int i = 0; i < enemies.size(); i++)
			{
				if(enemies[i]->collides(thief))
				{
					_isColliding = true;
					break;
				}
			}
		}
		else
		{
			collisionTimer += game->getUpdateTime();
			if(collisionTimer >= PROTECTION_TIME)
			{
				collisionTimer = 0;
				_isColliding = false;
			}
		}
	}
	else
	{
		collisionTimer += game->getUpdateTime();
		if(collisionTimer >= INITIAL_PROTECTION_TIME)
		{
			collisionTimer = 0;
			initialProtectionOver = true;
		}
	}

	int jewelIndex  = -1;
	_gotJewel = false;

	for(int i = 0; i < jewels.size(); i++)
	{
		if(jewels[i]->collides(thief))
		{
			_gotJewel = true;
			jewelIndex = i;
			break;
		}
	}

	if(_gotJewel) jewels.erase(jewels.begin() + jewelIndex);
}

bool Stage::isColliding()
{
	return _isColliding && collisionTimer == 0;
}

bool Stage::gotJewel()
{
	return _gotJewel;
}

bool Stage::isClear()
{
	return jewels.size() == 0;
}

bool Stage::exists(unsigned short _number)
{
	string fileName = "content/stage/stage" + AGL::Format::formatInt(_number, 2);
	return AGL::FileManipulator::exists(fileName);
}

