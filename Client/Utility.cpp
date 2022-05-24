#include "Header.h"



bool Game::isColliding(Entity* e1, Entity* e2) {
	if (e1->x < e2->x + e2->w && e1->x + e1->w > e2->x && e1->y < e2->y + e2->h && e1->y + e1->h > e2->y) {
		return true;
	}
	return false;
}

void Game::connectToServer(std::string ip, std::vector<Player>& players, std::vector<Bullet>& bullets) {
	Gore gore;
	asio::ip::tcp::socket tcpsock(io);
	asio::ip::tcp::endpoint tcpend(asio::ip::address::from_string(ip), 6890);
	tcpsock.connect(tcpend);
	//read all the data it sends
	bool notread = true;
	char mode = 0;
	char buf[28];
	while (notread) {
		Player p;
		Bullet b;
		tcpsock.receive(asio::buffer(buf));
		switch (mode) {
		case 0:
			if (buf[0] == -1) {
				mode = 1;
			}
			else {
				gore.deserilizeStruct((char*)&p, buf, 24);
				std::cout << buf << "\n";
				players.push_back(p);
			}
			break;
		case 1:
			//have to do this because bullet data can get out of order
			for (int i = 0; i < 28; i++) {
				if (buf[i] == -1) {
					notread = false;
					break;
				}
			}
			gore.deserilizeStruct((char*)&b, buf, 28);
			std::cout << buf << "\n";
			bullets.push_back(b);		
			break;
		}
	}
	tcpsock.close();
	//now you get the udp socket setup
	asio::ip::udp::endpoint udpend(asio::ip::address::from_string(ip), 6891);
	udpsock.connect(udpend);
}