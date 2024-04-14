#pragma once

#include <string>
#include <cmath>

template <class T>
class vec2 {
public:
    T x, y;

    vec2() : x(0), y(0) {}

    vec2(T x, T y) : x(x), y(y) {}

    vec2(const vec2<T>& v) : x(v.x), y(v.y) {}

    vec2(vec2<T>&& v) : x(v.x), y(v.y) {}

    template <typename F>
    vec2(const vec2<F>& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) {}

    // constants

    static constexpr vec2<T> zero() { return { static_cast<T>(0), static_cast<T>(0) }; }

    // assignment

    inline vec2<T>& operator =(vec2<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline vec2<T>& operator =(const vec2<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline vec2<T>& operator =(vec2<T>&& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    // compound vector arithmetic

    inline vec2<T>& operator +=(const vec2<T>& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    inline vec2<T>& operator -=(const vec2<T>& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    inline vec2<T>& operator *=(const vec2<T>& v) {
        x *= v.x;
        y *= v.y;
        return *this;
    }

    inline vec2<T>& operator /=(const vec2<T>& v) {
        x /= v.x;
        y /= v.y;
        return *this;
    }

    // unary minus

    inline constexpr vec2<T> operator -() const {
        return vec2<T>(-x, -y);
    }

    // vector arithmetic

    inline constexpr vec2<T> operator +(const vec2<T>& v) const {
        return vec2<T>(x + v.x, y + v.y);
    }

    inline constexpr vec2<T> operator -(const vec2<T>& v) const {
        return vec2<T>(x - v.x, y - v.y);
    }

    inline constexpr vec2<T> operator /(const vec2<T>& v) const {
        return vec2<T>(x / v.x, y / v.y);
    }

    inline constexpr vec2<T> operator *(const vec2<T>& v) const {
        return vec2<T>(x * v.x, y * v.y);
    }

    // scalar arithmetic

    inline constexpr vec2<T> operator +(T s) const {
        return vec2<T>(x + s, y + s);
    }

    inline constexpr vec2<T> operator -(T s) const {
        return vec2<T>(x - s, y - s);
    }

    inline constexpr vec2<T> operator *(T s) const {
        return vec2<T>(x * s, y * s);
    }

    inline constexpr vec2<T> operator /(T s) const {
        T d = static_cast<T>(1) / s;
        return vec2<T>(x * d, y * d);
    }

    // compound scalar arithmetic

    inline vec2<T>& operator +=(T s) {
        x += s;
        y += s;
        return *this;
    }

    inline vec2<T>& operator -=(T s) {
        x -= s;
        y -= s;
        return *this;
    }

    inline vec2<T>& operator *=(T s) {
        x *= s;
        y *= s;
        return *this;
    }

    inline vec2<T>& operator /=(T s) {
        x /= s;
        y /= s;
        return *this;
    }

    inline constexpr vec2<T> normalized() {
        T len = length();
        return len > 0 ? *this / len : vec2<T>(0, 0);
    }

    inline vec2<T> normalize() {
        T len = length();
        if (!len) {
            return { 0, 0 };
        }

        T invLen = static_cast<T>(1) / len;
        x *= invLen;
        y *= invLen;

        return *this;
    }

    inline constexpr T dist(const vec2<T>& v) const {
        vec2<T> d(v.x - x, v.y - y);
        return d.length();
    }

    inline constexpr T length() const {
        return std::sqrt(x * x + y * y);
    }

    inline constexpr T length2() const {
        return x * x + y * y;
    }

    inline void truncate(double length) {
        double angle = atan2f(y, x);
        x = length * cos(angle);
        y = length * sin(angle);
    }

    inline constexpr vec2<T> ortho() const {
        return vec2<T>(y, -x);
    }

    inline constexpr vec2<T> transformAngular(T sin, T cos, bool reverse) const {
        return vec2<T>(
            reverse
            ? (x * cos  + y * sin)
            : (x * cos  - y * sin),
            reverse
            ? (y * cos - x * sin)
            : (y * cos + x * sin));
    }

    std::string str() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    inline constexpr static T dot(vec2<T> v1, vec2<T> v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }

    inline constexpr static T cross(vec2<T> v1, vec2<T> v2) {
        return (v1.x * v2.y) - (v1.y * v2.x);
    }
};

typedef vec2<float> vec2f;
typedef vec2<double> vec2d;
