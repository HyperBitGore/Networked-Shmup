#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <thread>
#include "GoreEngine.h"

extern int playerindex;
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

enum { NEWPLAYER = 24, NEWBULLET, BULLETPOS, PLAYERPOS };

class Game {
private:

public:
	static bool isColliding(Entity* e1, Entity* e2);
	static void connectToServer(std::string ip, std::vector<Player>& players, std::vector<Bullet>& bullets, Player* p1);
};