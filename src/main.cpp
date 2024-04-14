#include "simulator.hpp"
#include "world.hpp"
#include "ball.hpp"
#include "renderer.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>

#include <GLFW/glfw3.h>

static std::unique_ptr<BallSimulator::World> world;

void generate_filled_circle(std::vector<gfx::Vertex>& vertOut, std::vector<uint16_t>& idxOut, float radius = 1.0f) {
    constexpr int triangleFanCount = GL_DRAW_CIRCLE_TRIANGLE_AMOUNT;
    constexpr float twoPi = 2.0f * 3.1415926f;
    constexpr float circleSegmentTheta = twoPi / triangleFanCount;

    vertOut.reserve(triangleFanCount + 1);
    idxOut.reserve(triangleFanCount * 3);

    vertOut.push_back({ vec2f::zero() });
    vertOut.push_back({ vec2f(radius, 0.0f) });
    for (auto i = 1; i < triangleFanCount; i++) {
        vec2f v(
            radius * std::cos(i * circleSegmentTheta),
            radius * std::sin(i * circleSegmentTheta)
        );
        vertOut.push_back({ v });
        idxOut.emplace_back(0);
        idxOut.emplace_back(i + 1);
        idxOut.emplace_back(i);
    }
    idxOut.emplace_back(0);
    idxOut.emplace_back(1);
    idxOut.emplace_back(triangleFanCount);
}

unsigned ballMesh = 0;
int lastTimeBase = 0;
int frames = 0;
double fps = 60.0;
double lastFrameTime = 0.0;

void render_quadtree_bounds(gfx::Renderer& render, const BallSimulator::CollisionQuadtree& tree) {
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

void render(gfx::Renderer& render) {
    frames++;
    const auto currentTime = glfwGetTime();
    auto elapsed = static_cast<int>(currentTime * 1000);

    if (elapsed - lastTimeBase > 1000) {
        fps = frames * 1000.0 / (elapsed - lastTimeBase);
        lastTimeBase = elapsed;
        frames = 0;
        std::cout << "FPS: " << fps << std::endl;
    }

    double deltaTime = currentTime - lastFrameTime;
    BallSimulator::DoQuadtreeCollisionDetection(*world, static_cast<float>(deltaTime));
    lastFrameTime = currentTime;

    static std::vector<gfx::Instance> ballInstances;
    ballInstances.clear();
    ballInstances.reserve(world->entities().size());

    render.newFrame();
    for (auto ball : world->entities()) {
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
    render.drawMesh(ballMesh, ballInstances.data(), ballInstances.size());
    render_quadtree_bounds(render, world->quadtree());
}

void init(GLFWwindow* window, gfx::Renderer& render) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    std::vector<gfx::Vertex> ballVerts;
    std::vector<uint16_t> ballIndices;
    generate_filled_circle(ballVerts, ballIndices);
    ballMesh = render.createMesh(ballVerts, ballIndices);
}

void reshape(GLFWwindow* window, int w, int h) {
    std::cout << "Window Size: " << w << "x" << h << std::endl;

    auto& render = *reinterpret_cast<gfx::Renderer*>(glfwGetWindowUserPointer(window));
    render.viewport(w, h);
    world->resize(static_cast<float>(w), static_cast<float>(h));
    world->scatter();
}

static vec2d contentScale = { 1, 1 };

void mouse(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        vec2d cursorPos;
        glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
        cursorPos *= contentScale;

        BallSimulator::Ball ball(5.0f, 20.0f);
        ball.set_position(static_cast<vec2f>(cursorPos));
        ball.set_velocity(vec2f(10.0f, 10.0f));
        world->add(std::move(ball));
    }
}

void handle_error(int code, const char* msg) {
    std::cerr << "GLFW Error: (code = " << code << "): " << msg << std::endl;
}

int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count()));

    world = std::make_unique<BallSimulator::World>(1024, 1024);
#ifdef SIMULATION_GRAVITY
    world->change_gravity(SIMULATION_GRAVITY);
#endif
    auto state = 10.0f;
    for (auto i = 1; i <= 200; i++) {
        auto ball = new BallSimulator::Ball(3.0f, 10.0f);
        auto stateNegate = -state;
        ball->set_velocity(state, stateNegate);
        world->add(ball);
        state = -state;
    }
    world->scatter();

    if (glfwInit() == 0) {
        return 1;
    }

    glfwSetErrorCallback(handle_error);

    auto window = glfwCreateWindow(1024, 1024, "Ball Simulation", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }

    // work out the content scale for scaling mouse input
    {
        int winWidth, winHeight, frameWidth, frameHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
        if (winWidth > 0 && winHeight > 0 && frameWidth > 0 && frameHeight > 0) {
            contentScale = {
                static_cast<double>(frameWidth) / static_cast<double>(winWidth),
                static_cast<double>(frameHeight) / static_cast<double>(winHeight)
            };
        }
    }

    glfwSetMouseButtonCallback(window, mouse);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    auto renderer = std::make_unique<gfx::Renderer>(gfx::Color::black());
    glfwSetWindowUserPointer(window, &renderer);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    reshape(window, w, h);
    glfwSetFramebufferSizeCallback(window, reshape);

    init(window, *renderer);

    while (glfwWindowShouldClose(window) == 0) {
        render(*renderer);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer->deleteMesh(ballMesh);
    renderer.reset();  // ensure the renderer is deleted before terminating glfw

    glfwTerminate();

    return 0;
}
