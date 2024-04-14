#include "application.hpp"
#include "simulator.hpp"
#include "world.hpp"
#include "ball.hpp"
#include "renderer.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>

#include <GLFW/glfw3.h>

unsigned Application::generate_filled_circle(gfx::Renderer& render, float radius) {
    constexpr int triangleFanCount = GL_DRAW_CIRCLE_TRIANGLE_AMOUNT;
    constexpr float twoPi = 2.0f * 3.1415926f;
    constexpr float circleSegmentTheta = twoPi / triangleFanCount;

    std::vector<gfx::Vertex> vertices;
    std::vector<uint16_t> indices;
    vertices.reserve(triangleFanCount + 1);
    indices.reserve(triangleFanCount * 3);

    vertices.push_back({ vec2f::zero() });
    vertices.push_back({ vec2f(radius, 0.0f) });
    for (auto i = 1; i < triangleFanCount; i++) {
        vec2f v(
            radius * std::cos(i * circleSegmentTheta),
            radius * std::sin(i * circleSegmentTheta)
        );
        vertices.push_back({ v });
        indices.emplace_back(0);
        indices.emplace_back(i + 1);
        indices.emplace_back(i);
    }
    indices.emplace_back(0);
    indices.emplace_back(1);
    indices.emplace_back(triangleFanCount);

    return render.create_mesh(vertices, indices);
}

void Application::render_quadtree_bounds(gfx::Renderer& render, const BallSimulator::CollisionQuadtree& tree) {
    static auto* pRender = &render;
    static void (*inner)(const BallSimulator::CollisionQuadtree&) =
            [](const BallSimulator::CollisionQuadtree& innerTree) {
        auto bounds = innerTree.bounds();
        gfx::Rect<float> rect = { bounds.x, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h };
        pRender->draw_unfilled_rect(gfx::Color::blue(), rect);
        innerTree.for_each_node<decltype(inner)>(inner);
    };
    inner(tree);
}

void Application::render() {
    frames++;
    const auto currentTime = glfwGetTime();
    auto elapsed = static_cast<int>(currentTime * 1000);

    if (elapsed - lastTimeBase > 1000) {
        fps = frames * 1000.0 / (elapsed - lastTimeBase);
        lastTimeBase = elapsed;
        frames = 0;
        std::cerr << "FPS: " << fps << std::endl;
    }

    double deltaTime = currentTime - lastFrameTime;
#ifdef USE_QUADTREES
    BallSimulator::DoQuadtreeCollisionDetection(world, static_cast<float>(deltaTime));
#else
    BallSimulator::DoSimpleCollisionDetection(world, static_cast<float>(deltaTime));
#endif
    lastFrameTime = currentTime;

    static std::vector<gfx::Instance> ballInstances;
    ballInstances.clear();
    ballInstances.reserve(world.entities().size());

    renderer->new_frame();
    for (auto ball : world.entities()) {
        gfx::Instance instance;
        instance.position = ball->get_position();
        instance.scale = ball->radius();
        if (ball->collisionFlash > 0) {
            instance.color = gfx::Color::yellow();
            --ball->collisionFlash;
        } else {
            instance.color = gfx::Color::red();
        }
        ballInstances.emplace_back(instance);
    }
    renderer->draw_mesh(ballMesh, ballInstances.data(), ballInstances.size());
    render_quadtree_bounds(*renderer, world.quadtree());
}

bool Application::init() {
    srand(static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count()));

    window = glfwCreateWindow(1024, 1024, "Ball Simulation", nullptr, nullptr);
    if (window == nullptr) {
        return false;
    }

    // setup callback handlers
    glfwSetErrorCallback([](int code, const char* msg) {
        std::cerr << "GLFW Error: (code = " << code << "): " << msg << std::endl;
    });

    glfwSetWindowUserPointer(window, this);
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int) -> void {
        void* user = glfwGetWindowUserPointer(window);
        auto& app = *reinterpret_cast<Application*>(user);
        app.mouse(button, action);
    });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h) -> void {
        void* user = glfwGetWindowUserPointer(window);
        auto& app = *reinterpret_cast<Application*>(user);

        app.frameWidth  = w;
        app.frameHeight = h;
        app.resize();
    });

    // setup OpenGL context
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // work out the content scale for scaling mouse input
    const auto calculate_content_size_and_scale = [this]() {
        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
        if (winWidth > 0 && winHeight > 0 && frameWidth > 0 && frameHeight > 0) {
            contentScale = vec2d(frameWidth, frameHeight) / vec2d(winWidth, winHeight);
        }
    };

    // setup rendering
    renderer = std::make_unique<gfx::Renderer>(gfx::Color::black());
    calculate_content_size_and_scale();
    resize();

    // setup world
#ifdef SIMULATION_GRAVITY
    world.change_gravity(SIMULATION_GRAVITY);
#endif
    auto state = 10.0f;
    for (auto i = 1; i <= 200; i++) {
        BallSimulator::Ball ball(3.0f, 10.0f);
        auto stateNegate = -state;
        ball.set_velocity(state, stateNegate);
        world.add(std::move(ball));
        state = -state;
    }
    world.scatter();

    ballMesh = generate_filled_circle(*renderer);
    if (ballMesh == 0) {
        return false;
    }

    return true;
}

void Application::quit() {
    if (renderer) {
        renderer->delete_mesh(ballMesh);
        renderer.reset();  // ensure the renderer is deleted before terminating glfw
    }
}

void Application::resize() {
    std::cerr << "Window Size: " << frameWidth << "x" << frameHeight << std::endl;

    renderer->viewport(frameWidth, frameHeight);
    world.resize(static_cast<float>(frameWidth), static_cast<float>(frameHeight));
    world.scatter();
}

void Application::mouse(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        vec2d cursorPos;
        glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
        cursorPos *= contentScale;

        BallSimulator::Ball ball(5.0f, 20.0f);
        ball.set_position(static_cast<vec2f>(cursorPos));
        ball.set_velocity(vec2f(10.0f, 10.0f));
        world.add(std::move(ball));
    }
}

int Application::run() {
    if (glfwInit() == 0) {
        return 1;
    }

    if (!init()) {
        quit();
        glfwTerminate();
        return 1;
    }

    while (glfwWindowShouldClose(window) == 0) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit();
    glfwTerminate();

    return 0;
}
