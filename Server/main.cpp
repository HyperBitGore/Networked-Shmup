#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include <time.h>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

struct Entity {
	float x;
	float y;
	int ID;
	sockaddr_in addr;
};


std::vector<sockaddr_in> addresses;
std::vector<Entity> players;


enum { PPOSITION = 28, BPOSITION, DISCONNECT, CONNECT};

void tcpThread() {
	fd_set master;
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
		fd_set copy = master;
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
				*tt = rand() % 750 + 1;
				p.x = *tt;
				tt++;
				*tt = rand() % 750 + 1;
				p.y = *tt;
				tt++;
				int* ttp = (int*)tt;
				*ttp = players.size();
				p.ID = *ttp;
				p.addr.sin_port = 0;
				players.push_back(p);
				std::cout << players.size() << std::endl;
				send(clientSocket, buf, 128, 0);

				FD_SET(clientSocket, &master);
			}
			else {
				//recv data from socket
				
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
	char buf[128];
	//break off seperate thread for tcp listening
	std::thread tcpT(tcpThread);
	while (!exitf) {
		ZeroMemory(buf, 128);
		bool push = true;
		int bytesIn = recvfrom(in, buf, 128, 0, (sockaddr*)&client, &clientLen);
		if (bytesIn == SOCKET_ERROR) {
			std::cout << "Error recv from client " << WSAGetLastError() << std::endl;
		}
		else {
			//read data sent and then send back out
			char clientIp[256];
			ZeroMemory(clientIp, 256);
			inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
			
			switch (buf[0]) {
			case PPOSITION:
				for (auto& i : addresses) {
					sendto(in, buf, 128, 0, (sockaddr*)&i, sizeof(i));
				}
				break;
			case CONNECT:
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
				break;
			case DISCONNECT:

				break;
			}
		}

	}
	//close tcp listener
	
	//close udp listener
	closesocket(in);
	//cleanup things
	WSACleanup();
	return 0;
}
