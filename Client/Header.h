#pragma once
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

enum {PPOSITION = 28, BPOSITION, DISCONNECT, CONNECT};

Gore::FreeList<Entity> players;

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
		}
	}
}