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
struct END {
	asio::ip::udp::endpoint recv;
	asio::ip::udp::endpoint send;
};

//bulletpos and playerpos both large packets containting pos data for all bullets and players
enum {NEWPLAYER = 24, NEWBULLET, BULLETPOS, PLAYERPOS};

bool exitf = false;
asio::io_context io;
std::vector<Player> players;
std::vector<Bullet> bullets;
std::vector<END> endpoints;

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
		soc.close();
		END en;
		en.recv = asio::ip::udp::endpoint(asio::ip::address::from_string(ipstring), 6891);
		en.send = asio::ip::udp::endpoint(asio::ip::address::from_string(ipstring), 6892);
		endpoints.push_back(en);
		//get udp socket
		/*asio::ip::udp::endpoint udpaddr(asio::ip::address::from_string(ipstring), 6891);
		asio::ip::udp::socket* udpsoc = new asio::ip::udp::socket(io, udpaddr);
		udpsoc->open(asio::ip::udp::v4());
		//udpsoc->connect(udpaddr);
		asio::ip::udp::endpoint udpaddr2(asio::ip::address::from_string(ipstring), 6892);
		asio::ip::udp::socket* udpsoc2 = new asio::ip::udp::socket(io, udpaddr2);
		udpsoc2->open(asio::ip::udp::v4());
		//udpsoc2->connect(udpaddr2);
		NetworkObject n;
		n.recieve = udpsoc;
		n.index = p.index;
		n.send = udpsoc2;
		udpsockets.push_back(n);*/
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
void writePlayers(asio::ip::udp::socket* soc, asio::ip::udp::endpoint *end) {
	char buf[121];
	buf[0] = PLAYERPOS;
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
			if (i >= players.size() && i != 0) { *m = -1; soc->send(asio::buffer(buf)); break; }
		}
		soc->send_to(asio::buffer(buf), *end);
		std::cout << "Wrote data to " << end->address().to_string() << "\n";
	}
}
//gets stuck here
void recieveData(asio::ip::udp::socket* soc, asio::ip::udp::endpoint* end) {
	char buf[129];
	soc->receive_from(asio::buffer(buf), *end);
	//string
	std::string ip = end->address().to_string();
	std::cout << "Recieved data from " << ip << "\n";
	char* m = buf;
	int* mp;
	float* mf;
	m++;
	switch (buf[0]) {
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
//server is getting stuck somewhere

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
	asio::ip::udp::endpoint recvend(asio::ip::udp::v4(), 6891);
	asio::ip::udp::endpoint sendend(asio::ip::udp::v4(), 6892);

	asio::ip::udp::socket recvsoc(io, recvend);
	asio::ip::udp::socket sendsoc(io, sendend);
	//std::thread recieve(recieveData, &recvsoc);
	//thread to write data
	while (!exitf) {
		//write data needed for each socket
		std::cout << endpoints.size() << "\n";
		for (auto& i : endpoints) {
			recieveData(&recvsoc, &i.recv);
			writePlayers(&sendsoc, &i.send);
		}

	}
	listener.join();
	//recieve.join();
}