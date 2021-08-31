#pragma once

#include <GLES3/gl3.h>

namespace GL::Boilerplate
{
    template<typename Vertex>
    void create_vertex_objects
        ( const Vertex * vertices
        , GLuint numVertices
        , GLuint& vbo
        , GLuint& vao
        , GLenum type = GL_STATIC_DRAW
        )
    {
        vbo = 0;
        vao = 0;
    
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    
        glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
    
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            glDeleteBuffers(1, &vbo);
            std::cerr << "VBO creation failed with code '" << (unsigned int)err << "'.\n";
            vbo = 0;
        }
    }
    
    template<typename Vertex>
    void update_vertex_buffer
        ( const Vertex * vertices
        , GLuint numVertices
        , GLuint vbo
        , GLintptr offset = 0
        )
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vertex) * numVertices, vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
