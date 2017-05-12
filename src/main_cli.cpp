#include "simulator.h"

#include <thread>
#include <chrono>

int main() {
    auto world = new BallSimulator::World(1024, 1024);
    for (auto i = 1; i <= 20; i++) {
        auto ball = new BallSimulator::Ball(5.0f, 20.0f);
        world->add(ball);
    }

    for (auto i = 1; i <= 1000000; i++) {
        world->tick(0.01f);
    }

	delete world;

    return 0;
}
