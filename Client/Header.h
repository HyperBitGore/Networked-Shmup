#pragma once
#include <iostream>
#include <asio.hpp>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
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


enum { NEWPLAYER = 24, NEWBULLET, BULLETPOS, PLAYERPOS, CLOSE };

class Game {
private:

public:
	static bool isColliding(Entity* e1, Entity* e2);
	static void connectToServer(std::string ip, std::vector<Player>& players, std::vector<Bullet>& bullets, Player* p1);
	static void calcSlope(int x1, int y1, int x2, int y2, float* dx, float* dy) {
		int steps = std::max(abs(x1 - x2), abs(y1 - y2));
		if (steps == 0) {
			*dx = *dy = 0;
			return;
		}
		*dx = (x1 - x2);
		*dx /= steps;

		*dy = (y1 - y2);
		*dy /= steps;
	}
};