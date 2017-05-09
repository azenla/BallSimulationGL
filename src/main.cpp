#include "Simulator.h"

#include <iostream>

#include <GL/gl.h>
#include <GL/glut.h>

namespace S = BallSimulator;

S::World *world;

void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i;
    auto triangleAmount = 20;

    auto twicePi = 2.0f * 3.1415926f;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for(i = 0; i <= triangleAmount;i++) {
        glVertex2f(
                (GLfloat) (x + (radius * cos(i * twicePi / triangleAmount))),
                (GLfloat) (y + (radius * sin(i * twicePi / triangleAmount)))
        );
    }
    glEnd();
}

int lastTimeBase = 0;
int frames = 0;
double fps = 60.0;

void render() {
    frames++;
    auto elapsed = glutGet(GLUT_ELAPSED_TIME);

    if (elapsed - lastTimeBase > 1000) {
        fps = frames * 1000.0 / (elapsed - lastTimeBase);
        lastTimeBase = elapsed;
        frames = 0;
        std::cout << "FPS: " << fps << std::endl;
    }

    world->tick(1000.0f / float(fps));

    glClear(GL_COLOR_BUFFER_BIT);
    auto entities = &world->entities();
    for (auto it = entities->begin(); it != entities->end(); it++) {
        auto ball = *it;
        auto pos = ball->position();
        drawFilledCircle(pos.x, pos.y, ball->radius());
    }
    glutSwapBuffers();
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    auto w = glutGet(GLUT_WINDOW_WIDTH);
    auto h = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(0, 0, w, h);
    gluOrtho2D(0, w, h, 0);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    world->resize(w, h);
    world->scatter();
}

void idle() {
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        auto ball = new S::Ball(5.0f, 20.0f);
        ball->position().set(x, y);
        ball->velocity().set(10.0f, 10.0f);
        world->entities().push_back(ball);
    }
}

int main(int argc, char **argv) {
    world = new S::World(1024, 1024);
    for (auto i = 1; i <= 5; i++) {
        auto ball = new S::Ball(5.0f, 20.0f);
        world->entities().push_back(ball);
    }
    world->scatter();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("Ball Simulation");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    init();
    glutDisplayFunc(render);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
