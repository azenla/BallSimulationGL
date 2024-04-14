#pragma once

#include "simulator.hpp"
#include "ball.hpp"
#include <memory>

namespace BallSimulator {
    class World {
        float _width;
        float _height;
        float _gravity;
        std::vector<std::unique_ptr<Ball>> _entities;
        CollisionQuadtree _quadtree;
        Rectangle<float> _bounds;

    public:
        World(float width, float height);

        inline constexpr float width() const { return _width; }
        inline constexpr float height() const { return _height; }
        inline constexpr float gravity() const { return _gravity; }

        void resize(float width, float height);
        inline void set_gravity(float gravity) { _gravity = gravity; }
        void scatter();

        void add(const Ball& ball) { _entities.emplace_back(std::make_unique<Ball>(ball)); }
        void add(Ball&& ball)      { _entities.emplace_back(std::make_unique<Ball>(std::move(ball))); }

        inline constexpr const std::vector<std::unique_ptr<Ball>>& entities() const { return _entities; }
        inline constexpr const CollisionQuadtree& quadtree() const { return _quadtree; }
        inline constexpr CollisionQuadtree& quadtree() { return _quadtree; }
        inline constexpr const Rectangle<float>& bounds() const { return _bounds; }
    };
}
