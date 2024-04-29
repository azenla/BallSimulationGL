#include "application.hpp"
#include "simulator.hpp"
#include "world.hpp"
#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>
#include <iostream>

gfx::Mesh Application::generate_filled_circle(gfx::Renderer& render, float radius) {
    constexpr int triangleFanCount = GL_DRAW_CIRCLE_TRIANGLE_AMOUNT;
    constexpr float twoPi = 2.0f * 3.1415926f;
    constexpr float circleSegmentTheta = twoPi / triangleFanCount;

    std::vector<gfx::Vertex> vertices;
    std::vector<uint16_t> indices;
    vertices.reserve(triangleFanCount + 1);
    indices.reserve(triangleFanCount * 3);

    vertices.push_back({ vec2f::zero() });
    vertices.push_back({{ radius, 0.0f }});
    for (auto i = 1; i < triangleFanCount; i++) {
        vertices.push_back({
            vec2f(
                std::cos(i * circleSegmentTheta),
                std::sin(i * circleSegmentTheta)
            ) * radius
        });
        indices.emplace_back(0);
        indices.emplace_back(i + 1);
        indices.emplace_back(i);
    }
    indices.emplace_back(0);
    indices.emplace_back(1);
    indices.emplace_back(triangleFanCount);

    return render.create_mesh(vertices, indices);
}

gfx::Mesh Application::generate_filled_rect(gfx::Renderer& render, const Extent<float>& rect) {
    constexpr std::array<uint16_t, 6> indices{ 0, 1, 2, 2, 3, 0 };
    const gfx::Vertex vertices[4] = {
        { rect.bottom_left() }, { rect.bottom_right() },
        { rect.top_right() }, { rect.top_left() }
    };

    return render.create_mesh(gfx::Span<gfx::Vertex>(vertices, 4), indices);
}

void Application::render() {
    using namespace gfx;

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

    // build instance lists
    ballInstances.reserve(world.entities().size());
    for (auto& ball : world.entities()) {
        Instance instance;
        instance.position = ball->get_position();
        instance.scale = vec2f(ball->radius());
        if (ball->collisionFlash > 0) {
            instance.color = color::yellow();
            --ball->collisionFlash;
        } else {
            instance.color = color::red();
        }
        ballInstances.emplace_back(instance);
    }

    const auto build_quadtree_vis_instances = [this](const BallSimulator::CollisionQuadtree& tree) {
        static auto& quads = quadInstances;
#ifdef SHOW_QUADTREE_HEATMAP
        static auto& rects = rectInstances;
#endif
        static void (*inner)(const BallSimulator::CollisionQuadtree&) =
                [](const BallSimulator::CollisionQuadtree& innerTree) {
            
            const auto& bounds = innerTree.bounds();
            if (innerTree.has_child_nodes()) {
                quads.push_back({
                    .position = bounds.position(),
                    .scale = bounds.size(),
                    .color = color::blue()
                });
                innerTree.for_each_node<decltype(inner)>(inner);
            }
#ifdef SHOW_QUADTREE_HEATMAP
            else if (!innerTree.objects().empty()) {
                auto rescale = [](float x, float lin, float exp) {
                    return std::max(x * lin, x * (1.0f - exp) + x * x * exp);
                };

                float normalisedCount = static_cast<float>(innerTree.objects().size()) / QUADTREE_MAX_OBJECTS;
                float alpha = 0.4f * rescale(normalisedCount, 0.45f, 1.6f);

                rects.push_back({
                    .position = bounds.position(),
                    .scale = bounds.size(),
                    .color = color::black().mix(color::cyan(), alpha)
                });
            }
#endif
        };
        inner(tree);
    };
    build_quadtree_vis_instances(world.quadtree());

    // draw everything
    renderer->new_frame();
#ifdef SHOW_QUADTREE_HEATMAP
    if (!rectInstances.empty()) {
        renderer->draw_mesh(rectMesh, rectInstances);
    }
    rectInstances.clear();
#endif
    if (!ballInstances.empty()) {
        renderer->draw_mesh(ballMesh, ballInstances);
    }
    ballInstances.clear();
    if (!quadInstances.empty()) {
        renderer->draw_mesh(quadMesh, quadInstances);
    }
    quadInstances.clear();
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
    renderer = std::make_unique<gfx::Renderer>(gfx::colorf::black());
    calculate_content_size_and_scale();
    resize();

    // setup world
#ifdef SIMULATION_GRAVITY
    world.set_gravity(SIMULATION_GRAVITY);
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

    // create meshes for rendering
    ballMesh = generate_filled_circle(*renderer);
    rectMesh = generate_filled_rect(*renderer);
    quadMesh = renderer->create_mesh(std::initializer_list<gfx::Vertex>{
        {{ 0.0f, 0.5f }}, {{ 1.0f, 0.5f }},
        {{ 0.5f, 0.0f }}, {{ 0.5f, 1.0f }}
    }, std::initializer_list<uint16_t>{ 0, 1, 2, 3 }, gfx::PrimitiveType::LINES);
    if (!ballMesh.valid() || !rectMesh.valid() || !quadMesh.valid()) {
        return false;
    }

    return true;
}

void Application::quit() {
    if (renderer) {
        renderer->delete_mesh(quadMesh);
        renderer->delete_mesh(rectMesh);
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
        world.add(ball);
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
