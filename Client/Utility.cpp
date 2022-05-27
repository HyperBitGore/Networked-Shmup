#include "Header.h"


bool Game::isColliding(Entity* e1, Entity* e2) {
	if (e1->x < e2->x + e2->w && e1->x + e1->w > e2->x && e1->y < e2->y + e2->h && e1->y + e1->h > e2->y) {
		return true;
	}
	return false;
}

void Game::connectToServer(std::string ip, std::vector<Player>& players, std::vector<Bullet>& bullets, Player* p1) {
	Gore gore;
	asio::io_context io;
	asio::ip::tcp::endpoint socend(asio::ip::address::from_string(ip), 6890);
	asio::ip::tcp::socket soc(io);
	soc.connect(socend);
	//read all the data it sends
	bool notread = true;
	char mode = 0;
	char buf[28];
	while (notread) {
		Player p;
		Bullet b;
		soc.receive(asio::buffer(buf));
		switch (mode) {
		case 0:
			for (int i = 0; i < 28; i++) {
				if (buf[i] == -1) {
					mode = 1;
					break;
				}
			}
			gore.deserilizeStruct((char*)&p, buf, 24);
			std::cout << buf << "\n";
			players.push_back(p);
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
	p1->index = players.size()-1;
	playerindex = p1->index;
	soc.close();
}