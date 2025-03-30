#pragma once

#include "renderer.hpp"
#include <memory>

class FpsCalculator {
    double _accumulator;
    int _frames;

public:
    FpsCalculator();
    void frame(double deltaTime, void (*resultCallback)(double fps));
};

typedef struct SDL_Window SDL_Window;
typedef struct SDL_GLContextState* SDL_GLContext;

class Application {
public:
    enum class SwapInterval {
        OFF, VSYNC, ADAPTIVE_VSYNC
    };

    Application(int width, int height, std::string&& title, SwapInterval swap = SwapInterval::VSYNC)
        : _initialWidth(width), _initialHeight(height), _title(std::forward<std::string>(title)), _swap(swap) {};
    virtual ~Application() = default;

    int run();

protected:

    constexpr gfx::Renderer& renderer() { return *_renderer.get(); }

    constexpr Rectangle<int> get_frame() const {
        return { 0, 0, _frameWidth, _frameHeight };
    };

    [[nodiscard]] constexpr vec2d get_cursor_pos() const noexcept { return _cursorPos; }

    virtual bool init() = 0;
    virtual void quit() = 0;

    virtual void render(double deltaTime) = 0;

    virtual void resize(int width, int height);
    virtual void mouse(int button, bool pressed) = 0;

private:
    const std::string _title;
    const int _initialWidth, _initialHeight;
    const SwapInterval _swap;

    SDL_Window* _window = nullptr;
    SDL_GLContext _glCtx = nullptr;
    std::unique_ptr<gfx::Renderer> _renderer;
    int _frameWidth, _frameHeight;
    vec2d _contentScale;
    vec2d _cursorPos;

    uint64_t lastFrameTime;

    bool setup();
    void shutdown();
};
