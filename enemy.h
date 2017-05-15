#ifndef _JWL_ENEMY
#define _JWL_ENEMY

#include "agl/gameobject.h"
#include "agl/game.h"
#include "agl/drawer.h"
#include "agl/support.h"

#define ENEMY_SPEED_FACTOR 60.0

class Enemy : public AGL::GameObject
{
	public:
		AGL::Game* game;
		AGL::Drawer* drawer;

		AGL::Rectangle moveBounds;
		float timer;
		AGL::Vector2* speed;

		Enemy(AGL::Game* _game, AGL::Drawer* _drawer, AGL::Rectangle _moveBounds);
		
		~Enemy();

		void update();
		void draw();
};

#endif

