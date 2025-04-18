#include "ball.hpp"
#include "world.hpp"
#include "simulator.hpp"
#include "config.h"

using namespace BallSimulator;

void Ball::update(const World& world, float deltaTime) {
    _velocity.y += world.gravity() * deltaTime;
    set_position(_position + _velocity * deltaTime);
}

bool Ball::collide(Ball& other) {
    float totalRadius = radius() + other.radius();
    vec2f delta = get_position() - other.get_position();
    float distance2 = delta.length2();
    if (distance2 == 0.0f || totalRadius * totalRadius < distance2) {
        return false;
    }

    other.collisionFlash = collisionFlash = COLLISION_FLASH_DURATION;

    // calculate intersection depth and normal
    float distance = std::sqrt(distance2);
    vec2f normal = delta / distance;
    float intersectDepth = totalRadius - distance;
    vec2f pushDirection = normal * intersectDepth + Epsilon;

    float inverseMassA = 1.0f / mass();
    float inverseMassB = 1.0f / other.mass();
#ifndef SIMULATION_LOSSES
    const float inverseMassScale = 1.0f / (inverseMassA + inverseMassB);
#else
    constexpr float inverseMassScale = 1.0f;  // disabling rescaling induces losses and may be more realistic
#endif

    // push balls out of each other
    set_position(_position + pushDirection * (inverseMassA * inverseMassScale));
    other.set_position(other.get_position() - pushDirection * (inverseMassB * inverseMassScale));

    auto impactSpeed = _velocity - other._velocity;
    auto velocityNumber = vec2f::dot(impactSpeed, normal);

    if (velocityNumber > 0.0f) {
        return true;
    }

    // compute and apply velocity response
    auto impulseFactor = -2.0f * velocityNumber * inverseMassScale * IMPULSE_MULTIPLIER;
    vec2f impulse = normal * impulseFactor;
    _velocity += impulse * inverseMassA;
    other.set_velocity(other._velocity - impulse * inverseMassB);

    return true;
}

void Ball::apply_world_boundary(const World& world) {
#ifdef SIMULATION_LOSSES
    const float inverseMass = 1.0f / mass();
#else
    constexpr float inverseMass = 1.0f;
#endif

    if (_position.x - _radius < Epsilon) {
        set_position(_radius, _position.y);
        _velocity.x = -_velocity.x * inverseMass;
        collisionFlash = COLLISION_FLASH_DURATION;
    } else if (_position.x + _radius > world.width()) {
        set_position(world.width() - _radius, _position.y);
        _velocity.x = -_velocity.x * inverseMass;
        collisionFlash = COLLISION_FLASH_DURATION;
    }

    if (_position.y - _radius < Epsilon) {
        set_position(_position.x, _radius);
        _velocity.y = -_velocity.y * inverseMass;
        collisionFlash = COLLISION_FLASH_DURATION;
    } else if (_position.y + _radius > world.height()) {
        set_position(_position.x, world.height() - _radius);
        _velocity.y = -_velocity.y * inverseMass;
        collisionFlash = COLLISION_FLASH_DURATION;
    }
}
