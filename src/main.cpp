#include "simulator.h"

#include <iostream>
#include <chrono>
#include <thread>

extern "C" {
#include <GLFW/glfw3.h>
}

BallSimulator::World *world;

#define TRIANGLE_AMOUNT 20

static const int TriangleAmount = TRIANGLE_AMOUNT;
static const float TwicePi = 2.0f * 3.1415926f;
static const float CircleMagicConstant = TwicePi / TriangleAmount;

static float CircleDrawingCacheCos[TRIANGLE_AMOUNT + 1] = {};
static float CircleDrawingCacheSin[TRIANGLE_AMOUNT + 1] = {};

static void fulfillDrawingCache() {
    for (auto i = 0; i <= TriangleAmount; i++) {
        CircleDrawingCacheCos[i] = cos(i * CircleMagicConstant);
    }

    for (auto i = 0; i <= TriangleAmount; i++) {
        CircleDrawingCacheSin[i] = sin(i * CircleMagicConstant);
    }
}

void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for(i = 0; i <= TriangleAmount; i++) {
        glVertex2f(
                (GLfloat) (x + (radius * CircleDrawingCacheCos[i])),
                (GLfloat) (y + (radius * CircleDrawingCacheSin[i]))
        );
    }
    glEnd();
}

int lastTimeBase = 0;
int frames = 0;
double fps = 60.0;
double lastFrameTime = 0.0;

void render() {
    frames++;
    auto glTime = glfwGetTime();
    auto elapsed = (int) (glTime * 1000);

    if (elapsed - lastTimeBase > 1000) {
        fps = frames * 1000.0 / (elapsed - lastTimeBase);
        lastTimeBase = elapsed;
        frames = 0;
        std::cout << "FPS: " << fps << std::endl;
    }

    auto micros = glTime * 1000 * 1000;
    auto timeHasPassed = micros - lastFrameTime;
    auto divisor = float(timeHasPassed) / 1000.0f;
    world->tick(divisor);
    lastFrameTime = micros;

    glClear(GL_COLOR_BUFFER_BIT);
    auto entities = world->entities();
    for (auto it = entities->begin(); it != entities->end(); it++) {
        auto ball = (BallSimulator::Ball*) *it;
        auto pos = ball->position();
        if (ball->isInsideCollision) {
            glColor3f(1.0, 1.0, 0.0);
        } else {
            glColor3f(1.0, 0.0, 0.0);
        }
        drawFilledCircle(pos.x, pos.y, ball->radius());
    }
}

void init(GLFWwindow *window) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

void reshape(GLFWwindow *window, int w, int h) {
    std::cout << "Window Size: " << w << "x" << h << std::endl;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);

    world->resize(float(w), float(h));
    world->scatter();
}

void mouse(GLFWwindow *window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        auto ball = new BallSimulator::Ball(5.0f, 20.0f);
        ball->position().set(float(x), float(y));
        ball->velocity().set(10.0f, 10.0f);
        world->entities()->push_back(ball);
    }
}

void handle_error(int code, const char *msg) {
    std::cerr << "GLFW Error: (code = " << code << "): " << msg << std::endl;
}

int main(int argc, char **argv) {
    fulfillDrawingCache();

    srand((unsigned int) std::chrono::duration_cast<std::chrono::seconds>
                       (std::chrono::system_clock::now().time_since_epoch()).count());
    world = new BallSimulator::World(1024, 1024);
    for (auto i = 1; i <= 5; i++) {
        auto ball = new BallSimulator::Ball(5.0f, 20.0f);
        world->entities()->push_back(ball);
    }
    world->scatter();

    if (glfwInit() == 0) {
        return 1;
    }

    GLFWwindow *window;

    glfwSetErrorCallback(handle_error);

    window = glfwCreateWindow(1024, 1024, "Ball Simulation", nullptr, nullptr);
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
