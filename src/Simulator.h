#pragma once

#include <vector>

#include "vec2.h"
#include "quadtree.h"

namespace BallSimulator {
    const float RealWorldGravity = 9.18f;

    const float RealWorldScale = 10.0f;
    const float DefaultGravity = RealWorldScale * RealWorldGravity;

    const float Epsilon = 0.000009f;

    class World;

    class Ball {
        float _mass;
        float _radius;
        vec2f *_position;
        vec2f *_velocity;

    public:
        bool isInsideCollision = false;

        Ball(float mass, float radius);
        Ball(float mass, float radius, vec2f& position);
        Ball(float mass, float radius, vec2f& position, vec2f& velocity);
        ~Ball();

        float mass();
        float radius();
        vec2f& position();
        vec2f& velocity();

        Rectangle<Ball*>* rect();

        bool collides(Ball &other);
        void collide(Ball &other);

        void apply_gravity(World& world, float divisor);
        void apply_velocity(float divisor);
        void check_world_boundary(World& world);
    };

    class World {
    private:
        float _width;
        float _height;
        float _gravity;
        std::vector<Ball*> *_entities;

    public:
        World(float width, float height);
        ~World();

        float width();
        float height();
        float gravity();

        void resize(float width, float height);
        void change_gravity(float gravity);
        void check_collisions(float divisor);

        void scatter();
        void tick(float divisor);

        std::vector<Ball*>& entities();
    };
}
