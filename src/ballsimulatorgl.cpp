#include "ballsimulatorgl.hpp"
#include "simulator.hpp"

#include <GLFW/glfw3.h>
#include <chrono>
#include <algorithm>
#include <iostream>

gfx::Mesh BallSimulatorGl::generate_filled_circle(gfx::Renderer& render, float radius) {
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

gfx::Mesh BallSimulatorGl::generate_filled_rect(gfx::Renderer& render, const Extent<float>& rect) {
    constexpr std::array<uint16_t, 6> indices{ 0, 1, 2, 2, 3, 0 };
    const gfx::Vertex vertices[4] = {
        { rect.bottom_left() }, { rect.bottom_right() },
        { rect.top_right() }, { rect.top_left() }
    };

    return render.create_mesh(gfx::Span<gfx::Vertex>(vertices, 4), indices);
}


BallSimulatorGl::BallSimulatorGl() :
    Application(1024, 1024, "Ball Simulation") {
}

bool BallSimulatorGl::init() {
    srand(static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count()));

    // setup world
    world.resize(static_cast<Rectangle<float>>(get_frame()));
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
    ballMesh = generate_filled_circle(renderer());
    rectMesh = generate_filled_rect(renderer());
    quadMesh = renderer().create_mesh(std::initializer_list<gfx::Vertex>{
        {{ 0.0f, 0.5f }}, {{ 1.0f, 0.5f }},
        {{ 0.5f, 0.0f }}, {{ 0.5f, 1.0f }}
    }, std::initializer_list<uint16_t>{ 0, 1, 2, 3 }, gfx::PrimitiveType::LINES);
    if (!ballMesh.valid() || !rectMesh.valid() || !quadMesh.valid()) {
        return false;
    }

    return true;
}

void BallSimulatorGl::quit() {
    renderer().delete_mesh(quadMesh);
    renderer().delete_mesh(rectMesh);
    renderer().delete_mesh(ballMesh);
}

void BallSimulatorGl::render(double deltaTime) {
    using namespace gfx;

    fpscalc.frame(deltaTime, [](double fps) {
        std::cerr << "FPS: " << fps << std::endl;
    });

#ifdef USE_QUADTREES
    BallSimulator::DoQuadtreeCollisionDetection(world, static_cast<float>(deltaTime));
#else
    BallSimulator::DoSimpleCollisionDetection(world, static_cast<float>(deltaTime));
#endif

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
    renderer().new_frame();
#ifdef SHOW_QUADTREE_HEATMAP
    if (!rectInstances.empty()) {
        renderer().draw_mesh(rectMesh, rectInstances);
    }
    rectInstances.clear();
#endif
    if (!ballInstances.empty()) {
        renderer().draw_mesh(ballMesh, ballInstances);
    }
    ballInstances.clear();
    if (!quadInstances.empty()) {
        renderer().draw_mesh(quadMesh, quadInstances);
    }
    quadInstances.clear();
}

void BallSimulatorGl::resize(int width, int height) {
    Application::resize(width, height);
    std::cerr << "Window Size: " << width << "x" << height << std::endl;
    world.resize({ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) });
    world.scatter();
}

void BallSimulatorGl::mouse(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        BallSimulator::Ball ball(5.0f, 20.0f);
        ball.set_position(static_cast<vec2f>(get_cursor_pos()));
        ball.set_velocity(vec2f(10.0f, 10.0f));
        world.add(ball);
    }
}
