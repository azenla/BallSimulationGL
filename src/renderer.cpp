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
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}


void Renderer::new_frame() {
    glClear(GL_COLOR_BUFFER_BIT);
}

Mesh Renderer::create_mesh(const Vertex* vertices, std::size_t numVertices,
        const uint16_t* indices, std::size_t numIndices, PrimitiveType mode) {

    GLenum beginMode;
    switch (mode) {
        case PrimitiveType::POINTS:    beginMode = GL_POINTS; break;
        case PrimitiveType::LINES:     beginMode = GL_LINES; break;
        case PrimitiveType::TRIANGLES: beginMode = GL_TRIANGLES; break;
    }

    GLuint list = glGenLists(1);
    if (list == 0) {
        return 0;
    }
    glNewList(list, GL_COMPILE);
        glBegin(beginMode);
            for (std::size_t i = 0; i < numIndices; ++i) {
                uint16_t index = indices[i];
                assert(index < numVertices);
                const Vertex& vertex = vertices[index];
                glVertex2f(vertex.position.x, vertex.position.y);
            }
        glEnd();
    glEndList();
    return static_cast<Mesh>(list);
}

void Renderer::delete_mesh(Mesh mesh) {
    if (mesh != 0u) {
        glDeleteLists(mesh, 1);
    }
}

void Renderer::draw_mesh(Mesh mesh, const Instance& instance) {
    assert(mesh);

    glLoadIdentity();
    glTranslatef(
        static_cast<GLfloat>(instance.position.x),
        static_cast<GLfloat>(instance.position.y), 0.0f);
    glScalef(
        static_cast<GLfloat>(instance.scale.x),
        static_cast<GLfloat>(instance.scale.y), 1.0f);
    glColor4f(
        static_cast<GLfloat>(instance.color.r * colorMul),
        static_cast<GLfloat>(instance.color.g * colorMul),
        static_cast<GLfloat>(instance.color.b * colorMul),
        static_cast<GLfloat>(instance.color.a * colorMul)
    );
    glCallList(static_cast<GLuint>(mesh));
}

void Renderer::draw_meshes(Mesh mesh, const Instance* instances, std::size_t numInstance) {
    assert(instances);
    for (std::size_t i = 0; i < numInstance; ++i) {
        draw_mesh(mesh, instances[i]);
    }
}
