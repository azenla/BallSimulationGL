#include "ball.hpp"
#include "world.hpp"
#include "simulator.hpp"

using namespace BallSimulator;

Ball::Ball(float mass, float radius, const vec2f& position, const vec2f& velocity) :
        _rect(Rectangle<Ball*>(position.x, position.y, radius * 2, radius * 2, this)) {
    _mass = mass;
    _radius = radius;
    _position = position;
    _velocity = velocity;
}

void Ball::set_position(const vec2f& newPos) {
    _position = newPos;
    _rect.x = newPos.x;
    _rect.y = newPos.y;
}

bool Ball::collide(Ball& other) {
    float totalRadius = radius() + other.radius();
    vec2f delta = get_position() - other.get_position();
    float distance2 = delta.length2();
    if (totalRadius * totalRadius < distance2) {
        return false;
    }

    // calculate intersection depth and normal
    float distance = std::sqrt(distance2);
    vec2f normal = delta / distance;
    float intersectDepth = totalRadius - distance;
    vec2f pushDirection = normal * intersectDepth + Epsilon;

    float inverseMassA = 1.0f / mass();
    float inverseMassB = 1.0f / other.mass();
#if 1
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

void Ball::apply_gravity(World& world, float divisor) {
    if (std::abs(world.gravity()) > Epsilon) {
        _velocity.y += world.gravity() / divisor;
    }
}

void Ball::apply_velocity(float divisor) {
    if (std::abs(_velocity.x) < Epsilon) {
        _velocity.x = 0.0f;
    } else {
        auto delta = _velocity.x / divisor;
        set_position(_position.x + delta, _position.y);
    }

    if (std::abs(_velocity.y) < Epsilon) {
        _velocity.y = 0.0f;
    } else {
        auto delta = _velocity.y / divisor;
        set_position(_position.x, _position.y + delta);
    }
}

void Ball::apply_world_boundary(const World& world) {
    if (_position.x - _radius < Epsilon) {
        set_position(_radius, _position.y);
        _velocity.x = -_velocity.x;
    } else if (_position.x + _radius > world.width()) {
        set_position(world.width() - _radius, _position.y);
        _velocity.x = -_velocity.x;
    }

    if (_position.y - _radius < Epsilon) {
        set_position(_position.x, _radius);
        _velocity.y = -_velocity.y;
    } else if (_position.y + _radius > world.height()) {
        set_position(_position.x, world.height() - _radius);
        _velocity.y = -_velocity.y;
    }
}
