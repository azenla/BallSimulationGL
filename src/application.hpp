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

typedef struct GLFWwindow GLFWwindow;

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

    vec2d get_cursor_pos();

    virtual bool init() {};
    virtual void quit() {};

    virtual void render(double deltaTime) {};

    virtual void resize(int width, int height);
    virtual void mouse(int button, int action) {};

private:
    const std::string _title;
    const int _initialWidth, _initialHeight;
    const SwapInterval _swap;

    GLFWwindow* _window = nullptr;
    std::unique_ptr<gfx::Renderer> _renderer;
    int _frameWidth, _frameHeight;
    vec2d _contentScale;

    uint64_t lastFrameTime;

    bool setup();
};
