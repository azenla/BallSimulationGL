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

    template <typename ElementType>
    struct Span {
        typedef const ElementType element_type;
        typedef element_type* pointer;
        typedef element_type& reference;
        typedef pointer iterator;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

    private:
        pointer const _data;
        const size_type _length;

    public:
        constexpr Span() noexcept: _data(nullptr), _length(0u) {}
        constexpr Span(pointer data, size_type count) : _data(data), _length(count) {}
        constexpr Span(pointer first, pointer last) : _data(first), _length(last - first) {}
        template <typename T, std::size_t N>
        constexpr Span(const std::array<T, N>& array) noexcept : _data(array.data()), _length(N) {}
        template <typename T>
        constexpr Span(const std::initializer_list<T>& list) noexcept : _data(list.begin()), _length(list.size()) {}
        template <typename C>
        constexpr Span(const C& c) : _data(c.data()), _length(c.size()) {}

        constexpr Span(const Span& other) noexcept = default;
        ~Span() noexcept = default;
        constexpr Span& operator =(const Span& other) noexcept = default;

        constexpr reference operator [](size_type idx) const noexcept { return _data[idx]; }

        constexpr pointer data() const noexcept { return _data; }
        constexpr size_type size() const noexcept { return _length; }

        constexpr iterator begin() const noexcept { return data(); }
        constexpr iterator end() const noexcept { return data() + size(); }

    private:
    };

    class Renderer {
    public:
        Renderer(Color clear = Color::black());
        virtual ~Renderer();

        void viewport(int width, int height);

        void new_frame();

        Mesh create_mesh(const Span<Vertex> vertices, const Span<uint16_t> indices,
            PrimitiveType mode = PrimitiveType::TRIANGLES);
        void delete_mesh(unsigned mesh);

        void draw_mesh(Mesh mesh, const Instance& instance);
        void draw_mesh(Mesh mesh, const Span<Instance> instances);
    };
}
