#include "Header.h"
#undef main
bool exitf = false;
Gore gore;
asio::io_context io;
asio::ip::udp::socket udpsock(io);

//break off data recieving thread
//recieve players positions
//recieve bullets positions
//recieve player deaths
//recieve bullets deaths
//add camera

void recieveData() {
	while (!exitf) {
		//receive data from server
	}
}

int main() {
	if (SDL_Init(SDL_INIT_EVERYTHING) > 0) {
		std::cout << "SDL Init failed: " << SDL_GetError << std::endl;
	}
	if (TTF_Init()) {
		std::cout << "TTF failed to init" << TTF_GetError << std::endl;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 4, 2048) < 0) {
		std::cout << "Mix failed to init" << Mix_GetError << std::endl;
	}
	SDL_Window* window = SDL_CreateWindow("Networked Shmup", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, 0);
	
	Player p1;
	p1.w = 50;
	p1.h = 50;
	p1.health = 100;
	//get these from server
	p1.x = 400;
	p1.y = 400;
	p1.index = 0;
	//will be populated from server connect event
	//players will contain every other player besides local one
	std::vector<Player> players;
	std::vector<Bullet> bullets;
	std::vector<Entity> walls;
	//vector of timers that corresponds to bullets by index
	std::vector<TimerObject> btimers;
	

	//loop variables
	double delta;
	SDL_Event e;
	const Uint8* keys;
	keys = SDL_GetKeyboardState(NULL);

	std::string ip;
	std::cout << "Input server IP ";
	std::cin >> ip;
	std::cout << std::endl;

	std::cout << sizeof(Bullet) << std::endl;
	char* serilizeplayer = gore.serilizeStruct((char*)&p1, sizeof(Player));
	Player p2;
	gore.deserilizeStruct((char*)&p2, serilizeplayer, 24);
	free(serilizeplayer);
	Game::connectToServer(ip, players, bullets);

	std::thread datathread(recieveData);
	while (!exitf) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				exitf = true;
				break;
			}
		}
		delta = gore.getDelta();
		SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
		SDL_RenderClear(rend);
		//local player input
		if (keys[SDL_SCANCODE_W]) {
			p1.y -= 150 * delta;
		}
		else if (keys[SDL_SCANCODE_S]) {
			p1.y += 150 * delta;
		}
		if (keys[SDL_SCANCODE_D]) {
			p1.x += 200 * delta;
		}
		else if (keys[SDL_SCANCODE_A]) {
			p1.x -= 200 * delta;
		}

		//rendering the players
		SDL_SetRenderDrawColor(rend, 255, 50, 50, 0);
		for (int i = 0; i < players.size();) {
			bool erase = false;

			SDL_Rect rect = { players[i].x, players[i].y, players[i].w, players[i].h };
			SDL_RenderFillRect(rend, &rect);
			if (erase) {
				players.erase(players.begin() + i);
			}
			else {
				i++;
			}
		}
		SDL_SetRenderDrawColor(rend, 0, 50, 255, 0);
		for (int i = 0; i < bullets.size(); ) {
			bool erase = false;
			//btimers[i].time += (float)delta;
			//if (btimers[i].time >= btimers[i].maxtime) {
				//bullets[i].x += bullets[i].trajx;
				//bullets[i].y += bullets[i].trajy;
			//}
			for (int j = 0; j < players.size(); j++) {
				if (j != bullets[i].index && Game::isColliding(&bullets[i], &players[j])) {
					//damage that player and send packet to server, to damage player and delete bullet
					//erase = true;
				}
			}
			SDL_Rect rect = { bullets[i].x, bullets[i].y, bullets[i].w, bullets[i].h };
			SDL_RenderFillRect(rend, &rect);
			if (erase) {
				bullets.erase(bullets.begin() + i);
				//btimers.erase(btimers.begin() + i);
			}
			else {
				i++;
			}
		}
		SDL_SetRenderDrawColor(rend, 50, 255, 50, 0);
		SDL_Rect prect = { p1.x, p1.y, p1.w, p1.h };
		SDL_RenderFillRect(rend, &prect);
		SDL_RenderPresent(rend);
	}
	datathread.join();
	return 0;
}