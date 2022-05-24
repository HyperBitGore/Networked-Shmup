#pragma once
#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <asio.hpp>
#include "GoreEngine.h"

extern asio::io_context io;
extern asio::ip::udp::socket udpsock;
struct Entity {
	float x;
	float y;
	UINT32 w;
	UINT32 h;
};

struct Player : Entity {
	UINT32 index;
	UINT32 health;
};

struct Bullet : Entity {
	float trajx;
	float trajy;
	//index of player it was fired from
	UINT32 index;
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
	static void connectToServer(std::string ip, std::vector<Player>& players, std::vector<Bullet>& bullets);
};