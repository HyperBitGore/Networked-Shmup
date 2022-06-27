#pragma once
#define NOMINMAX
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <string>
#include <vector>
#include <time.h>
#include <algorithm>
#include "GoreEngine.h"

#pragma comment(lib, "ws2_32.lib")

//tcp recvs like end of game and other of the like will be recieved on main thread with select
//new bullets or players are created when client hasn't seen their index yet when sent

struct Entity {
	float x;
	float y;
	int w;
	int h;
	int ID;
};

struct Bullet : Entity{
	float trajx;
	float trajy;
};


enum {PPOSITION = 28, BPOSITION, DISCONNECT, CONNECT, NEWBULLET};

Gore::FreeList<Entity> players;
std::vector<Bullet> bullets;

void connectToServer(SOCKET* udpSock, SOCKET* tcpSock, sockaddr_in *server, Entity* player) {
	std::string serverip;
	std::cout << "Input server ip\n";
	std::cin >> serverip;
	std::cout << std::endl;


	//figure out ip to send UDP packets to
	server->sin_family = AF_INET;
	server->sin_port = htons(6890);
	inet_pton(AF_INET, serverip.c_str(), &server->sin_addr);
	*udpSock = socket(AF_INET, SOCK_DGRAM, 0);
	//connect TCP socket, which is only used for sending username and other things
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(6891);
	inet_pton(AF_INET, serverip.c_str(), &hint.sin_addr);
	*tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	int rsc = connect(*tcpSock, (sockaddr*)&hint, sizeof(hint));
	if (rsc == SOCKET_ERROR) {
		std::cerr << "Socket connection failed ERR: " << WSAGetLastError() << std::endl;
		return;
	}
	char buf[128];
	ZeroMemory(buf, 128);
	//connect event where you recieve data like what ur id and the spawn point
	recv(*tcpSock, buf, 128, 0);
	float* t = (float*)buf;
	player->x = *t;
	t++;
	player->y = *t;
	t++;
	int* tt = (int*)t;
	player->ID = *tt;
	player->w = 25;
	player->h = 25;
	ZeroMemory(buf, 128);
	buf[0] = CONNECT;
	char* tpt = buf;
	tpt++;
	t = (float*)tpt;
	*t = player->x;
	t++;
	*t = player->y;
	t++;
	tt = (int*)t;
	*tt = player->ID;
	sendto(*udpSock, buf, 128, 0, (sockaddr*)server, sizeof(*server));
}

//only recieve data in udp
void udpRcv(SOCKET udpSock, sockaddr_in server, int pid) {
	char buf[128];
	int serverSize = sizeof(server);
	while (true) {
		ZeroMemory(buf, 128);
		int bytesIn = recvfrom(udpSock, buf, 128, 0, (sockaddr*)&server, &serverSize);
		char* tt;
		float* tpo;
		int* ind;
		float tx, ty;
		int tid;
		bool push = true;
		//bool makenew = true;
		switch (buf[0]) {
		case PPOSITION:
			std::cout << "recv player position\n";
			//reading the x, y, and index from packet
			tt = buf;
			tt++;
			tpo = (float*)tt;
			tx = *tpo;
			tpo++;
			ty = *tpo;
			tpo++;
			ind = (int*)tpo;
			tid = *ind;
			push = true;
			for (int i = 0; i < players.size(); i++) {
				if (players[i].ID == tid) {
					players[i].x = tx;
					players[i].y = ty;
					i = players.size();
					push = false;
				}
			}
			if (push) {
				Entity p;
				p.x = tx;
				p.y = ty;
				p.ID = tid;
				p.w = 25;
				p.h = 25;
				players.insert(p);
			}
			break;
		case BPOSITION:
			tt = buf;
			tt++;
			ind = (int*)tt;
			for (int i = 0; i < bullets.size(); i++) {
				if (*ind == bullets[i].ID) {
					ind++;
					tpo = (float*)ind;
					bullets[i].x = *tpo;
					tpo++;
					bullets[i].y = *tpo;
					tpo++;
					break;
				}
			}

			break;
		}
	}
}
void tcpRcv(SOCKET tcpSock) {
	fd_set master;
	FD_ZERO(&master);
	FD_SET(tcpSock, &master);
	char buf[128];
	while (true) {
		fd_set copy = master;
		int socksTo = select(0, &copy, nullptr, nullptr, nullptr);
		if (socksTo >= 1) {
			ZeroMemory(buf, 128);
			recv(copy.fd_array[0], buf, 128, 0);
			char* tt;
			float* t;
			int* tp;
			Bullet b;
			switch (buf[0]) {
			case NEWBULLET:
				tt = buf;
				tt++;
				t = (float*)tt;
				b.x = *t;
				t++;
				b.y = *t;
				t++;
				b.trajx = *t;
				t++;
				b.trajy = *t;
				t += 2;
				tp = (int*)t;
				b.ID = *tp;
				b.w = 5;
				b.h = 5;
				bullets.push_back(b);
				break;
			}
		}
	}
}

void calcSlope(int x1, int y1, int x2, int y2, float* dx, float* dy) {
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