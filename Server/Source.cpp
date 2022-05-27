#include <iostream>
#include <asio.hpp>
#include <thread>
#include <random>
#include <time.h>
#include <math.h>
#undef main
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
struct END {
	std::string recv;
	std::string send;
};

//bulletpos and playerpos both large packets containting pos data for all bullets and players
enum {NEWPLAYER = 24, NEWBULLET, BULLETPOS, PLAYERPOS};

bool exitf = false;
std::vector<Player> players;
std::vector<Bullet> bullets;
std::vector<std::string> endpoints;
asio::io_context io;

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
	//asio::io_context io;
	while (!exitf) {
		asio::ip::tcp::acceptor accept(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 6890));
		asio::error_code ec;
		asio::ip::tcp::socket soc(io);
		accept.listen();
		accept.accept(soc, ec);
		std::cerr << ec.message() << std::endl;
		asio::ip::tcp::endpoint remote_ep = soc.remote_endpoint();
		asio::ip::address remote_ad = remote_ep.address();
		std::string ipstring = remote_ad.to_string();
		std::cout << "New connection from " << remote_ad.to_string() << "\n";
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
				outbuf[i] = -1;
			}
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
				outbuf[i] = -1;
			}
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
		char indexbuf[4];
		int* pit = (int*)indexbuf;
		*pit = p.index;
		soc.send(asio::buffer(indexbuf));
		/*asio::ip::udp::endpoint(remote_ad, 6892);
		asio::ip::udp::endpoint(remote_ad, 6891);
		en.recv = 
		en.send =*/
		endpoints.push_back(ipstring);
		std::cout << endpoints.size() << std::endl;
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
void writePlayers(asio::ip::udp::socket* soc, std::string ip) {
	char buf[121];
	buf[0] = PLAYERPOS;
	asio::ip::udp::endpoint en1(asio::ip::address::from_string(ip), 6892);
	asio::error_code er1;
	//soc->bind(en1);
	for (int i = 0; i < players.size(); i++) {
		char* m = buf;
		m++;
		for (int j = 1; j < 121; j+=12) {
			int* ind = (int*)m;
			*ind = players[i].index;
			ind++;
			float* pos = (float*)ind;
			*pos = players[i].x;
			pos++;
			*pos = players[i].y;
			pos++;
			m += 12;
			i++;
			if (i >= players.size()) { 
				*m = -1; 
				//soc->send_to(asio::buffer(buf), en1, 0, er1); 
				break; 
			}
		}
		std::cerr << er1.message() << "\n";
		soc->send_to(asio::buffer(buf), en1, 0, er1);
		std::cerr << er1.message() << "\n";
		std::cout << "Wrote data to " << ip << "\n";
	}
}
void recieveData(asio::ip::udp::socket* soc, std::string ip) {
	char buf[129];
	asio::ip::udp::endpoint end(asio::ip::address::from_string(ip), 6891);
	asio::error_code ec;
	soc->receive(asio::buffer(buf), 0, ec);
	std::cerr << ec.message() << "\n";
	std::cout << "Recieved data from " << ip << "\n";
	char* m = buf;
	int* mp;
	float* mf;
	m++;
	switch (m[0]) {
	case PLAYERPOS:
		mp = (int*)m;
		mf = (float*)m;
		mf++;
		players[*mp].x = *mf;
		mf++;
		players[*mp].y = *mf;
		break;
	}
}

int main() {
	srand(time(NULL));
	//doing this to avoid moving of memory block
	players.reserve(100);
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
	endpoints.reserve(100);
	std::thread listener(connectListener);
	asio::ip::udp::socket recv(io);
	asio::ip::udp::socket sendv(io);
	recv.open(asio::ip::udp::v4());
	asio::ip::udp::endpoint en1(asio::ip::udp::v4(), 6892);
	recv.bind(en1);

	sendv.open(asio::ip::udp::v4());
	asio::ip::udp::endpoint en2(asio::ip::udp::v4(), 7000);
	sendv.bind(en2);
	//std::thread recieve(recieveData, &recvsoc);
	//thread to write data
	while (!exitf) {
		//write data needed for each socket
		//std::cout << endpoints.size() << "\n";
		for (auto& i : endpoints) {
			writePlayers(&sendv, i);
			recieveData(&recv, i);
		}

	}
	listener.join();
	//recieve.join();
}