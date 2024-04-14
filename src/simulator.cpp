#include "simulator.hpp"
#include "world.hpp"
#include "ball.hpp"

#include <random>
#include <iostream>

void BallSimulator::DoQuadtreeCollisionDetection(World& world, float deltaTime) {
    deltaTime *= SIMULATION_TIMESCALE;
    auto& tree = world.quadtree();

    tree.clear();

    const auto& entities = world.entities();
    auto i = 0;
    static std::vector<const Rectangle<Ball*>*> array;
    array.resize(entities.size());
    for (auto ball : entities) {
        ball->apply_gravity(world, deltaTime);
        ball->apply_velocity(deltaTime);

        const auto& rect = ball->rect();
        tree.insert(rect);
        array[i++] = &rect;
    }

    std::vector<const Rectangle<Ball*>*> queued;
    for (const auto& rect : array) {
        auto ballA = rect->value;
        tree.retrieve(queued, *rect);

        for (auto bb : queued) {
            const auto& ballB = bb->value;

            if (ballB && ballB != ballA && ballA->collide(*ballB)) {
                ballA->collisionFlash = ballB->collisionFlash = COLLISION_FLASH_DURATION;
            }
        }

        ballA->apply_world_boundary(world);

        queued.clear();
    }
}

void BallSimulator::DoSimpleCollisionDetection(World& world, float deltaTime) {
    deltaTime *= SIMULATION_TIMESCALE;
    auto& entities = world.entities();

    for (auto ball : entities) {
        ball->apply_gravity(world, deltaTime);
        ball->apply_velocity(deltaTime);
    }

    for (unsigned long i = 0; i < entities.size(); i++) {
        auto b = entities.at(i);

        for (auto j = i + 1; j < entities.size(); j++) {
            auto bb = entities.at(j);
            if (b->collide(*bb)) {
                b->collisionFlash = bb->collisionFlash = COLLISION_FLASH_DURATION;
            }
        }

        b->apply_world_boundary(world);
    }
}
