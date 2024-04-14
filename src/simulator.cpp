#include "simulator.hpp"
#include "world.hpp"
#include "ball.hpp"

#include <random>
#include <iostream>

void BallSimulator::DoQuadtreeCollisionDetection(World& world, float deltaTime) {
    static std::vector<CollisionQuadtree::RefT> array, queued;

    deltaTime *= SIMULATION_TIMESCALE;
    auto& tree = world.quadtree();

    tree.clear();

    const auto& entities = world.entities();
    auto i = 0;
    array.reserve(entities.size());
    for (auto ball : entities) {
        ball->update(world, deltaTime);
        tree.insert(std::ref(*ball));
        array.emplace_back(std::ref(*ball));
    }

    for (auto& ballA : array) {
        tree.retrieve(queued, ballA);

        for (auto& ballB : queued) {
            if (&ballB.get() != &ballA.get()) {
                ballA.get().collide(ballB);
            }
        }

        ballA.get().apply_world_boundary(world);
        queued.clear();
    }
    array.clear();
}

void BallSimulator::DoSimpleCollisionDetection(World& world, float deltaTime) {
    deltaTime *= SIMULATION_TIMESCALE;
    auto& entities = world.entities();

    for (auto ball : entities) {
        ball->update(world, deltaTime);
    }

    for (unsigned long i = 0; i < entities.size(); i++) {
        auto b = entities.at(i);

        for (auto j = i + 1; j < entities.size(); j++) {
            b->collide(*entities.at(j));
        }

        b->apply_world_boundary(world);
    }
}
