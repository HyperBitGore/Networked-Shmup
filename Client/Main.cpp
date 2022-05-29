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

//fix server stack corruption
//Fix server crash on new player join
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
		//std::cerr << ec1.message() << std::endl;
		char* m = buf;
		m++;
		int* mp = (int*)m;
		float* mf;
		switch (buf[0]) {
		case BULLETPOS:
			for (int i = 1; i < 121 && buf[i] != -52; i += 12) {
				if (*mp < bullets.size()) {
					mf = (float*)mp;
					mf++;
					bullets[*mp].x = *mf;
					mf++;
					bullets[*mp].y = *mf;
					mp += 3;
				}
				else {
					*mp += 3;
				}
				
			}
				break;
		case PLAYERPOS:
			for (int i = 1; i < 121 && buf[i] != -52; i += 12) {
				if (*mp != playerindex && *mp < players.size()) {
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
void tcpRECV(asio::ip::tcp::socket* soc) {
	char buf[121];
	while (!exitf) {
		soc->receive(asio::buffer(buf));
		Player outp;
		Bullet outb;
		switch (buf[120]) {
		case NEWPLAYER:
			gore.deserilizeStruct((char*)&outp, buf, 24);
			players.push_back(outp);
			break;
		case NEWBULLET:
			gore.deserilizeStruct((char*)&outb, buf, 28);
			bullets.push_back(outb);
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

	players.reserve(100);
	bullets.reserve(1000);
	walls.reserve(100);

	//server connection
	std::string ip;
	std::cout << "Input server IP ";
	std::cin >> ip;
	std::cout << std::endl;
	asio::io_context io;
	Game::connectToServer(ip, players, bullets, &p1);
	//tcp socket used for new bullets and new players added
	asio::ip::tcp::socket tcprecv(io);
	asio::ip::tcp::endpoint tcpendp(asio::ip::address::from_string(ip), 6890);
	//tcp socket is automatically opened and we don't have to bind like a udp listener
	tcprecv.connect(tcpendp);

	//udp socket used for write operations
	asio::ip::udp::endpoint sendip(asio::ip::address::from_string(ip), 6891);
	asio::ip::udp::socket sendsoc(io);
	asio::error_code ignore1;
	sendsoc.open(asio::ip::udp::v4(), ignore1);
	//sendsoc.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 6891), ignore1);
	//sendsoc.connect(sendip);
	std::cerr << ignore1.message() << std::endl;
	std::thread datathread(recieveData, ip);
	std::thread tcpthread(tcpRECV, &tcprecv);

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
	
	//recieveData(ip);
	//loop variables
	double delta = 0;
	double shootimer = 0;
	SDL_Event e;
	const Uint8* keys;
	keys = SDL_GetKeyboardState(NULL);
	char movebuf[129];
	char shootbuf[121];
	int mx, my;
	//testing vars
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
			shootimer += delta;
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
			if (shootimer > 0.1) {
				if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					std::cout << "Shot\n";
					//send bullet creation packet to server, maybe break off thread or have a thread pool do this
					shootbuf[120] = NEWBULLET;
					Bullet b;
					b.h = 10;
					b.w = 10;
					b.x = p1.x + 25;
					b.y = p1.y + 25;
					b.index = bullets.size();
					Game::calcSlope(p1.x, p1.y, mx, my, &b.trajx, &b.trajy);
					char* data = (char*)&b;
					gore.serilizeStruct(data, shootbuf, 28);
					tcprecv.send(asio::buffer(shootbuf));
					shootimer = 0;
				}
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
			*m = -52;
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
				//move this to server
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
	shootbuf[120] = CLOSE;
	tcprecv.send(asio::buffer(shootbuf));
	tcprecv.close();
	datathread.join();
	tcpthread.join();
	return 0;
}