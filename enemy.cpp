#include "enemy.h"

#include <iostream>
using namespace std;

Enemy::Enemy(AGL::Game* _game, AGL::Drawer* _drawer, AGL::Rectangle _moveBounds)
{
	timer = 0;
	game = _game;
	drawer = _drawer;
	moveBounds = _moveBounds;
}

Enemy::~Enemy()
{
	
}

void Enemy::update()
{
	float deltaTime = game->getUpdateTime();

	AGL::Vector2 offset = (*speed) * (deltaTime * ENEMY_SPEED_FACTOR);	
	AGL::Vector2 newPos = *position + offset;

	if((newPos.x < moveBounds.x && speed->x < 0) ||
		(newPos.x > moveBounds.x + moveBounds.width - size->x && speed->x > 0))
		speed->x = speed->x * -1;

	if((newPos.y < moveBounds.y && speed->y < 0) ||
		(newPos.y > moveBounds.y + moveBounds.height - size->y && speed->y > 0))
		speed->y = speed->y * -1;

	delete position;
	position = new AGL::Vector2(newPos);
}

void Enemy::draw()
{
	AGL::IntVector2 pos = position->toIntVector2();
	drawer->draw(image, pos);
}

