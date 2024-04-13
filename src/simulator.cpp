#include "simulator.hpp"
#include "world.hpp"
#include "ball.hpp"

#include <random>
#include <iostream>

void BallSimulator::DoQuadtreeCollisionDetection(World& world, float divisor) {
    auto& tree = world.quadtree();

    tree.clear();

    const auto& entities = world.entities();
    auto i = 0;
    static std::vector<const Rectangle<Ball*>*> array;
    array.resize(entities.size());
    for (auto ball : entities) {
        ball->apply_gravity(world, divisor);
        ball->apply_velocity(divisor);

        const auto& rect = ball->rect();
        tree.insert(rect);
        array[i++] = &rect;
    }

    std::vector<const Rectangle<Ball*>*> queued;
    for (const auto& rect : array) {
        auto ballA = rect->value;
        tree.retrieve(queued, *rect);

        auto colliding = false;
        for (auto bb : queued) {
            const auto& ballB = bb->value;

            if (ballB && ballB != ballA && ballA->collides(*ballB)) {
                ballA->collide(*ballB);
                colliding = true;
            }
        }

        ballA->isInsideCollision = colliding;

        ballA->apply_world_boundary(world);

        queued.clear();
    }
}

void BallSimulator::DoSimpleCollisionDetection(World& world, float divisor) {
    auto& entities = world.entities();

    for (auto ball : entities) {
        ball->apply_gravity(world, divisor);
        ball->apply_velocity(divisor);
    }

    for (unsigned long i = 0; i < entities.size(); i++) {
        auto b = entities.at(i);
        auto colliding = false;

        for (auto j = i + 1; j < entities.size(); j++) {
            auto bb = entities.at(j);
            if (b->collides(*bb)) {
                colliding = true;
                b->collide(*bb);
            }
        }

        b->isInsideCollision = colliding;
        b->apply_world_boundary(world);
    }
}
