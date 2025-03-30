#pragma once

#include <string>
#include <cmath>

template <class T>
class vec2 {
public:
    T x, y;

    constexpr vec2() : x(0), y(0) {}

    constexpr vec2(T x, T y) : x(x), y(y) {}

    constexpr explicit vec2(T v) : x(v), y(v) {}

    constexpr vec2(const vec2<T>& v) : x(v.x), y(v.y) {}

    constexpr vec2(vec2<T>&& v) : x(v.x), y(v.y) {}

    template <typename F>
    constexpr vec2(const vec2<F>& v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) {}

    // constants

    static constexpr vec2<T> zero() { return { static_cast<T>(0), static_cast<T>(0) }; }
    static constexpr vec2<T> one() { return { static_cast<T>(1), static_cast<T>(1) }; }

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

    inline constexpr T length() const {
        return std::sqrt(x * x + y * y);
    }

    inline constexpr T length2() const {
        return x * x + y * y;
    }

    std::string str() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    inline constexpr static T dot(vec2<T> v1, vec2<T> v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }
};

typedef vec2<float> vec2f;
typedef vec2<double> vec2d;
