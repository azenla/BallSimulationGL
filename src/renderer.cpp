#include "renderer.hpp"
#include <GLFW/glfw3.h>
#include <cassert>

using namespace gfx;

Renderer::Renderer(const colorf& clear) {
    glClearColor(clear.r, clear.g, clear.b, clear.a);
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
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}


void Renderer::new_frame() {
    glClear(GL_COLOR_BUFFER_BIT);
}

Mesh Renderer::create_mesh(const Span<Vertex> vertices, const Span<uint16_t> indices, PrimitiveType mode) {
    GLenum beginMode;
    switch (mode) {
        case PrimitiveType::POINTS:    beginMode = GL_POINTS; break;
        case PrimitiveType::LINES:     beginMode = GL_LINES; break;
        case PrimitiveType::TRIANGLES: beginMode = GL_TRIANGLES; break;
    }

    GLuint list = glGenLists(1);
    if (list == 0) {
        return Mesh();
    }
    glNewList(list, GL_COMPILE);
        glBegin(beginMode);
            for (auto index : indices) {
                assert(index < vertices.size());
                const Vertex& vertex = vertices[index];
                glVertex2f(vertex.position.x, vertex.position.y);
            }
        glEnd();
    glEndList();
    return Mesh(list);
}

void Renderer::delete_mesh(Mesh& mesh) {
    if (mesh.valid()) {
        glDeleteLists(mesh.get_hnd(), 1);
        mesh = Mesh();
    }
}

static inline void inner_draw_mesh(Mesh mesh, const Instance& instance) {
    glLoadIdentity();
    glTranslatef(
        static_cast<GLfloat>(instance.position.x),
        static_cast<GLfloat>(instance.position.y), 0.0f
    );
    glScalef(
        static_cast<GLfloat>(instance.scale.x),
        static_cast<GLfloat>(instance.scale.y), 1.0f
    );
    colorf fc(instance.color);
    glColor4f(fc.r, fc.g, fc.b, fc.a);
    glCallList(static_cast<GLuint>(mesh.get_hnd()));
}

void Renderer::draw_mesh(Mesh mesh, const Instance& instance) {
    assert(mesh.valid());
    inner_draw_mesh(mesh, instance);
}

void Renderer::draw_mesh(Mesh mesh, const Span<Instance> instances) {
    assert(mesh.valid());
    assert(instances.data());
    for (auto& instance : instances) {
        inner_draw_mesh(mesh, instance);
    }
}
