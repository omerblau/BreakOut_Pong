#include "include/game.h"
using namespace game;

int main() {
	Game p;
	if (p.valid())
		p.run();
	return 0;
}