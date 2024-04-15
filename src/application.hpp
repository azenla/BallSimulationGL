#pragma once

#include "world.hpp"
#include "renderer.hpp"
#include "config.h"
#include <memory>

typedef struct GLFWwindow GLFWwindow;

class Application final {
    GLFWwindow* window = nullptr;
    std::unique_ptr<gfx::Renderer> renderer;

    int frameWidth, frameHeight;
    vec2d contentScale;

    BallSimulator::World world = BallSimulator::World(1024, 1024);
    gfx::Mesh ballMesh, rectMesh, quadMesh;
    int lastTimeBase = 0;
    int frames = 0;
    double fps = 60.0;
    double lastFrameTime = 0.0;

    std::vector<gfx::Instance> ballInstances, quadInstances;
#ifdef SHOW_QUADTREE_HEATMAP
    std::vector<gfx::Instance> rectInstances;
#endif

    static gfx::Mesh generate_filled_circle(gfx::Renderer& render, float radius = 1.0f);
    static gfx::Mesh generate_filled_rect(gfx::Renderer& render, const Extent<float>& rect = { 0, 0, 1, 1 });

    void render_quadtree_bounds();

    bool init();
    void quit();

    void render();

    void resize();
    void mouse(int button, int action);

public:
    Application() = default;
    ~Application() = default;

    int run();
};
