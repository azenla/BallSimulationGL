#include "application.hpp"
#include "renderer.hpp"

#include "gl.h"
#include <SDL3/SDL.h>
#include <cmath>
#include <iostream>

FpsCalculator::FpsCalculator() {
    _accumulator = 0.0;
    _frames = 0;
}

void FpsCalculator::frame(double deltaTime, void (*resultCallback)(double fps)) {
    _frames++;
    _accumulator += deltaTime;

    if (_accumulator > 1.0) {
        int elapsed = static_cast<int>(_accumulator * 1000);
        double fps = _frames / _accumulator;
        _frames = 0;
        _accumulator = std::fmod(_accumulator, 1.0);
        resultCallback(fps);
    }
}


bool Application::setup() {
    // create main window
    const SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    _window = SDL_CreateWindow(_title.c_str(), _initialWidth, _initialHeight, flags);
    if (_window == nullptr) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // setup OpenGL context
    _glCtx = SDL_GL_CreateContext(_window);
    if (_glCtx == nullptr || !SDL_GL_MakeCurrent(_window, _glCtx)) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    switch (_swap) {
        case SwapInterval::OFF:            SDL_GL_SetSwapInterval(0); break;
        case SwapInterval::ADAPTIVE_VSYNC: if (SDL_GL_SetSwapInterval(-1)) { break; }
        // fallthrough to vsync if adaptive is not available
        case SwapInterval::VSYNC:          SDL_GL_SetSwapInterval(1); break;
    }

    if (gladLoadGL(SDL_GL_GetProcAddress) == 0) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return false;
    }

    // work out the content scale for scaling mouse input
    const auto calculate_content_size_and_scale = [this]() {
        int winWidth, winHeight;
        SDL_GetWindowSize(_window, &winWidth, &winHeight);
        SDL_GetWindowSizeInPixels(_window, &_frameWidth, &_frameHeight);
        if (winWidth > 0 && winHeight > 0 && _frameWidth > 0 && _frameHeight > 0) {
            _contentScale = vec2d(_frameWidth, _frameHeight) / vec2d(winWidth, winHeight);
        }
    };

    // setup rendering
    _renderer = std::make_unique<gfx::Renderer>(gfx::colorf::black());
    calculate_content_size_and_scale();
    _renderer->viewport(_frameWidth, _frameHeight);

    _cursorPos = vec2d::zero();

    return true;
}

void Application::shutdown() {
    _renderer.reset();  // ensure the renderer is deleted before deleting context
    if (_glCtx != nullptr) {
        SDL_GL_MakeCurrent(nullptr, nullptr);
        SDL_GL_DestroyContext(_glCtx);
        _glCtx = nullptr;
    }
    if (_window != nullptr) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
    SDL_Quit();
}


void Application::resize(int width, int height) {
    _frameWidth = width;
    _frameHeight = height;
    _renderer->viewport(width, height);
}


int Application::run() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!setup()) {
        shutdown();
        return 1;
    }

    // call user init
    if (!init()) {
        quit();
        shutdown();
        return 1;
    }

    // run the main loop
    lastFrameTime = SDL_GetPerformanceCounter();
    bool shouldClose = false;
    do {
        // process the event loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                shouldClose = true;
                break;

            case SDL_EVENT_MOUSE_MOTION:
                _cursorPos = static_cast<vec2d>(vec2f(event.motion.x, event.motion.y));
                _cursorPos *= _contentScale;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:   mouse(MouseButton::LEFT, event.button.down); break;
                case SDL_BUTTON_MIDDLE: mouse(MouseButton::MIDDLE, event.button.down); break;
                case SDL_BUTTON_RIGHT:  mouse(MouseButton::RIGHT, event.button.down); break;
                default: break;
                }
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                resize(event.window.data1, event.window.data2);
                break;
            }
        }
    
        const auto timeDivisor = static_cast<double>(SDL_GetPerformanceFrequency());
        const auto currentTime = SDL_GetPerformanceCounter();
        const auto deltaTime = static_cast<double>(currentTime - lastFrameTime) / timeDivisor;
        lastFrameTime = currentTime;
        render(deltaTime);
        SDL_GL_SwapWindow(_window);
    } while (!shouldClose);

    quit();
    shutdown();

    return 0;
}
