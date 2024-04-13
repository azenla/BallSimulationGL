#include "simulator.h"

#include <random>
#include <iostream>

namespace BallSimulator {
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

    static void DoApplyBallPhysics(World& world, Ball& ball, float divisor) {
        ball.apply_gravity(world, divisor);
        ball.apply_velocity(divisor);
    }

    static void DoQuadtreeCollisionDetection(World& world, float divisor) {
        auto& tree = world.quadtree();

        tree.clear();

        const auto& entities = world.entities();
        auto i = 0;
        auto count = entities.size();
        std::vector<const Rectangle<Ball*>*> array(count);
        for (const auto& ball : entities) {
            DoApplyBallPhysics(world, *ball, divisor);

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

    static void DoSimpleCollisionDetection(World& world, float divisor) {
        auto& entities = world.entities();

        for (auto& ball : entities) {
            DoApplyBallPhysics(world, *ball, divisor);
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

    void World::check_collisions(float divisor) {
        DoQuadtreeCollisionDetection(*this, divisor);
        //DoSimpleCollisionDetection(*this, divisor);
    }

    void World::tick(float divisor) {
        check_collisions(divisor);
    }

    Ball::Ball(float mass, float radius, const vec2f& position, const vec2f& velocity) :
            _rect(Rectangle<Ball*>(position.x, position.y, radius * 2, radius * 2, this)) {
        _mass = mass;
        _radius = radius;
        _position = position;
        _velocity = velocity;
    }

    void Ball::set_position(const vec2f& newPos) {
        _position = newPos;
        _rect.x = newPos.x;
        _rect.y = newPos.y;
    }

    bool Ball::collides(const Ball& other) const {
        auto diff = _position - other._position;
        auto totalRadius = radius() + other.radius();
        auto radiusSquared = totalRadius * totalRadius;
        auto distanceSquared = diff.length();

        return radiusSquared - diff.length2() > Epsilon;
    }

    void Ball::collide(Ball& other) {
        auto totalRadius = radius() + other.radius();
        auto delta = _position - other.get_position();
        auto distance = delta.length();

        if (abs(totalRadius * totalRadius - vec2f::dot(delta, delta)) < Epsilon) {
            return;
        }

        auto isOnTopOfEachOther = abs(distance) <= Epsilon;
        if (isOnTopOfEachOther) {
            distance = totalRadius - 1.0f;
            delta = vec2f(totalRadius, 0.0f);
        }
        auto minimumTranslationDistance = delta * ((totalRadius - distance) / distance);

        auto inverseMassA = 1.0f / mass();
        auto inverseMassB = 1.0f / other.mass();
        auto inverseMassTotal = inverseMassA + inverseMassB;

        set_position(_position + minimumTranslationDistance * (inverseMassA / inverseMassTotal));
        other.set_position(other.get_position() - minimumTranslationDistance * (inverseMassB / inverseMassTotal));

        auto impactSpeed = _velocity - other._velocity;
        auto velocityNumber = vec2f::dot(impactSpeed, minimumTranslationDistance.normalize());

        if (velocityNumber > 0.0f) {
            return;
        }

        auto impulseFactor = -2.0f * velocityNumber / inverseMassTotal;
        vec2f impulse = minimumTranslationDistance.normalize() * impulseFactor * IMPULSE_MULTIPLIER;

        _velocity += impulse * inverseMassA;
        other.set_velocity(other._velocity - impulse * inverseMassB);
    }

    void Ball::apply_gravity(World& world, float divisor) {
        if (std::abs(world.gravity()) > Epsilon) {
            _velocity.y += world.gravity() / divisor;
        }
    }

    void Ball::apply_velocity(float divisor) {
        if (std::abs(_velocity.x) < Epsilon) {
            _velocity.x = 0.0f;
        } else {
            auto delta = _velocity.x / divisor;
            set_position(_position.x + delta, _position.y);
        }

        if (std::abs(_velocity.y) < Epsilon) {
            _velocity.y = 0.0f;
        } else {
            auto delta = _velocity.y / divisor;
            set_position(_position.x, _position.y + delta);
        }
    }

    void Ball::apply_world_boundary(const World& world) {
        if (_position.x - _radius < Epsilon) {
            set_position(_radius, _position.y);
            _velocity.x = -_velocity.x;
        } else if (_position.x + _radius > world.width()) {
            set_position(world.width() - _radius, _position.y);
            _velocity.x = -_velocity.x;
        }

        if (_position.y - _radius < Epsilon) {
            set_position(_position.x, _radius);
            _velocity.y = -_velocity.y;
        } else if (_position.y + _radius > world.height()) {
            set_position(_position.x, world.height() - _radius);
            _velocity.y = -_velocity.y;
        }
    }
}
