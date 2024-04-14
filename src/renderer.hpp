#pragma once

#include "vec2.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace gfx {
    struct Vertex {
        vec2f position;
    };

    struct Color {
        std::uint8_t r, g, b, a;

        constexpr Color(): r(0), g(0), b(0), a(0) {}
        constexpr Color(float r, float g, float b, float a = 1.0) :
            r(static_cast<std::uint8_t>(r * 0xFF)),
            g(static_cast<std::uint8_t>(g * 0xFF)),
            b(static_cast<std::uint8_t>(b * 0xFF)),
            a(static_cast<std::uint8_t>(a * 0xFF)) {}

        static constexpr Color black()   { return Color(0.0f, 0.0f, 0.0f); }
        static constexpr Color red()     { return Color(1.0f, 0.0f, 0.0f); }
        static constexpr Color green()   { return Color(0.0f, 1.0f, 0.0f); }
        static constexpr Color blue()    { return Color(0.0f, 0.0f, 1.0f); }
        static constexpr Color yellow()  { return Color(1.0f, 1.0f, 0.0f); }
        static constexpr Color magenta() { return Color(1.0f, 0.0f, 1.0f); }
        static constexpr Color cyan()    { return Color(0.0f, 1.0f, 1.0f); }
        static constexpr Color white()   { return Color(1.0f, 1.0f, 1.0f); }
    };

    struct Instance {
        vec2f position = vec2f::zero();
        float scale = 1.0f;
        Color color = Color();
    };

    template <typename T>
    struct Rect {
        T x1, y1, x2, y2;
    };

    class Renderer {
    public:
        Renderer(Color clear = Color::black());
        virtual ~Renderer();

        void viewport(int width, int height);

        void newFrame();

        unsigned createMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
        void deleteMesh(unsigned mesh);

        void drawMesh(unsigned mesh, const Instance& instance);
        void drawMesh(unsigned mesh, const Instance* instances, std::size_t numInstance);
        void draw_unfilled_rect(Color color, const Rect<float>& rect);
    };
}
