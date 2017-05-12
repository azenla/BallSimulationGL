#pragma once

#include <string>
#include <cmath>

template <class T>
class vec2 {
public:
    T x, y;

    vec2() : x(0), y(0) {
    }

    vec2(T x, T y) : x(x), y(y) {
    }

    vec2(const vec2<T>& v) : x(v.x), y(v.y) {
    }

    vec2<T>& operator =(const vec2<T>& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    vec2<T>& operator +=(vec2<T>& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    vec2<T>& operator -=(vec2<T>& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    vec2<T> operator -() {
        return vec2<T>(-x, -y);
    }

    vec2<T> operator +(vec2<T> v) {
        return vec2<T>(x + v.x, y + v.y);
    }

    vec2<T> operator -(vec2<T> v) {
        return vec2<T>(x - v.x, y - v.y);
    }

    vec2<T> operator /(vec2<T> v) {
        return vec2<T>(x / v.x, y / v.y);
    }

    vec2<T> operator *(vec2<T> v) {
        return vec2<T>(x * v.x, y * v.y);
    }

    vec2<T> operator +(T s) {
        return vec2<T>(x + s, y + s);
    }

    vec2<T> operator -(T s) {
        return vec2<T>(x - s, y - s);
    }

    vec2<T> operator *(T s) {
        return vec2<T>(x * s, y * s);
    }

    vec2<T> operator /(T s) {
        return vec2<T>(x / s, y / s);
    }

    vec2<T>& operator +=(T s) {
        x += s;
        y += s;
        return *this;
    }

    vec2<T>& operator -=(T s) {
        x -= s;
        y -= s;
        return *this;
    }

    vec2<T>& operator *=(T s) {
        x *= s;
        y *= s;
        return *this;
    }

    vec2<T>& operator /=(T s) {
        x /= s;
        y /= s;
        return *this;
    }

    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }

    void set(vec2<T> input) {
        set(input.x, input.y);
    }

    vec2<T> normalize() {
        vec2<T> result;

        auto len = length();
        if (!len) {
            return result;
        }

        result.set(x / len, y / len);

        return result;
    }

    float dist(vec2<T> v) const {
        vec2<T> d(v.x - x, v.y - y);
        return d.length();
    }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    void truncate(double length) {
        double angle = atan2f(y, x);
        x = length * cos(angle);
        y = length * sin(angle);
    }

    vec2<T> ortho() const {
        return vec2<T>(y, -x);
    }

    static T dot(vec2<T> v1, vec2<T> v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }

    static T cross(vec2<T> v1, vec2<T> v2) {
        return (v1.x * v2.y) - (v1.y * v2.x);
    }

    std::string str() {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

typedef vec2<float> vec2f;
typedef vec2<double> vec2d;
