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
        vec2f _position;
        vec2f _velocity;
        Rectangle<Ball*> _rect;

    public:
        bool isInsideCollision = false;

        Ball(float mass, float radius, const vec2f& position = vec2f::zero(), const vec2f& velocity = vec2f::zero());

        inline constexpr float mass() const { return _mass; }
        inline constexpr float radius() const { return _radius; }

        inline constexpr const vec2f& get_position() const { return _position; }
        inline constexpr const vec2f& get_velocity() const { return _velocity; }

        void set_position(const vec2f& newpos);
        inline void set_position(float x, float y) { set_position({ x, y }); }
        inline void set_velocity(const vec2f& newvel) { _velocity = newvel; }
        inline void set_velocity(float x, float y) { set_velocity({ x, y }); }

        inline constexpr const Rectangle<Ball*>& rect() const { return _rect; }

        bool collides(const Ball& other) const;
        void collide(Ball& other);

        void apply_gravity(World& world, float divisor);
        void apply_velocity(float divisor);
        void apply_world_boundary(const World& world);
    };

    typedef Quadtree<Ball*, QUADTREE_MAX_OBJECTS, QUADTREE_MAX_LEVELS> CollisionQuadtree;

    class World {
        float _width;
        float _height;
        float _gravity;
        std::vector<Ball*> _entities;
        CollisionQuadtree _quadtree;
        Rectangle<Ball*> _bounds;

    public:
        World(float width, float height);

        inline constexpr float width() const { return _width; }
        inline constexpr float height() const { return _height; }
        inline constexpr float gravity() const { return _gravity; }

        void resize(float width, float height);
        inline void change_gravity(float gravity) { _gravity = gravity; }
        void check_collisions(float divisor);

        void scatter();
        void tick(float divisor);
        void add(Ball* ball);

        inline constexpr const std::vector<Ball*>& entities() const { return _entities; }
        inline constexpr const CollisionQuadtree& quadtree() const { return _quadtree; }
        inline constexpr CollisionQuadtree& quadtree() { return _quadtree; }
        inline constexpr const Rectangle<Ball*>& bounds() const { return _bounds; }
    };
}
