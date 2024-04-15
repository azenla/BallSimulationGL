#pragma once

#include "vec2.hpp"
#include "rectangle.hpp"
#include <cstddef>
#include <cstdint>

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
        vec2f scale = vec2f::one();
        Color color = Color();
    };

    enum class PrimitiveType {
        POINTS, LINES, TRIANGLES
    };

    typedef unsigned Mesh;

    class Renderer {
    public:
        Renderer(Color clear = Color::black());
        virtual ~Renderer();

        void viewport(int width, int height);

        void new_frame();

        template <typename VertContainer, typename IdxContainer>
        constexpr Mesh create_mesh(VertContainer&& vertices, IdxContainer&& indices,
                PrimitiveType mode = PrimitiveType::TRIANGLES) {
            return create_mesh(vertices.data(), vertices.size(), indices.data(), indices.size(), mode);
        }
        Mesh create_mesh(
            const Vertex* vertices, std::size_t numVertices,
            const uint16_t* indices, std::size_t numIndices,
            PrimitiveType mode);
        void delete_mesh(unsigned mesh);

        void draw_mesh(Mesh mesh, const Instance& instance);
        template <typename InstanceContainer>
        constexpr void draw_meshes(Mesh mesh, InstanceContainer&& instances) {
            draw_meshes(mesh, instances.data(), instances.size());
        }
        void draw_meshes(Mesh mesh, const Instance* instances, std::size_t numInstance);
    };
}
