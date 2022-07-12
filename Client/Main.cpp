#include "Header.h"
#undef main


//maybe change connection to not use UDP for finding players


void generateWalls(std::vector<Entity>& output) {
	std::vector<int> walls = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	float sx = 0.0f;
	float sy = 0.0f;
	for (int i = 0; i < walls.size();) {
		for (int j = 0; j < 30 && i < walls.size(); j++) {
			if (walls[i] == 1) {
				Entity w;
				w.x = sx;
				w.y = sy;
				w.w = 50;
				w.h = 50;
				w.ID = walls.size();
				w.er = false;
				output.push_back(w);
			}
			sx += 50.0f;
			i++;
		}
		sx = 0.0f;
		sy += 50.0f;
	}
}


int main() {
	//start winsock
	WSADATA wdata;
	WORD version = MAKEWORD(2, 2);
	int wsOK = WSAStartup(version, &wdata);
	if (wsOK != 0) { std::cout << "can't start winsock! " << wsOK << std::endl; return -1; }


	sockaddr_in server;
	SOCKET udpSock;
	

	SOCKET tcpSock;
	
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

	std::vector<Entity> walls;
	generateWalls(walls);
	SDL_Rect camera = { 0, 0, 800, 800 };
	
	std::thread udpRcvThread(udpRcv, udpSock, server, player.ID);
	std::thread tcpRcvThread(tcpRcv, tcpSock);
	//main loop
	char buf[128];
	double shootcool = 0;
	while (!exitf) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				exitf = true;
				break;
			}
		}
		camera.x = player.x - 400;
		camera.y = player.y - 400;
		double delta = dt.getDelta();
		shootcool += delta;
		SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
		SDL_RenderClear(rend);
		if (keys[SDL_SCANCODE_W]) {
			player.y -= 150 * delta;
			for(auto& i : walls){
				if (isColliding(player, i)) {
					player.y += 150 * delta;
					break;
				}
			}
		}
		else if (keys[SDL_SCANCODE_S]) {
			player.y += 150 * delta;
			for (auto& i : walls) {
				if (isColliding(player, i)) {
					player.y -= 150 * delta;
					break;
				}
			}
		}
		if (keys[SDL_SCANCODE_A]) {
			player.x -= 150 * delta;
			for (auto& i : walls) {
				if (isColliding(player, i)) {
					player.x += 150 * delta;
					break;
				}
			}
		}
		else if (keys[SDL_SCANCODE_D]) {
			player.x += 150 * delta;
			for (auto& i : walls) {
				if (isColliding(player, i)) {
					player.x -= 150 * delta;
					break;
				}
			}
		}
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
		int mx, my;
		if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT) && shootcool > 0.005) {
			//send bullet creation data to server
			buf[0] = NEWBULLET;
			char* tt = buf;
			tt++;
			float* t = (float*)tt;
			*t = player.x;
			t++;
			*t = player.y;
			t++;
			//calculate trajectory now
			float dx, dy;
			calcSlope(player.x, player.y, mx + camera.x, my + camera.y, &dx, &dy);
			*t = -dx;
			t++;
			*t = -dy;
			t++;
			int* top = (int*)t;
			*top = player.ID;
			send(tcpSock, buf, 128, 0);
			shootcool = 0;
		}
		SDL_SetRenderDrawColor(rend, 75, 25, 255, 255);
		for (auto& i : walls) {
			SDL_Rect rect = { i.x - camera.x, i.y - camera.y, i.w, i.h };
			SDL_RenderFillRect(rend, &rect);
		}
		SDL_SetRenderDrawColor(rend, 255, 25, 50, 255);
		bt1.lock();
		for (int i = 0; i < players.size(); i++) {
			SDL_Rect rect = { players[i].x - camera.x, players[i].y - camera.y, players[i].w, players[i].h };
			SDL_RenderFillRect(rend, &rect);
		}
		for (int i = 0; i < bullets.size(); i++) {
			bullets[i].mvtime += delta;
			if (bullets[i].mvtime > 0.01) {
				bullets[i].x += bullets[i].trajx;
				bullets[i].y += bullets[i].trajy;
				bullets[i].mvtime = 0;
			}
			SDL_Rect rect = { bullets[i].x - camera.x, bullets[i].y - camera.y, bullets[i].w, bullets[i].h };
			SDL_RenderFillRect(rend, &rect);
		}
		bt1.unlock();
		SDL_SetRenderDrawColor(rend, 255, 55, 75, 255);
		SDL_Rect hrect = { 0, 0, player.health * 3, 35 };
		SDL_RenderFillRect(rend, &hrect);

		SDL_SetRenderDrawColor(rend, 50, 255, 100, 255);
		SDL_Rect prect = { player.x - camera.x, player.y - camera.y, player.w, player.h };
		SDL_RenderFillRect(rend, &prect);


		SDL_RenderPresent(rend);
	}
	//send disconnect
	ZeroMemory(buf, 128);
	buf[0] = DISCONNECT;
	char* tt = buf;
	tt++;
	int* tpot = (int*)tt;
	*tpot = player.ID;
	send(tcpSock, buf, 128, 0);
	//close sockets
	closesocket(udpSock);
	closesocket(tcpSock);
	udpRcvThread.join();
	tcpRcvThread.join();
	WSACleanup();
	return 0;
}