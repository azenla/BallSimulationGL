#include "world.hpp"
#include "quadtree.hpp"
#include "config.h"

#include <utility>

using namespace BallSimulator;

World::World() :
    _bounds(Rectangle<float>::zero()),
    _gravity(0.0f) {
}

void World::resize(const Rectangle<float>& bounds) {
    _bounds = bounds;
    _quadtree = CollisionQuadtree(0, _bounds);
}

void World::scatter() {
    for (auto& ball : _entities) {
        ball->set_position(
            rand() / (RAND_MAX / _bounds.w),
            rand() / (RAND_MAX / _bounds.h)
        );
    }
}
