#pragma once

#include "vec2.hpp"


template <typename T>
struct Rectangle {
    T x, y, w, h;

    constexpr Rectangle(T xx, T yy, T ww, T hh): x(xx), y(yy), w(ww), h(hh) {}
    constexpr Rectangle(const vec2<T>& xy, const vec2<T>& wh): x(xy.x), y(xy.y), w(wh.x), h(wh.y) {}
    constexpr Rectangle(const vec2<T>& xy, T wh): x(xy.x), y(xy.y), w(wh), h(wh) {}

    constexpr bool is_inside(const Rectangle<T>& item) const {
        return
            item.x >= x &&
            item.x + item.w <= x + w &&
            item.y >= y &&
            item.y + item.h <= y + h;
    }
};
