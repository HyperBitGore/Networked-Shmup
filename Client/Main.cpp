#include "Header.h"
#undef main
bool exitf = false;
Gore gore;
std::string ip;
int playerindex = 0;
//will be populated from server connect event
//players will contain every other player besides local one
std::vector<Player> players;
std::vector<Bullet> bullets;
std::vector<Entity> walls;

//recvsoc invalid argument supplied might be from running server and client on same pc, test hosting on another pc; https://marc.info/?l=boost-users&m=122746978108299
//fix player index not being sent on connect
//fix server getting hung on recieve
//recieve players positions
//recieve bullets positions
//recieve player deaths
//recieve bullets deaths
//add camera
//walls and collision

void recieveData(std::string curip) {
	char buf[121];
	asio::io_context io;
	//asio::ip::udp::endpoint endpoi(asio::ip::address::from_string(curip), 6892);
	asio::ip::udp::socket recvsoc(io);
	asio::error_code ec1;
	recvsoc.open(asio::ip::udp::v4(), ec1);
	std::cerr << ec1.message() << std::endl;
	//asio::ip::udp::endpoint en1(asio::ip::udp::v4(), 6892);
	//recvsoc.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 6892), ec1);
	recvsoc.bind(asio::ip::udp::endpoint(asio::ip::address::from_string(curip), 6892));
	//recvsoc.connect(endpoi, ec1);
	std::cerr << ec1.message() << std::endl;
	bool exitl = false;
	while (!exitl) {
		//receive data from server
		recvsoc.receive(asio::buffer(buf), 0, ec1);
		std::cerr << ec1.message() << std::endl;
		char* m = buf;
		m++;
		int* mp = (int*)m;
		float* mf;
		switch (buf[0]) {
		case NEWPLAYER:
				
			break;
		case NEWBULLET:
			
				break;
		case BULLETPOS:
			
				break;
		case PLAYERPOS:
			for (int i = 1; i < 121 && buf[i] != -1; i += 12) {
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
	}
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
	asio::io_context io;
	Game::connectToServer(ip, players, bullets, &p1);
	
	asio::ip::udp::endpoint sendip(asio::ip::address::from_string(ip), 6891);
	
	asio::ip::udp::socket sendsoc(io);
	asio::error_code ignore1;
	sendsoc.open(asio::ip::udp::v4(), ignore1);
	sendsoc.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 6891), ignore1);
	//sendsoc.connect(sendip);
	std::cerr << ignore1.message() << std::endl;
	std::thread datathread(recieveData, ip);

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
	
	//recieveData(ip);
	//loop variables
	double delta;
	SDL_Event e;
	const Uint8* keys;
	keys = SDL_GetKeyboardState(NULL);
	char movebuf[129];
	char shootbuf[20];
	//testing vars
	p1.index = 0;
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
			m += 9;
			*m = -1;
			sendsoc.send_to(asio::buffer(movebuf), sendip);

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