#include "simulator.h"

#include <random>
#include <iostream>

namespace BallSimulator {
    World::World(float width, float height) {
        _entities = new std::vector<Ball*>();
        _gravity = DefaultGravity;
        _bounds = new Rectangle<Ball*>(0.0f, 0.0f, width, height);
        _quadtree = new CollisionQuadtree(0, bounds());
    }

    World::~World() {
        delete _entities;
        delete _quadtree;
    }

    float World::width() const {
        return _width;
    }

    float World::height() const {
        return _height;
    }

    void World::resize(float width, float height) {
        _width = width;
        _height = height;

        _bounds->w = width;
        _bounds->h = height;

        delete _quadtree;

        _quadtree = new CollisionQuadtree(0, bounds());

        scatter();
    }

    void World::scatter() const {
        for (auto it = _entities->begin(); it != _entities->end(); ++it) {
            auto x = rand() / (RAND_MAX / _width);
            auto y = rand() / (RAND_MAX / _height);
            auto ball = *it;

            ball->position().set(x, y);
        }
    }

    void World::change_gravity(float gravity) {
        _gravity = gravity;
    }

    float World::gravity() const {
        return _gravity;
    }

    std::vector<Ball*>* World::entities() const {
        return _entities;
    }

    void World::add(Ball* ball) const {
        _entities->push_back(ball);
    }

    CollisionQuadtree* World::quadtree() const {
        return _quadtree;
    }

    Rectangle<Ball*>* World::bounds() const {
        return _bounds;
    }

    static void DoApplyBallPhysics(World* world, Ball* ball, float divisor) {
        ball->apply_gravity(*world, divisor);
        ball->apply_velocity(divisor);
    }

    static void DoQuadtreeCollisionDetection(World* world, float divisor) {
        auto tree = world->quadtree();

        tree->clear();

        auto entities = world->entities();
        auto i = 0;
        auto count = entities->size();
        auto array = new Rectangle<Ball*>*[count];
        for (auto it = entities->begin(); it != entities->end(); ++it) {
            auto ball = *it;

            DoApplyBallPhysics(world, ball, divisor);

            auto rect = ball->rect();
            tree->insert(rect);
            array[i] = rect;
            i++;
        }


        std::vector<Rectangle<Ball*>*> queued;
        for (i = 0; i < entities->size(); i++) {
            auto rect = array[i];
            auto ballA = rect->value;
            tree->retrieve(&queued, rect);

            auto colliding = false;
            for (auto bb = queued.begin(); bb != queued.end(); ++bb) {
                auto ballB = (*bb)->value;

                if (ballB && ballA->collides(*ballB)) {
                    ballA->collide(*ballB);
                    colliding = true;
                }
            }

            ballA->isInsideCollision = colliding;

            queued.clear();
        }

        delete[] array;
    }

    static void DoSimpleCollisionDetection(World* world, float divisor) {
        auto entities = world->entities();

        for (auto it = entities->begin(); it != entities->end(); ++it) {
            DoApplyBallPhysics(world, *it, divisor);
        }

        for (unsigned long i = 0; i < entities->size(); i++) {
            auto b = entities->at(i);
            auto colliding = false;

            for (auto j = i + 1; j < entities->size(); j++) {
                auto bb = entities->at(j);
                if (b->collides(*bb)) {
                    colliding = true;
                    b->collide(*bb);
                }
            }

            b->isInsideCollision = colliding;
        }
    }

    void World::check_collisions(float divisor) {
        DoQuadtreeCollisionDetection(this, divisor);

        for (auto it = _entities->begin(); it != _entities->end(); ++it) {
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
        _rect = new Rectangle<Ball*>(0.0f, 0.0f, radius * 2, radius * 2, this);
    }

    Ball::Ball(float mass, float radius, vec2f& position) {
        _mass = mass;
        _radius = radius;
        _position = new vec2f(position);
        _velocity = new vec2f();
        _rect = new Rectangle<Ball*>(0.0f, 0.0f, radius * 2, radius * 2, this);
    }

    Ball::Ball(float mass, float radius, vec2f& position, vec2f& velocity) {
        _mass = mass;
        _radius = radius;
        _position = new vec2f(position);
        _velocity = new vec2f(velocity);
        _rect = new Rectangle<Ball*>(0.0f, 0.0f, radius * 2, radius * 2, this);
    }

    Ball::~Ball() {
        delete _position;
        delete _velocity;
        delete _rect;
    }

    float Ball::radius() const {
        return _radius;
    }

    float Ball::mass() const {
        return _mass;
    }

    void Ball::set_position(float x, float y) const {
        position().set(x, y);
        _rect->x = x;
        _rect->y = y;
    }


    Rectangle<Ball*>* Ball::rect() const {
        return _rect;
    }

    vec2f& Ball::position() const {
        return *_position;
    }

    vec2f& Ball::velocity() const {
        return *_velocity;
    }

    bool Ball::collides(Ball& other) const {
        auto diffX = position().x - other.position().x;
        auto diffY = position().y - other.position().y;
        auto totalRadius = radius() + other.radius();
        auto radiusSquared = totalRadius * totalRadius;
        auto distanceSquared = diffX * diffX + diffY * diffY;

        return radiusSquared - distanceSquared > Epsilon;
    }

    void Ball::collide(Ball& other) const {
        auto totalRadius = radius() + other.radius();
        auto delta = position() - other.position();
        auto distance = delta.length();

        if (abs(totalRadius * totalRadius - vec2f::dot(delta, delta)) < Epsilon) {
            return;
        }

        auto isOnTopOfEachOther = abs(distance) <= Epsilon;
        if (isOnTopOfEachOther) {
            distance = other.radius() + radius() - 1.0f;
            delta.set(other.radius() + radius(), 0.0f);
        }
        auto minimumTranslationDistance = delta * ((radius() + other.radius() - distance) / distance);

        auto inverseMassA = 1.0f / mass();
        auto inverseMassB = 1.0f / other.mass();
        auto inverseMassTotal = inverseMassA + inverseMassB;

        auto targetPositionA = position() + minimumTranslationDistance * (inverseMassA / inverseMassTotal);
        auto targetPositionB = other.position() - minimumTranslationDistance * (inverseMassB / inverseMassTotal);

        auto impactSpeed = velocity() - other.velocity();
        auto velocityNumber = vec2f::dot(impactSpeed, minimumTranslationDistance.normalize());

        set_position(targetPositionA.x, targetPositionA.y);
        other.set_position(targetPositionB.x, targetPositionB.y);

        if (velocityNumber > Epsilon) {
            return;
        }

        auto impulseFactor = -(velocityNumber / inverseMassTotal);
        auto impulse = minimumTranslationDistance.normalize() * impulseFactor * IMPULSE_MULTIPLIER;

        auto deltaVelocityA = impulse * inverseMassA;
        auto deltaVelocityB = -(impulse * inverseMassB);
        auto targetVelocityA = velocity() + deltaVelocityA;
        auto targetVelocityB = other.velocity() + deltaVelocityB;

        velocity().set(targetVelocityA);
        other.velocity().set(targetVelocityB);
    }

    void Ball::apply_gravity(World& world, float divisor) const {
        if (abs(world.gravity()) > Epsilon) {
            auto vel = _velocity;
            vel->y = vel->y + world.gravity() / divisor;
        }
    }

    void Ball::apply_velocity(float divisor) const {
        auto vel = _velocity;
        auto pos = _position;

        if (abs(vel->x) < Epsilon) {
            vel->x = 0.0f;
        } else {
            auto delta = vel->x / divisor;
            set_position(pos->x + delta, pos->y);
        }

        if (abs(vel->y) < Epsilon) {
            vel->y = 0.0f;
        } else {
            auto delta = vel->y / divisor;
            set_position(pos->x, pos->y + delta);
        }
    }

    void Ball::check_world_boundary(World& world) const {
        auto r2 = radius();
        auto vel = _velocity;
        auto pos = _position;

        if (pos->x - r2 < Epsilon) {
            set_position(r2, pos->y);
            vel->x = -vel->x;
        } else if (pos->x + r2 > world.width()) {
            set_position(world.width() - r2, pos->y);
            vel->x = -vel->x;
        }

        if (pos->y - r2 < Epsilon) {
            set_position(pos->x, r2);
            vel->y = -vel->y;
        } else if (pos->y + r2 > world.height()) {
            set_position(pos->x, world.height() - r2);
            vel->y = -vel->y;
        }
    }
}
