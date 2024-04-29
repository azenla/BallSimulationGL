#include "config.h"
#include "simulator.hpp"
#include "ball.hpp"
#include "world.hpp"

using namespace BallSimulator;

int main() {
    World world;
    world.resize({ 0, 0, 1024, 1024 });

    for (auto i = 1; i <= 20; i++) {
        world.add(Ball(5.0f, 20.0f));
    }

    for (auto i = 1; i <= 1000000; i++) {
#ifdef USE_QUADTREES
        DoQuadtreeCollisionDetection(world, 0.01f);
#else
        DoSimpleCollisionDetection(world, 0.01f);
#endif
    }

    return 0;
}
