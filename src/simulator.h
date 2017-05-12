#pragma once

#include <vector>

#include "vec2.h"
#include "quadtree.h"
#include "config.h"

namespace BallSimulator {
    const float RealWorldGravity = 9.18f;

    const float RealWorldScale = 10.0f;
    const float DefaultGravity = RealWorldScale * RealWorldGravity;

    const float Epsilon = PHYSICS_EPSILON;

    class World;

    class Ball {
        float _mass;
        float _radius;
        vec2f* _position;
        vec2f* _velocity;

    public:
        bool isInsideCollision = false;

        Ball(float mass, float radius);
        Ball(float mass, float radius, vec2f& position);
        Ball(float mass, float radius, vec2f& position, vec2f& velocity);
        ~Ball();

        float mass() const;
        float radius() const;
        vec2f& position() const;
        vec2f& velocity() const;

        Rectangle<Ball*> rect();

        bool collides(Ball& other) const;
        void collide(Ball& other) const;

        void apply_gravity(World& world, float divisor) const;
        void apply_velocity(float divisor) const;
        void check_world_boundary(World& world) const;
    };

    typedef Quadtree<Ball*, QUADTREE_MAX_OBJECTS, QUADTREE_MAX_LEVELS> CollisionQuadtree;

    class World {
        float _width;
        float _height;
        float _gravity;
        std::vector<Ball*>* _entities;
        CollisionQuadtree* _quadtree;
        Rectangle<Ball*>* _bounds;

    public:
        World(float width, float height);
        ~World();

        float width() const;
        float height() const;
        float gravity() const;

        void resize(float width, float height);
        void change_gravity(float gravity);
        void check_collisions(float divisor);

        void scatter() const;
        void tick(float divisor);
        void add(Ball* ball) const;

        std::vector<Ball*>* entities() const;
        CollisionQuadtree* quadtree() const;
        Rectangle<Ball*>* bounds() const;
    };
}
