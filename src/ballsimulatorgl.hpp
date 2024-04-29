#pragma once

#include "application.hpp"
#include "world.hpp"
#include "config.h"

class BallSimulatorGl final : public Application {
    FpsCalculator fpscalc;

    BallSimulator::World world;

    gfx::Mesh ballMesh, rectMesh, quadMesh;

    std::vector<gfx::Instance> ballInstances, quadInstances;
#ifdef SHOW_QUADTREE_HEATMAP
    std::vector<gfx::Instance> rectInstances;
#endif

    static gfx::Mesh generate_filled_circle(gfx::Renderer& render, float radius = 1.0f);
    static gfx::Mesh generate_filled_rect(gfx::Renderer& render, const Extent<float>& rect = { 0, 0, 1, 1 });

    void render_quadtree_bounds();

    virtual bool init();
    virtual void quit();

    virtual void render(double deltatime);

    virtual void resize(int width, int height);
    virtual void mouse(int button, int action);

public:
    BallSimulatorGl();
    virtual ~BallSimulatorGl() = default;
};

