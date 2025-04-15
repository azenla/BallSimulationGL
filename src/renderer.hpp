#pragma once

#include "vec2.hpp"
#include "rectangle.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <limits>
#include <algorithm>
#include <cassert>
#include <type_traits>

namespace gfx {
    struct Vertex {
        vec2f position;

        constexpr Vertex(vec2f&& newpos) noexcept
            : position(std::forward<vec2f>(newpos)) {}
    };

    namespace detail {
        template <typename T> constexpr T unit() noexcept { return T{}; }
        template <> constexpr float unit<float>() noexcept { return 1.0f; }
        template <> constexpr std::uint8_t unit<std::uint8_t>() noexcept { return std::numeric_limits<std::uint8_t>::max(); }
    }

    template <typename T>
    struct Color {
        T r, g, b, a;
        static constexpr T zero() noexcept { return static_cast<T>(0); }
        static constexpr T one() noexcept { return detail::unit<T>(); }

        constexpr Color() noexcept : r(0), g(0), b(0), a(0) {}
        constexpr Color(T rr, T gg, T bb, T aa = one()) noexcept : r(rr), g(gg), b(bb), a(aa) {}
        constexpr Color(const Color<T>& other) noexcept = default;
        constexpr Color(Color<T>&& other) noexcept = default;
        constexpr Color& operator =(const Color<T>& other) noexcept = default;
        ~Color() noexcept = default;

        template <typename A>
        constexpr explicit Color(Color<A> other) noexcept requires(std::is_same<T, std::uint8_t>::value && std::is_same<A, float>::value) :
            r(static_cast<std::uint8_t>(std::min(std::max(other.r * 255.0f, 0.0f), 255.0f))),
            g(static_cast<std::uint8_t>(std::min(std::max(other.g * 255.0f, 0.0f), 255.0f))),
            b(static_cast<std::uint8_t>(std::min(std::max(other.b * 255.0f, 0.0f), 255.0f))),
            a(static_cast<std::uint8_t>(std::min(std::max(other.a * 255.0f, 0.0f), 255.0f))) {}

        template <typename A>
        constexpr explicit Color(Color<A> other) noexcept requires(std::is_same<T, float>::value && std::is_same<A, std::uint8_t>::value) {
            constexpr float colorMul = 1.0f / static_cast<float>(0xFF);
            r = std::min(one(), static_cast<T>(other.r) * colorMul);
            g = std::min(one(), static_cast<T>(other.g) * colorMul);
            b = std::min(one(), static_cast<T>(other.b) * colorMul);
            a = std::min(one(), static_cast<T>(other.a) * colorMul);
        }

        static constexpr Color black() noexcept   { return Color(zero(), zero(), zero()); }
        static constexpr Color red() noexcept     { return Color(one(),  zero(), zero()); }
        static constexpr Color green() noexcept   { return Color(zero(), one(),  zero()); }
        static constexpr Color blue() noexcept    { return Color(zero(), zero(), one()); }
        static constexpr Color yellow() noexcept  { return Color(one(),  one(),  zero()); }
        static constexpr Color magenta() noexcept { return Color(one(),  zero(), one()); }
        static constexpr Color cyan() noexcept    { return Color(zero(), one(),  one()); }
        static constexpr Color white() noexcept   { return Color(one(),  one(),  one()); }

        constexpr Color mix(Color<T>&& other, float x) noexcept {
            auto interp = std::min(std::max(x, 0.0f), 1.0f);
            return Color<T>(
                static_cast<T>(static_cast<float>(r) * (1.0f - interp) + static_cast<float>(other.r) * interp),
                static_cast<T>(static_cast<float>(g) * (1.0f - interp) + static_cast<float>(other.g) * interp),
                static_cast<T>(static_cast<float>(b) * (1.0f - interp) + static_cast<float>(other.b) * interp),
                static_cast<T>(static_cast<float>(a) * (1.0f - interp) + static_cast<float>(other.a) * interp)
            );
        }
    };

    typedef Color<std::uint8_t> color;
    typedef Color<float> colorf;

    struct Instance {
        vec2f position = vec2f::zero();
        vec2f scale = vec2f::one();
        color color = color::white();
    };

    enum class PrimitiveType {
        POINTS, LINES, TRIANGLES
    };

    struct Mesh {
        Mesh() noexcept : _valid(false) {};
        constexpr explicit Mesh(PrimitiveType mode, unsigned vbo, unsigned ibo, int numindices) noexcept
            : _valid(true), _mode(mode), _hnd{ vbo, ibo }, _numindices(numindices) {}

        static constexpr int VERTEX = 0, ELEMENT = 1;

        constexpr bool valid() const noexcept {
            return _valid;
        }

        constexpr PrimitiveType mode() const noexcept {
            return _mode;
        }

        constexpr unsigned get_buffer(int type) noexcept {
            assert(type >= VERTEX && type <= ELEMENT);
            return _hnd[type];
        }

        constexpr int element_count() const noexcept {
            return _numindices;
        }

    private:
        bool _valid = false;
        PrimitiveType _mode;
        unsigned _hnd[2];
        int _numindices;
    };

    class Renderer {
    public:
        Renderer(const colorf& clear = colorf::black());
        virtual ~Renderer();

        void viewport(int width, int height);

        void new_frame();

        Mesh create_mesh(std::span<const Vertex> vertices, std::span<const uint16_t> indices,
            PrimitiveType mode = PrimitiveType::TRIANGLES);
        void delete_mesh(Mesh& mesh);

        void draw_mesh(Mesh& mesh, const Instance& instance);
        void draw_mesh(Mesh& mesh, const std::span<const Instance> instances);
    };
}
