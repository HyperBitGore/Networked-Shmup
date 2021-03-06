#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
#include <vector>
#include <time.h>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

struct Entity {
	float x;
	float y;
	int ID;
	int health;
	sockaddr_in addr;
	float sx;
	float sy;
	int score;
};
struct Bullet {
	float x;
	float y;
	float trajx;
	float trajy;
	int ID;
	int PID;
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
std::vector<sockaddr_in> addresses;
std::vector<Entity> players;
std::vector<Bullet> bullets;
std::mutex mt1;
fd_set master;

enum { PPOSITION = 28, BPOSITION, DISCONNECT, CONNECT, NEWBULLET, RMVBULLET, HEALTH, DEATH, SCORE};

bool isColliding(Bullet b, Entity p) {
	return (b.x < p.x + 25 && b.x + 5 > p.x && b.y < p.y + 25 && b.y + 5 > p.y);
}


void tcpThread() {
	FD_ZERO(&master);
	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == INVALID_SOCKET) {
		std::cerr << "Socket creation failed " << std::endl;
		return;
	}
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(6891);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(listener, (sockaddr*)&hint, sizeof(hint));
	//tells winsock that the socket is for listening
	listen(listener, SOMAXCONN);
	FD_SET(listener, &master);
	char buf[128];
	while (true) {
		mt1.lock();
		fd_set copy = master;
		mt1.unlock();
		int sockCount = select(0, &copy, nullptr, nullptr, nullptr);
		for (int i = 0; i < sockCount; i++) {
			ZeroMemory(buf, 128);
			SOCKET sock = copy.fd_array[i];
			if (sock == listener) {
				//make connection and push address into addresses vector
				sockaddr_in client;
				int clientsize = sizeof(client);

				SOCKET clientSocket = accept(sock, (sockaddr*)&client, &clientsize);
				if (clientSocket == INVALID_SOCKET) {
					std::cerr << "Client connect failed" << std::endl;
					return;
				}
				char host[NI_MAXHOST]; //clients remote name
				char service[NI_MAXHOST]; //service (port) client is connection on

				ZeroMemory(host, NI_MAXHOST);
				ZeroMemory(host, NI_MAXHOST);

				if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
					std::cout << host << " connected on port " << service << std::endl;
				}
				else {
					inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
					std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
				}
				char* t = buf;
				Entity p;
				float* tt = (float*)buf;
				*tt = rand() % 750 + 70;
				p.x = *tt;
				tt++;
				*tt = rand() % 750 + 70;
				p.y = *tt;
				p.sx = p.x;
				p.sy = p.y;
				tt++;
				int* ttp = (int*)tt;
				*ttp = players.size();
				p.ID = *ttp;
				p.addr.sin_port = 0;
				p.health = 100;
				p.score = 0;
				players.push_back(p);
				std::cout << players.size() << std::endl;
				send(clientSocket, buf, 128, 0);
				mt1.lock();
				FD_SET(clientSocket, &master);
				mt1.unlock();
			}
			else {
				//recv data from socket
				recv(copy.fd_array[i], buf, 128, 0);
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
					t++;
					tp = (int*)t;
					b.PID = *tp;
					b.ID = bullets.size();
					bullets.push_back(b);
					tt = buf;
					tt++;
					tp = (int*)tt;
					tp += 5;
					*tp = b.ID;
					mt1.lock();
					for (int j = 0; j < master.fd_count; j++) {
						send(master.fd_array[j], buf, 128, 0);
					}
					mt1.unlock();
					break;
				case DISCONNECT:
					mt1.lock();
					tt = buf;
					tt++;
					tp = (int*)tt;
					for (int kl = 0; kl < addresses.size(); kl++) {
						if (addresses[kl].sin_addr.S_un.S_addr == players[*tp].addr.sin_addr.S_un.S_addr && addresses[kl].sin_port == players[*tp].addr.sin_port) {
							addresses.erase(addresses.begin() + kl);
							std::cout << "Erase success\n";
						}
					}
					std::cout << "DC'd player ID: " << *tp << "\n";
					players.erase(players.begin() + *tp);
					mt1.unlock();
					break;
				}
			}
		}
	}
}


int main() {
	srand(time(NULL));
	//start winsock
	WSADATA wdata;
	WORD version = MAKEWORD(2, 2);
	int wsOK = WSAStartup(version, &wdata);
	if (wsOK != 0) { std::cout << "can't start winsock! " << wsOK << std::endl; return -1; }
	//create both listening sockets
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
	if (in == SOCKET_ERROR) { std::cout << "can't create socket!" << WSAGetLastError() << std::endl; return -1;}
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(6890);
	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR) { std::cout << "can't bind socket!" << WSAGetLastError() << std::endl; return -1; }
	
	sockaddr_in client;
	int clientLen = sizeof(client);
	ZeroMemory(&client, sizeof(client));



	bool exitf = false;
	Timer btime;
	btime.startTime();
	char buf[128];
	//break off seperate thread for tcp listening
	std::thread tcpT(tcpThread);
	fd_set udpm;
	FD_ZERO(&udpm);
	FD_SET(in, &udpm);
	while (!exitf) {
		ZeroMemory(buf, 128);
		fd_set copy = udpm;
		bool push = true;
		int socs = select(0, &copy, nullptr, nullptr, nullptr);
		if (socs >= 1) {
			int bytesIn = recvfrom(copy.fd_array[0], buf, 128, 0, (sockaddr*)&client, &clientLen);
			mt1.lock();
			if (bytesIn == SOCKET_ERROR) {
				std::cout << "Error recv from client " << WSAGetLastError() << std::endl;
			}
			else {
				//read data sent and then send back out
				char clientIp[256];
				ZeroMemory(clientIp, 256);
				inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
				float tx, ty;
				char* cpo;
				int* ct;
				float* cc;
				switch (buf[0]) {
				case PPOSITION:
					cpo = buf;
					cpo++;
					cc = (float*)cpo;
					tx = *cc;
					cc++;
					ty = *cc;
					cc++;
					ct = (int*)cc;
					for (auto& i : players) {
						if (i.ID == *ct) {
							i.x = tx;
							i.y = ty;
						}
						sendto(in, buf, 128, 0, (sockaddr*)&i.addr, sizeof(i.addr));
					}
					break;
				case CONNECT:
					//if this gets dropped client is screwed
					for (auto& i : addresses) {
						if (client.sin_addr.S_un.S_addr == i.sin_addr.S_un.S_addr && client.sin_port == i.sin_port) {
							push = false;
						}
					}
					if (push) {
						char* tol;
						float* t;
						int* pt;
						addresses.push_back(client);
						//set players addr to current one based on the id we recieve from that address
						tol = buf;
						tol++;
						t = (float*)tol;
						t++;
						t++;
						pt = (int*)t;
						for (auto& i : players) {
							if (i.ID == *pt) {
								i.addr = client;
							}
						}
						std::cout << "New address added\n";
					}
					ZeroMemory(buf, 128);
					buf[0] = CONNECT;
					sendto(in, buf, 128, 0, (sockaddr*)&client, clientLen);
					break;
				}
			}
			mt1.unlock();
		}
		//checking for player death
		mt1.lock();
		for (int j = 0; j < players.size(); j++) {
			if (players[j].health <= 0) {
				ZeroMemory(buf, 128);
				buf[0] = DEATH;
				char* tp = buf;
				tp++;
				int* t = (int*)tp;
				*t = players[j].ID;
				t++;
				float* tow = (float*)t;
				*tow = players[j].sx;
				tow++;
				*tow = players[j].sy;
				for (int j = 0; j < master.fd_count; j++) {
					send(master.fd_array[j], buf, 128, 0);
				}
				players[j].health = 100;
			}
		}
		mt1.unlock();

		//bullet updates
		if (btime.getTime() > 5) {
			for (int i = 0; i < bullets.size();) {
				bullets[i].x += bullets[i].trajx;
				bullets[i].y += bullets[i].trajy;
				bool er = false;
				ZeroMemory(buf, 128);
				mt1.lock();
				for (int j = 0; j < players.size(); j++) {
					if (isColliding(bullets[i], players[j]) && players[j].ID != bullets[i].PID) {
						players[j].health -= 5;
						//send pack to damage player
						ZeroMemory(buf, 128);
						char* tp = buf;
						buf[0] = HEALTH;
						tp++;
						int* t = (int*)tp;
						*t = players[j].health;
						sendto(in, buf, 128, 0, (sockaddr*)&players[j].addr, sizeof(players[j].addr));
						Entity* pt = nullptr;
						for (auto k : players) {
							if (bullets[i].PID == k.ID) {
								k.score++;
								pt = &k;
							}
						}
						ZeroMemory(buf, 128);
						buf[0] = SCORE;
						tp = buf;
						tp++;
						t = (int*)tp;
						*t = bullets[i].PID;
						t++;
						*t = pt->score;
						for (int k = 0; k < master.fd_count; k++) {
							send(master.fd_array[k], buf, 128, 0);
						}
						er = true;
						j = players.size();
					}
				}
				mt1.unlock();
				if (bullets[i].x < 0 || bullets[i].y < 0 || bullets[i].x > 1500 || bullets[i].y > 1500) {
					er = true;
				}
				if (er) {
					ZeroMemory(buf, 128);
					char* tt = buf;
					buf[0] = RMVBULLET;
					tt++;
					int* tp = (int*)tt;
					*tp = bullets[i].ID;
					mt1.lock();
					for (int j = 0; j < master.fd_count; j++) {
						send(master.fd_array[j], buf, 128, 0);
					}
					mt1.unlock();
					bullets.erase(bullets.begin() + i);
				}
				else {
					ZeroMemory(buf, 128);
					char* tt = buf;
					buf[0] = BPOSITION;
					tt++;
					int* tp = (int*)tt;
					*tp = bullets[i].ID;
					tp++;
					float* t = (float*)tp;
					*t = bullets[i].x;
					t++;
					*t = bullets[i].y;
					t++;
					*t = bullets[i].trajx;
					t++;
					*t = bullets[i].trajy;
					i++;
					mt1.lock();
					for (auto& j : addresses) {
						sendto(in, buf, 128, 0, (sockaddr*)&j, sizeof(j));
					}
					mt1.unlock();
				}
			}
			btime.resetTime();
		}
	}
	//close tcp listener
	
	//close udp listener
	closesocket(in);
	//cleanup things
	WSACleanup();
	return 0;
}
