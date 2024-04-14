#pragma once

#include "vec2.hpp"
#include "rectangle.hpp"
#include <utility>

namespace BallSimulator {
    class World;

    class Ball {
        float _mass;
        float _radius;
        vec2f _position;
        vec2f _velocity;

    public:
        int collisionFlash = 0;

        Ball(float mass, float radius, const vec2f& position = vec2f::zero(), const vec2f& velocity = vec2f::zero()) :
            _mass(mass), _radius(radius), _position(position), _velocity(velocity) {}
        constexpr Ball(const Ball& other) :
            _mass(other._mass), _radius(other._radius), _position(other._position), _velocity(other._velocity) {}
        constexpr Ball(Ball&& other) : _mass(other._mass), _radius(other._radius),
            _position(std::move(other._position)), _velocity(std::move(other._velocity)) {}

        inline constexpr float mass() const { return _mass; }
        inline constexpr float radius() const { return _radius; }

        inline constexpr const vec2f& get_position() const { return _position; }
        inline constexpr const vec2f& get_velocity() const { return _velocity; }

        inline void set_position(const vec2f& newpos) { _position = newpos; }
        inline void set_position(float x, float y) { set_position({ x, y }); }
        inline void set_velocity(const vec2f& newvel) { _velocity = newvel; }
        inline void set_velocity(float x, float y) { set_velocity({ x, y }); }

        inline constexpr Rectangle<float> rect() const { return Rectangle<float>(_position, _radius * 2.0f); }

        void update(const World& world, float deltaTime);
        bool collide(Ball& other);
        void apply_world_boundary(const World& world);
    };
}
