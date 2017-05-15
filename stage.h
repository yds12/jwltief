#ifndef _JWL_STAGE
#define _JWL_STAGE

#include <vector>

#include "agl/game.h"
#include "agl/drawer.h"
#include "agl/gameobject.h"
#include "agl/image.h"
#include "agl/support.h"

#include "enemy.h"

const float PROTECTION_TIME = 0.35;
const float INITIAL_PROTECTION_TIME = 0.6;

class Stage
{
	private:
		bool initialProtectionOver;
		bool _isColliding;
		bool _gotJewel;
		float collisionTimer;

	public:
		AGL::Game* game;
		AGL::Drawer* drawer;
		AGL::GameObject* thief;

		unsigned short number;
		unsigned short jewelValue;
		AGL::Image* background;
		vector<Enemy*> enemies;
		vector<AGL::GameObject*> jewels;

		Stage(AGL::Game* _game, AGL::Drawer* _drawer, AGL::GameObject* _thief, unsigned short _number);
		~Stage();

		static bool exists(unsigned short _number);
		void update();
		void draw();
		void checkCollision();
		bool isColliding();
		bool gotJewel();
		bool isClear();
};

#endif

