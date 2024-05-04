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

Mesh Renderer::create_mesh(std::span<const Vertex> vertices, std::span<const uint16_t> indices, PrimitiveType mode) {
    GLuint bufferIds[2];
    glGenBuffers(2, bufferIds);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIds[Mesh::VERTEX]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIds[Mesh::ELEMENT]);

    glBufferData(GL_ARRAY_BUFFER,
        sizeof(Vertex) * vertices.size(),
        vertices.data(),
        GL_STATIC_DRAW
    );
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(uint16_t) * indices.size(),
        indices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    return Mesh(mode, bufferIds[Mesh::VERTEX], bufferIds[Mesh::ELEMENT], static_cast<int32_t>(indices.size()));
}

void Renderer::delete_mesh(Mesh& mesh) {
    if (mesh.valid()) {
        GLuint bufferIds[2] = { mesh.get_buffer(Mesh::VERTEX), mesh.get_buffer(Mesh::ELEMENT) };
        glDeleteBuffers(2, bufferIds);
        mesh = Mesh();
    }
}


static inline GLenum gl_draw_mode(const Mesh& mesh) {
    switch (mesh.mode()) {
        case PrimitiveType::POINTS:    return GL_POINTS;
        case PrimitiveType::LINES:     return GL_LINES;
        case PrimitiveType::TRIANGLES: return GL_TRIANGLES;
    }
}

static inline void gl_inner_bind_mesh(Mesh& mesh) {
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER,
        static_cast<GLuint>(mesh.get_buffer(Mesh::VERTEX))
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLuint>(mesh.get_buffer(Mesh::ELEMENT))
    );

    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), static_cast<GLvoid*>(0));
}

static inline void gl_inner_draw_mesh(GLenum mode, GLsizei numelements, const Instance& instance) {

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

    glDrawElements(mode, numelements, GL_UNSIGNED_SHORT, static_cast<GLvoid*>(0));
}

static inline void gl_inner_unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void Renderer::draw_mesh(Mesh& mesh, const Instance& instance) {
    assert(mesh.valid());

    auto drawmode = gl_draw_mode(mesh);
    auto numelements = static_cast<GLsizei>(mesh.element_count());

    gl_inner_bind_mesh(mesh);
    gl_inner_draw_mesh(drawmode, numelements, instance);
    gl_inner_unbind();
}

void Renderer::draw_mesh(Mesh& mesh, std::span<const Instance> instances) {
    assert(mesh.valid());
    assert(instances.data());

    auto drawmode = gl_draw_mode(mesh);
    auto numelements = static_cast<GLsizei>(mesh.element_count());

    gl_inner_bind_mesh(mesh);
    for (auto& instance : instances) {
        gl_inner_draw_mesh(drawmode, numelements, instance);
    }
    gl_inner_unbind();
}
