#include "application.hpp"
#include "renderer.hpp"

#include <GLFW/glfw3.h>
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
    _window = glfwCreateWindow(_initialWidth, _initialHeight, _title.c_str(), nullptr, nullptr);
    if (_window == nullptr) {
        return false;
    }

    // setup callback handlers
    glfwSetErrorCallback([](int code, const char* msg) {
        std::cerr << "GLFW Error: (code = " << code << "): " << msg << std::endl;
    });
    glfwSetWindowUserPointer(_window, this);
    glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int) -> void {
        void* user = glfwGetWindowUserPointer(window);
        auto& app = *reinterpret_cast<Application*>(user);
        app.mouse(button, action);
    });
    glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int w, int h) -> void {
        void* user = glfwGetWindowUserPointer(window);
        auto& app = *reinterpret_cast<Application*>(user);
        app.resize(w, h);
    });

    // setup OpenGL context
    glfwMakeContextCurrent(_window);
    switch (_swap) {
        case SwapInterval::OFF:            glfwSwapInterval(0); break;
        case SwapInterval::VSYNC:          glfwSwapInterval(1); break;
        case SwapInterval::ADAPTIVE_VSYNC: glfwSwapInterval(-1); break;
    }
    
    // work out the content scale for scaling mouse input
    const auto calculate_content_size_and_scale = [this]() {
        int winWidth, winHeight;
        glfwGetWindowSize(_window, &winWidth, &winHeight);
        glfwGetFramebufferSize(_window, &_frameWidth, &_frameHeight);
        if (winWidth > 0 && winHeight > 0 && _frameWidth > 0 && _frameHeight > 0) {
            _contentScale = vec2d(_frameWidth, _frameHeight) / vec2d(winWidth, winHeight);
        }
    };

    // setup rendering
    _renderer = std::make_unique<gfx::Renderer>(gfx::colorf::black());
    calculate_content_size_and_scale();
    _renderer->viewport(_frameWidth, _frameHeight);

    return true;
}


vec2d Application::get_cursor_pos() {
    vec2d cursorPos;
    glfwGetCursorPos(_window, &cursorPos.x, &cursorPos.y);
    return cursorPos * _contentScale;
}

void Application::resize(int width, int height) {
    _frameWidth = width;
    _frameHeight = height;
    _renderer->viewport(width, height);
}


int Application::run() {
    if (glfwInit() == 0) {
        return 1;
    }
    if (!setup()) {
        glfwTerminate();
        return 1;
    }

    // call user init
    if (!init()) {
        quit();
        glfwTerminate();
        return 1;
    }

    // run the main loop
    double timeScaler = 1.0 / static_cast<double>(glfwGetTimerFrequency());
    lastFrameTime = glfwGetTimerValue();
    do {
        const auto currentTime = glfwGetTimerValue();
        const auto deltaTime = timeScaler * static_cast<double>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;
        render(deltaTime);
        glfwSwapBuffers(_window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(_window));

    quit();
    _renderer.reset();  // ensure the renderer is deleted before terminating glfw
    glfwTerminate();

    return 0;
}
