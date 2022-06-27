#include "Header.h"
#undef main


//need to add basic bullet functionality, so creation of a bullet and then the server will keep track of and send the data to the user
//add bullet collision on the server side, so players can die
//add client side prediction

int main() {
	//start winsock
	WSADATA wdata;
	WORD version = MAKEWORD(2, 2);
	int wsOK = WSAStartup(version, &wdata);
	if (wsOK != 0) { std::cout << "can't start winsock! " << wsOK << std::endl; return -1; }


	sockaddr_in server;
	SOCKET udpSock;
	

	SOCKET tcpSock;
	
	Entity player = { 400, 400, 25, 25, 0 };
	connectToServer(&udpSock, &tcpSock, &server, &player);
	bool exitf = false;
	//init SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) > 0) {
		std::cerr << SDL_GetErrorMsg << std::endl;
	}
	SDL_Window* wind = SDL_CreateWindow("Net Shmup", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, 0);
	SDL_Renderer* rend = SDL_CreateRenderer(wind, -1, SDL_RENDERER_ACCELERATED);
	SDL_Event e;

	const Uint8* keys;
	keys = SDL_GetKeyboardState(NULL);
	Gore::DeltaTimer dt;

	std::thread udpRcvThread(udpRcv, udpSock, server, player.ID);
	//main loop
	char buf[128];
	while (!exitf) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				exitf = true;
				break;
			}
		}
		double delta = dt.getDelta();
		bool sendLoc = false;
		SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
		SDL_RenderClear(rend);
		if (keys[SDL_SCANCODE_W]) {
			player.y -= 150 * delta;
			sendLoc = true;
		}else if (keys[SDL_SCANCODE_S]) {
			player.y += 150 * delta;
			sendLoc = true;
		}
		if (keys[SDL_SCANCODE_A]) {
			player.x -= 150 * delta;
			sendLoc = true;
		}else if (keys[SDL_SCANCODE_D]) {
			player.x += 150 * delta;
			sendLoc = true;
		}
		if (sendLoc) {
			ZeroMemory(buf, 128);
			buf[0] = PPOSITION;
			char* tt = buf;
			tt++;
			float* tpo = (float*)tt;
			*tpo = player.x;
			tpo++;
			*tpo = player.y;
			tpo++;
			int* idn = (int*)tpo;
			*idn = player.ID;
			sendto(udpSock, buf, 128, 0, (sockaddr*)&server, sizeof(server));
		}
		SDL_SetRenderDrawColor(rend, 255, 25, 50, 255);
		for (int i = 0; i < players.size(); i++) {
			SDL_Rect rect = { players[i].x, players[i].y, players[i].w, players[i].h };
			SDL_RenderFillRect(rend, &rect);
		}

		SDL_SetRenderDrawColor(rend, 50, 255, 100, 255);
		SDL_Rect prect = { player.x, player.y, player.w, player.h };
		SDL_RenderFillRect(rend, &prect);


		SDL_RenderPresent(rend);
	}
	//close sockets
	closesocket(udpSock);
	closesocket(tcpSock);
	udpRcvThread.join();
	WSACleanup();
	return 0;
}