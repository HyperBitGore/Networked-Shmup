#include <iostream>
#include <asio.hpp>
#include <thread>
#include <random>
#include <time.h>
#include <math.h>
struct Entity {
	float x;
	float y;
	UINT32 w;
	UINT32 h;
};

struct Player : Entity {
	UINT32 index;
	UINT32 health;
};

struct Bullet : Entity {
	float trajx;
	float trajy;
	//index of player it was fired from
	UINT32 index;
};
//bulletpos and playerpos both large packets containting pos data for all bullets and players
enum {NEWPLAYER = 24, NEWBULLET, BULLETPOS, PLAYERPOS};

bool exitf = false;
asio::io_context io;
std::vector<Player> players;
std::vector<Bullet> bullets;
std::vector<asio::ip::udp::socket*> udpsockets;

char* serilizeStruct(char* ptr, int size) {
	char* mt = (char*)std::malloc(size);
	for (int i = 0; i < size; i++) {
		mt[i] = *ptr;
		ptr++;
	}
	return mt;
}
void deserilizeStruct(char* dest, char* data, int size) {
	for (int i = 0; i < size; i++) {
		*dest = data[i];
		dest++;
	}
}



//listens to connections and adds to socket list
void connectListener() {
	while (!exitf) {
		asio::ip::tcp::acceptor accept(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 6890));
		asio::error_code ec;
		asio::ip::tcp::socket soc(io);
		accept.listen();
		accept.accept(soc, ec);
		asio::ip::tcp::endpoint remote_ep = soc.remote_endpoint();
		asio::ip::address remote_ad = remote_ep.address();
		std::string ipstring = remote_ad.to_string();
		//write entire player list
		char* buf;
		if (players.size() > 0) {
			char outbuf[24];
			for (auto& i : players) {
				buf = serilizeStruct((char*)&i, 24);
				for (int j = 0; j < 24; j++) {
					outbuf[j] = buf[j];
				}
				std::cout << "Writing player to " << ipstring << "\n";
				std::cout << "Writing: " << outbuf << "\n";
				soc.send(asio::buffer(outbuf));

			}
			outbuf[0] = -1;
			soc.send(asio::buffer(outbuf));
		}
		else {
			char outbuf[24];
			for (int i = 0; i < 24; i++) {
				outbuf[i] = 0;
			}
			outbuf[0] = -1;
			soc.send(asio::buffer(outbuf));
		}
		//write entire bullet list
		if (bullets.size() > 0) {
			char outbuf[28];
			for (auto& i : bullets) {
				buf = serilizeStruct((char*)&i, 28);
				for (int j = 0; j < 28; j++) {
					outbuf[j] = buf[j];
				}
				std::cout << "Writing bullet to " << ipstring << "\n";
				std::cout << "Writing: " << outbuf << "\n";
				soc.send(asio::buffer(outbuf));

			}
			outbuf[0] = -1;
			soc.send(asio::buffer(outbuf));
		}
		else {
			char outbuf[28];
			for (int i = 0; i < 28; i++) {
				outbuf[i] = 0;
			}
			outbuf[0] = -1;
			soc.send(asio::buffer(outbuf));
		}
		Player p;
		p.health = 100;
		p.h = 50;
		p.w = 50;
		p.x = 400;
		p.y = 400;
		p.index = players.size();
		players.push_back(p);
		//get udp socket
		asio::ip::udp::socket* udpsoc = new asio::ip::udp::socket(io);
		asio::ip::udp::endpoint udpaddr(asio::ip::address::from_string(ipstring), 6891);
		udpsoc->connect(udpaddr);
		udpsockets.push_back(udpsoc);
		soc.close();
	}
}

void writenewPlayer() {
	
}
void writenewBullet() {
	
}
void writeBullets() {
	//write a bunch of small packets, will be more effiecent
	//char* output = std::malloc()
	for (auto& i : bullets) {

	}
}
void writePlayers() {
	
}

int main() {
	srand(time(NULL));
	//doing this to avoid moving of memory block
	players.reserve(100);
	/*Player p1;
	p1.w = 50;
	p1.h = 50;
	p1.health = 100;
	//get these from server
	p1.x = 400;
	p1.y = 400;
	p1.index = 0;
	players.push_back(p1);
	Player p2;
	p2.w = 50;
	p2.h = 50;
	p2.health = 89;
	//get these from server
	p2.x = 550;
	p2.y = 320;
	p2.index = 1;
	players.push_back(p2);*/
	bullets.reserve(1000);
	/*Bullet b1;
	b1.w = 10;
	b1.h = 10;
	b1.x = 400;
	b1.y = 400;
	b1.index = 0;
	b1.trajx = 0.3;
	b1.trajy = 0.4;
	bullets.push_back(b1);
	Bullet b2;
	b2.w = 10;
	b2.h = 10;
	b2.x = 500;
	b2.y = 200;
	b2.index = 1;
	b2.trajx = 0.2;
	b2.trajy = 0.1;
	bullets.push_back(b2);*/
	udpsockets.reserve(100);

	std::thread listener(connectListener);
	while (!exitf) {
		for (int i = 0; i < udpsockets.size(); i++) {
			//write data needed for each socket
		}

	}
	listener.join();
}