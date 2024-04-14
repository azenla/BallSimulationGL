#pragma once

#include <vector>

#include "vec2.hpp"
#include "quadtree.hpp"
#include "config.h"

namespace BallSimulator {
    class Ball;
    class World;

    const float RealWorldGravity = 9.18f;

    const float RealWorldScale = 10.0f;
    const float DefaultGravity = RealWorldScale * RealWorldGravity;

    const float Epsilon = PHYSICS_EPSILON;

    typedef Quadtree<Ball*, QUADTREE_MAX_OBJECTS, QUADTREE_MAX_LEVELS> CollisionQuadtree;

    void DoQuadtreeCollisionDetection(World& world, float deltaTime);
    void DoSimpleCollisionDetection(World& world, float deltaTime);
}
