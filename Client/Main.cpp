#include "Header.h"
#undef main
bool exitf = false;
Gore gore;
std::string ip;
IPaddress sendip;
int playerindex = 0;

//will be populated from server connect event
//players will contain every other player besides local one
std::vector<Player> players;
std::vector<Bullet> bullets;
std::vector<Entity> walls;

//recieve players positions
//recieve bullets positions
//recieve player deaths
//recieve bullets deaths
//add camera
//walls and collision

void recieveData() {
	char buf[121];
	UDPsocket recvsoc = SDLNet_UDP_Open(0);
	bool exitl = false;
	while (!exitl) {
		//receive data from server
		UDPpacket pack;
		SDLNet_UDP_Recv(recvsoc, &pack);
		if (pack.len > 0) {
			char* m = buf;
			char* mv = m;
			for (int i = 0; i < pack.len; i++) {
				*mv = pack.data[i];
				mv++;
			}
			mv = m;
			mv++;
			int* mp = (int*)m;
			float* mf;
			switch (m[0]) {
			case NEWPLAYER:

				break;
			case NEWBULLET:

				break;
			case BULLETPOS:

				break;
			case PLAYERPOS:
				for (int i = 1; i < 121 && mv[i] != -1; i += 12) {
					if (*mp != playerindex) {
						mf = (float*)mp;
						mf++;
						players[*mp].x = *mf;
						mf++;
						players[*mp].y = *mf;
					}
					else {
						*mp += 3;
					}
				}
				break;
			}
			free(m);
		}
	}
}

void writeData(char* buf, int size) {
	UDPsocket sendsoc = SDLNet_UDP_Open(0);
	UDPpacket* pack = SDLNet_AllocPacket(size);
	pack->address = sendip;
	pack->data = (Uint8*)buf;
	pack->channel = -1;
	pack->maxlen = size;
	SDLNet_UDP_Send(sendsoc, -1, pack);
	SDLNet_FreePacket(pack);
}

int main() {
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO) > 0) {
		std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
	}
	if (TTF_Init() == -1) {
		std::cout << "TTF failed to init" << TTF_GetError() << std::endl;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 4, 2048) < 0) {
		std::cout << "Mix failed to init" << Mix_GetError() << std::endl;
	}
	if (SDLNet_Init() == 1) {
		std::cout << "SDL failed to intit " << SDLNet_GetError() << std::endl;
	}
	Player p1;
	p1.w = 50;
	p1.h = 50;
	p1.health = 100;
	//get these from server
	p1.x = 400;
	p1.y = 400;
	p1.index = 0;

	//server connection
	std::string ip;
	std::cout << "Input server IP ";
	std::cin >> ip;
	std::cout << std::endl;

	Game::connectToServer(ip, players, bullets, &p1);
	SDLNet_ResolveHost(&sendip, ip.c_str(), 6892);

	SDL_Window* window;
	SDL_Renderer* rend;
	try {
		window = SDL_CreateWindow("Networked Shmup", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
		std::cout << SDL_GetError() << std::endl;
	}
	catch (std::exception& epp) {
		std::cout << epp.what() << std::endl;
	}
	try {
		rend = SDL_CreateRenderer(window, -1, 0);
		std::cout << SDL_GetError() << std::endl;
	}
	catch (std::exception& epp) {
		std::cout << epp.what() << std::endl;
	}
	//vector of timers that corresponds to bullets by index
	std::vector<TimerObject> btimers;
	
	std::thread datathread(recieveData);
	//loop variables
	double delta;
	SDL_Event e;
	const Uint8* keys;
	keys = SDL_GetKeyboardState(NULL);
	char movebuf[13];
	char shootbuf[20];
	while (!exitf) {
		try {
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
			movebuf[0] = PLAYERPOS;
			char* m = movebuf;
			m++;
			int* mp = (int*)m;
			*mp = p1.index;
			mp++;
			float* mf = (float*)mp;
			*mf = p1.x;
			mf++;
			*mf = p1.y;
			writeData(movebuf, 13);
			//std::thread write_dat(writeData, movebuf);
			//write_dat.detach();

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
		catch (std::exception& epp) {
			std::cout << "exception: " << epp.what() << "\n";
		}
	}
	datathread.join();
	return 0;
}