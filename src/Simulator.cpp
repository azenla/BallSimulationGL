#include "Simulator.h"

#include <random>
#include <iostream>

namespace BallSimulator {
    World::World(float width, float height) {
        _width = width;
        _height = height;
        _entities = new std::vector<Ball*>();
        _gravity = DefaultGravity;
    }

    World::~World() {
        delete _entities;
    }

    float World::width() {
        return _width;
    }

    float World::height() {
        return _height;
    }

    void World::resize(float width, float height) {
        _width = width;
        _height = height;

        scatter();
    }

    void World::scatter() {
        for (auto it = _entities->begin(); it != _entities->end(); it++) {
            auto x = std::rand() / (RAND_MAX / _width);
            auto y = std::rand() / (RAND_MAX / _height);
            auto ball = *it;

            ball->position().set(x, y);
        }
    }

    void World::change_gravity(float gravity) {
        _gravity = gravity;
    }

    float World::gravity() {
        return _gravity;
    }

    std::vector<Ball*>& World::entities() {
        return *_entities;
    }

    void World::check_collisions(float divisor) {
        for (auto it = _entities->begin(); it != _entities->end(); it++) {
            auto ball = *it;
            ball->apply_gravity(*this, divisor);
            ball->apply_velocity(divisor);
        }

        for (unsigned long i = 0; i < _entities->size(); i++) {
            auto b = _entities->at(i);

            for (auto j = i + 1; j < _entities->size(); j++) {
                auto bb = _entities->at(j);
                if (b->collides(*bb)) {
                    b->collide(*bb);
                }
            }
        }

        for (auto it = _entities->begin(); it != _entities->end(); it++) {
            auto ball = *it;
            ball->check_world_boundary(*this);
        }
    }

    void World::tick(float divisor) {
        check_collisions(divisor);
    }

    Ball::Ball(float mass, float radius) {
        _mass = mass;
        _radius = radius;
        _position = new vec2f();
        _velocity = new vec2f();
    }

    Ball::Ball(float mass, float radius, vec2f& position) {
        _mass = mass;
        _radius = radius;
        _position = new vec2f(position);
        _velocity = new vec2f();
    }

    Ball::Ball(float mass, float radius, vec2f& position, vec2f& velocity) {
        _mass = mass;
        _radius = radius;
        _position = new vec2f(position);
        _velocity = new vec2f(velocity);
    }

    Ball::~Ball() {
        delete _position;
        delete _velocity;
    }

    float Ball::radius() {
        return _radius;
    }

    float Ball::mass() {
        return _mass;
    }

    vec2f& Ball::position() {
        return *_position;
    }

    vec2f& Ball::velocity() {
        return *_velocity;
    }

    bool Ball::collides(Ball &other) {
        auto diffX = position().x - other.position().x;
        auto diffY = position().y - other.position().y;
        auto totalRadius = radius() + other.radius();
        auto radiusSquared = totalRadius * totalRadius;
        auto distanceSquared = diffX * diffX + diffY * diffY;

        return radiusSquared - distanceSquared > Epsilon;
    }

    void Ball::collide(Ball &other) {
        auto totalRadius = radius() + other.radius();
        auto delta = position() - other.position();
        auto distance = delta.length();

        if (std::abs(totalRadius * totalRadius - vec2f::dot(delta, delta)) < Epsilon) {
            return;
        }

        vec2f minimumTranslationDistance;

        if (std::abs(distance) < Epsilon) {
            minimumTranslationDistance = delta * ((radius() + other.radius() - distance) / distance);
        } else {
            distance = other.radius() + radius() - 1.0f;
            delta.set(other.radius() + radius(), 0.0f);
            minimumTranslationDistance = delta * ((radius() + other.radius() - distance) / distance);
        }

        auto inverseMassA = 1 / mass();
        auto inverseMassB = 1 / other.mass();
        auto inverseMassTotal = inverseMassA + inverseMassB;

        auto targetPositionA = position() + (minimumTranslationDistance * (inverseMassA / inverseMassTotal));
        auto targetPositionB = other.position() - (minimumTranslationDistance * (inverseMassB / inverseMassTotal));

        auto impactSpeed = velocity() - other.velocity();
        auto velocityNumber = vec2f::dot(impactSpeed, minimumTranslationDistance.normalize());

        position().set(targetPositionA);
        other.position().set(targetPositionB);

        if (velocityNumber > Epsilon) {
            return;
        }

        auto impulseFactor = -1.0f * velocityNumber / inverseMassTotal;
        auto impulse = minimumTranslationDistance.normalize() * impulseFactor;

        if (std::isnan(impulse.length())) {
            impulse.set(0.0f, 0.0f);
        }

        auto deltaVelocityA = impulse * inverseMassA;
        auto deltaVelocityB = -(impulse * inverseMassB);
        auto targetVelocityA = velocity() + deltaVelocityA;
        auto targetVelocityB = other.velocity() + deltaVelocityB;

        velocity().set(targetVelocityA);
        other.velocity().set(targetVelocityB);
    }

    void Ball::apply_gravity(World &world, float divisor) {
        if (std::abs(world.gravity()) > Epsilon) {
            auto vel = _velocity;
            vel->y = vel->y + world.gravity() / divisor;
        }
    }

    void Ball::apply_velocity(float divisor) {
        auto vel = _velocity;
        auto pos = _position;

        if (std::abs(vel->x) < Epsilon) {
            vel->x = 0.0f;
        } else {
            auto delta = vel->x / divisor;
            pos->x += delta;
        }

        if (std::abs(vel->y) < Epsilon) {
            vel->y = 0.0f;
        } else {
            auto delta = vel->y / divisor;
            pos->y += delta;
        }
    }

    void Ball::check_world_boundary(World &world) {
        auto r2 = radius();
        auto vel = _velocity;
        auto pos = _position;

        if (pos->x - r2 < Epsilon) {
            pos->x = r2;
            pos->x = -vel->x;
        } else if (pos->x + r2 > world.width()) {
            pos->x = world.width() - r2;
            vel->x = -vel->x;
        }

        if (pos->y - r2 < Epsilon) {
            pos->y = r2;
            vel->y = -vel->y;
        } else if (pos->y + r2 > world.height()) {
            pos->y = world.height() - r2;
            vel->y = -vel->y;
        }
    }
}
