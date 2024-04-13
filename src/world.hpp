#pragma once

#include "simulator.hpp"

namespace BallSimulator {
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

        void scatter();
        void add(Ball* ball);

        inline constexpr const std::vector<Ball*>& entities() const { return _entities; }
        inline constexpr const CollisionQuadtree& quadtree() const { return _quadtree; }
        inline constexpr CollisionQuadtree& quadtree() { return _quadtree; }
        inline constexpr const Rectangle<Ball*>& bounds() const { return _bounds; }
    };
}
