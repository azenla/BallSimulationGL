#pragma once

#include "vec2.hpp"
#include "rectangle.hpp"

namespace BallSimulator {
    class World;

    class Ball {
        float _mass;
        float _radius;
        vec2f _position;
        vec2f _velocity;
        Rectangle<Ball*> _rect;

    public:
        int collisionFlash = 0;

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

        void update(const World& world, float deltaTime);
        bool collide(Ball& other);
        void apply_world_boundary(const World& world);
    };
}
