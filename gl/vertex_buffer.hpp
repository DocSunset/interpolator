#pragma once

#include <GLES3/gl3.h>

namespace GL
{
    template<typename Vertex>
    struct VertexBuffer
    {
        GLuint vbo;
        GLuint vao;

        VertexBuffer(GLenum type = GL_STATIC_DRAW)
            : vbo{0}
            , vao{0}
            , _capacity{0}
            , _size{0}
            , _back_buffer{0}
            , _type{type}
        {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);

            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, _back_buffer);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        VertexBuffer(const Vertex * vertices, GLsizeiptr num_vertices, GLenum type = GL_STATIC_DRAW)
        {
            vbo = 0;
            vao = 0;
            _type = type;
    
            update(vertices, num_vertices);
        }

        void update(const Vertex* vertices, GLsizeiptr num_vertices)
        {
            if (_capacity < num_vertices) // grow buffer
            {
                // calculate new capacity
                GLsizeiptr new_capacity = 2 * _capacity;
                new_capacity = new_capacity > num_vertices ? new_capacity : num_vertices;

                // don't modify vao yet
                glBindVertexArray(0);

                // allocate new buffer
                glGenBuffers(1, &_back_buffer);
                glBindBuffer(GL_ARRAY_BUFFER, _back_buffer);
                bufferData(vertices, new_capacity);

                // bind new buffer to vao
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER, _back_buffer);
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                // clean up old buffer
                glDeleteBuffers(1, &vbo);
                vbo = _back_buffer;
                _back_buffer = 0;
            }
            else // update buffer
            {
                glBindVertexArray(vao);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * num_vertices, vertices);
                glBindVertexArray(0);
            }
            _size = num_vertices;
        }

        GLsizeiptr size()
        {
            return _size;
        }
    private:
        GLsizeiptr _size;
        GLsizeiptr _capacity;
        GLuint _back_buffer;
        GLenum _type;

        void bufferData(const Vertex* vertices, GLsizeiptr new_capacity)
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * new_capacity, vertices, _type);
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                glDeleteBuffers(1, &vbo);
                std::cerr << "VBO creation failed with code '" << (unsigned int)err << "'.\n";
                buffer = 0;
            }
            _capacity = new_capacity;
        }
    };
}
