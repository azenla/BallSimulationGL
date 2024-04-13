#include "simulator.hpp"
#include "ball.hpp"
#include "world.hpp"

#include <thread>
#include <chrono>

int main() {
    auto world = std::make_unique<BallSimulator::World>(1024, 1024);
    for (auto i = 1; i <= 20; i++) {
        auto ball = new BallSimulator::Ball(5.0f, 20.0f);
        world->add(ball);
    }

    for (auto i = 1; i <= 1000000; i++) {
        BallSimulator::DoQuadtreeCollisionDetection(*world, 0.01f);
    }

    return 0;
}
