#pragma once

#include <GLES3/gl3.h>

namespace GL::LL
{
    class VertexArray
    {
    private:
        friend class VertexArrayBinding;
        GLuint handle;
    public:
        VertexArray();
        ~VertexArray();
        operator bool() const;
    };

    class VertexArrayBinding
    {
        const VertexArray& v;
    public:
        VertexArrayBinding(const VertexArray& vao);
    };

    inline VertexArrayBinding bind(const VertexArray& vao) {return VertexArrayBinding(vao);}
}
