#pragma once
#define NOMINMAX
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
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
	bool er;
	int health;
};

struct Bullet : Entity{
	float trajx;
	float trajy;
	float mvtime;
};
class Timer {
private:
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> end;
public:
	void startTime() {
		start = std::chrono::steady_clock::now();
	}
	double getTime() {
		end = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}
	void resetTime() {
		start = std::chrono::steady_clock::now();
		end = std::chrono::steady_clock::now();
	}
};

enum {PPOSITION = 28, BPOSITION, DISCONNECT, CONNECT, NEWBULLET, RMVBULLET, HEALTH, DEATH, SCORE};

Gore::FreeList<Entity> players;
std::vector<Bullet> bullets;
std::mutex bt1;
Entity player = { 400, 400, 25, 25, 0, 100 };
bool isColliding(Entity b, Entity e) {
	if (b.x < e.x + e.w && b.x + b.w > e.x && b.y < e.y + e.h && b.y + b.h > e.y) {
		return true;
	}
	return false;
}


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
	std::cout << "Your ID is " << player->ID << "\n";
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
	player->health = 100;
	sendit:
	sendto(*udpSock, buf, 128, 0, (sockaddr*)server, sizeof(*server));
	//wait for udp server socket to send return connect packet
	char rbuf[128];
	ZeroMemory(rbuf, 128);
	int siz = sizeof(*server);
	int byts = recvfrom(*udpSock, rbuf, 128, 0, (sockaddr*)server, &siz);
	if (byts == 0) {
		goto sendit;
	}
	else if (rbuf[0] != CONNECT) {
		goto sendit;
	}
	std::cout << "UDP fully connected\n";

}

//only recieve data in udp
void udpRcv(SOCKET udpSock, sockaddr_in server, int pid) {
	char buf[128];
	int serverSize = sizeof(server);
	while (true) {
		ZeroMemory(buf, 128);
		int bytesIn = recvfrom(udpSock, buf, 128, 0, (sockaddr*)&server, &serverSize);
		if(bytesIn <= 0){
			std::cout << "exiting udp thread\n";
			closesocket(udpSock);
			return;
		}
		char* tt;
		float* tpo;
		int* ind;
		float tx, ty;
		int tid;
		bool push = true;
		//bool makenew = true;
		switch (buf[0]) {
		case PPOSITION:
			//std::cout << "recv player position\n";
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
			bt1.lock();
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
				p.er = false;
				players.insert(p);
			}
			bt1.unlock();
			break;
		case BPOSITION:
			tt = buf;
			tt++;
			ind = (int*)tt;
			bt1.lock();
			for (int i = 0; i < bullets.size(); i++) {
				if (*ind == bullets[i].ID) {
					ind++;
					tpo = (float*)ind;
					bullets[i].x = *tpo;
					tpo++;
					bullets[i].y = *tpo;
					tpo++;
					i = bullets.size();
				}
			}
			bt1.unlock();
			break;
		case HEALTH:
			tt = buf;
			tt++;
			ind = (int*)tt;
			player.health = *ind;
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
			int btsin = recv(copy.fd_array[0], buf, 128, 0);
			if (btsin <= 0) {
				std::cout << "Exiting tcp thread\n";
				closesocket(copy.fd_array[0]);
				return;
			}
			char* tt;
			float* t;
			int* tp;
			int temp = 0;
			float psx = 0, psy = 0;
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
				b.mvtime = 0;
				b.er = false;
				bullets.push_back(b);
				break;
			case RMVBULLET:
				tt = buf;
				tt++;
				tp = (int*)tt;
				bt1.lock();
				for (int i = 0; i < bullets.size(); i++) {
					if (bullets[i].ID == *tp) {
						bullets.erase(bullets.begin() + i);
						break;
					}
				}
				bt1.unlock();
				break;
			case DEATH:
				bt1.lock();
				tt = buf;
				tt++;
				tp = (int*)tt;
				temp = *tp;
				tp++;
				t = (float*)tp;
				psx = *t;
				t++;
				psy = *t;
				for (int i = 0; i < players.size(); i++) {
					if (players[i].ID == temp) {
						if (players[i].ID == player.ID) {
							player.x = psx;
							player.y = psy;
							player.health = 100;
						}
						players[i].x = psx;
						players[i].y = psy;
						break;
					}
				}
				bt1.unlock();
				break;
			case SCORE:
				tt = buf;
				tt++;
				tp = (int*)tt;
				bt1.lock();
				for (int i = 0; i < players.size(); i++) {
					if (players[i].ID == *tp) {
						tp++;
						std::cout << players[i].ID << " scored! New score is " << *tp << "\n";
						break;
					}
				}
				bt1.unlock();
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