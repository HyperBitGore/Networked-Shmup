#include "Header.h"



bool Game::isColliding(Entity* e1, Entity* e2) {
	if (e1->x < e2->x + e2->w && e1->x + e1->w > e2->x && e1->y < e2->y + e2->h && e1->y + e1->h > e2->y) {
		return true;
	}
	return false;
}