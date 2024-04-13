#include "world.hpp"
#include "ball.hpp"
#include "quadtree.hpp"

using namespace BallSimulator;

World::World(float width, float height) :
    _gravity(DefaultGravity),
    _bounds(Rectangle<Ball*>(0.0f, 0.0f, width, height)),
    _quadtree(CollisionQuadtree(0, bounds())) {
}

void World::resize(float width, float height) {
    _width = width;
    _height = height;

    _bounds.w = width;
    _bounds.h = height;

    _quadtree = CollisionQuadtree(0, bounds());

    scatter();
}

void World::scatter() {
    for (auto ball : _entities) {
        ball->set_position(
            rand() / (RAND_MAX / _width),
            rand() / (RAND_MAX / _height)
        );
    }
}

void World::add(Ball* ball) {
    _entities.emplace_back(ball);
}
