#include "simulator.h"

#include <iostream>
#include <chrono>
#include <thread>

extern "C" {
    #include <GLFW/glfw3.h>
}

BallSimulator::World* world;

static const int TriangleAmount = GL_DRAW_CIRCLE_TRIANGLE_AMOUNT;
static const float TwicePi = 2.0f * 3.1415926f;
static const float CircleMagicConstant = TwicePi / TriangleAmount;

static float CircleDrawingCacheCos[GL_DRAW_CIRCLE_TRIANGLE_AMOUNT + 1] = {};
static float CircleDrawingCacheSin[GL_DRAW_CIRCLE_TRIANGLE_AMOUNT + 1] = {};

static void fulfill_drawing_cache() {
    for (auto i = 0; i <= TriangleAmount; i++) {
        CircleDrawingCacheCos[i] = cos(i * CircleMagicConstant);
    }

    for (auto i = 0; i <= TriangleAmount; i++) {
        CircleDrawingCacheSin[i] = sin(i * CircleMagicConstant);
    }
}

void draw_filled_circle(GLfloat x, GLfloat y, GLfloat radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (auto i = 0; i <= TriangleAmount; i++) {
        glVertex2f(
            static_cast<GLfloat>(x + radius * CircleDrawingCacheCos[i]),
            static_cast<GLfloat>(y + radius * CircleDrawingCacheSin[i])
        );
    }
    glEnd();
}

void draw_unfilled_rect(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

int lastTimeBase = 0;
int frames = 0;
double fps = 60.0;
double lastFrameTime = 0.0;

void render_quadtree_bounds(const BallSimulator::CollisionQuadtree& tree) {
    auto bounds = tree.bounds();

    glColor3f(0.0f, 0.0f, 1.0f);
    draw_unfilled_rect(bounds.x, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h);

    tree.for_each_node(render_quadtree_bounds);
}

void render() {
    frames++;
    auto glTime = glfwGetTime();
    auto elapsed = static_cast<int>(glTime * 1000);

    if (elapsed - lastTimeBase > 1000) {
        fps = frames * 1000.0 / (elapsed - lastTimeBase);
        lastTimeBase = elapsed;
        frames = 0;
        std::cout << "FPS: " << fps << std::endl;
    }

    auto micros = glTime * 1000 * 1000;
    auto timeHasPassed = micros - lastFrameTime;
    auto divisor = float(timeHasPassed) / 1000.0f / 2;
    world->tick(divisor);
    lastFrameTime = micros;

    glClear(GL_COLOR_BUFFER_BIT);
    for (auto ball : world->entities()) {
        vec2f pos = ball->get_position();
        if (ball->isInsideCollision) {
            glColor3f(1.0, 1.0, 0.0);
        } else {
            glColor3f(1.0, 0.0, 0.0);
        }
        draw_filled_circle(pos.x, pos.y, ball->radius());
    }

    render_quadtree_bounds(world->quadtree());
}

void init(GLFWwindow* window) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

void reshape(GLFWwindow* window, int w, int h) {
    std::cout << "Window Size: " << w << "x" << h << std::endl;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);

    world->resize(float(w), float(h));
    world->scatter();
}

void mouse(GLFWwindow* window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        auto ball = new BallSimulator::Ball(5.0f, 20.0f);
        ball->set_position(vec2f(x, y));
        ball->set_velocity(vec2f(10.0f, 10.0f));
        world->add(ball);
    }
}

void handle_error(int code, const char* msg) {
    std::cerr << "GLFW Error: (code = " << code << "): " << msg << std::endl;
}

int main(int argc, char** argv) {
    fulfill_drawing_cache();

    srand(static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count()));
    world = new BallSimulator::World(1024, 1024);
    world->change_gravity(0.0f);
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

    glfwSetMouseButtonCallback(window, mouse);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    reshape(window, w, h);
    glfwSetFramebufferSizeCallback(window, reshape);

    init(window);

    while (glfwWindowShouldClose(window) == 0) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
