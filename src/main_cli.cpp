#include "Simulator.h"

#include <thread>
#include <chrono>

int main() {
    auto world = new BallSimulator::World(1024, 1024);
    for (auto i = 1; i <= 20; i++) {
        auto ball = new BallSimulator::Ball(5.0f, 20.0f);
        world->entities().push_back(ball);
    }

    while (true) {
        world->tick(5 / 1000.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return 0;
}
