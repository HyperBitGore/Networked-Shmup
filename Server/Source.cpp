#include <iostream>
#include <asio.hpp>
#include <thread>
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


bool exitf = false;
asio::io_context io;
std::vector<Player> players;
std::vector<Bullet> bullets;
std::vector<asio::ip::udp::socket*> udpsockets;

//listens to connections and adds to socket list
void connectListener() {
	while (!exitf) {
		asio::ip::tcp::acceptor accept(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 6890));
		asio::error_code ec;
		asio::ip::tcp::socket soc(io);
		accept.listen();
		accept.accept(soc, ec);
		//write entire player list
		unsigned char* buf;
		if (players.size() > 0) {
			buf = (unsigned char*)std::malloc(size_t(players.size() * sizeof(Player)));

			std::free(buf);
		}
		else {
			
		}
		//write entire bullet list
		if (bullets.size() > 0) {
			buf = (unsigned char*)std::malloc(size_t(players.size() * sizeof(Player)));

			std::free(buf);
		}
		else {
			
		}
		//get udp socket
		asio::ip::tcp::endpoint remote_ep = soc.remote_endpoint();
		asio::ip::address remote_ad = remote_ep.address();
		std::string ipstring = remote_ad.to_string();
		asio::ip::udp::socket* udpsoc = new asio::ip::udp::socket(io);
		asio::ip::udp::endpoint udpaddr(asio::ip::address::from_string(ipstring), 6890);
		udpsoc->connect(udpaddr);
		soc.close();
	}
}

int main() {
	//doing this to avoid moving of memory block
	players.reserve(100);
	bullets.reserve(1000);
	udpsockets.reserve(100);

	std::thread listener(connectListener);
	while (!exitf) {
		

	}
	listener.join();
}