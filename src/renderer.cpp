#include "renderer.hpp"
#include <GLFW/glfw3.h>
#include <cassert>

using namespace gfx;

static constexpr float colorMul = 1.0f / static_cast<float>(0xFF);

Renderer::Renderer(Color clear) {
    glClearColor(
        static_cast<GLfloat>(clear.r * colorMul),
        static_cast<GLfloat>(clear.g * colorMul),
        static_cast<GLfloat>(clear.b * colorMul),
        static_cast<GLfloat>(clear.a * colorMul)
    );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

Renderer::~Renderer() {
}


void Renderer::viewport(int width, int height) {
    glViewport(
        static_cast<GLint>(0),
        static_cast<GLint>(0),
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}


void Renderer::newFrame() {
    glClear(GL_COLOR_BUFFER_BIT);
}

unsigned Renderer::createMesh(const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) {
    GLuint list = glGenLists(1);
    if (list == 0) {
        return 0;
    }
    glNewList(list, GL_COMPILE);
        glBegin(GL_TRIANGLES);
            for (auto index : indices) {
                assert(index < vertices.size());
                const Vertex& vertex = vertices[index];
                glVertex2f(vertex.position.x, vertex.position.y);
            }
        glEnd();
    glEndList();
    return static_cast<unsigned>(list);
}

void Renderer::deleteMesh(unsigned mesh) {
    assert(mesh);
    glDeleteLists(mesh, 1);
}


static void glColor(Color color) {
    glColor4f(
        static_cast<GLfloat>(color.r * colorMul),
        static_cast<GLfloat>(color.g * colorMul),
        static_cast<GLfloat>(color.b * colorMul),
        static_cast<GLfloat>(color.a * colorMul)
    );
}

void Renderer::drawMesh(unsigned mesh, const Instance& instance) {
    assert(mesh);

    GLfloat x = instance.position.x;
    GLfloat y = instance.position.y;
    GLfloat scale = instance.scale * 1.0f;

    glLoadIdentity();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, scale);
    glColor(instance.color);
    glCallList(static_cast<GLuint>(mesh));
}

void Renderer::drawMesh(unsigned mesh, const Instance* instances, std::size_t numInstance) {
    assert(instances);
    for (std::size_t i = 0; i < numInstance; ++i) {
        drawMesh(mesh, instances[i]);
    }
}


void Renderer::draw_unfilled_rect(Color color, const Rect<float>& rect) {
    glLoadIdentity();
    glBegin(GL_LINE_LOOP);
        glColor(color);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x2, rect.y1);
        glVertex2f(rect.x2, rect.y2);
        glVertex2f(rect.x1, rect.y2);
    glEnd();
}
