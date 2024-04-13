#pragma once

template <typename T>
struct Rectangle {
    float x, y, w, h;
    T value;

    Rectangle(float xx, float yy, float ww, float hh) {
        x = xx;
        y = yy;
        w = ww;
        h = hh;
    }

    Rectangle(float xx, float yy, float ww, float hh, T value) {
        x = xx;
        y = yy;
        w = ww;
        h = hh;

        this->value = value;
    }

    constexpr bool is_inside(const Rectangle<T>& item) const {
        return item.x >= x &&
            item.x + item.w <= x + w &&
            item.y >= y &&
            item.y + item.h <= y + h;
    }

    ~Rectangle() = default;
};
