#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <asio.hpp>
#include "GoreEngine.h"

struct Entity {
	float x;
	float y;
	Uint32 w;
	Uint32 h;
};

struct Player : Entity {
	Uint32 index;
	Uint32 health;
};

struct Bullet : Entity {
	float trajx;
	float trajy;
	//index of player it was fired from
	Uint32 index;
};
struct TimerObject {
	float time;
	float maxtime;
};


class Game {
private:

public:
	static bool isColliding(Entity* e1, Entity* e2);
};