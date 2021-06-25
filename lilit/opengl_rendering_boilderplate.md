This document contains the helper functions that allow the copious amount of
boilerplate code involved in using OpenGL to be abstracted away slightly.

```cpp
// @+'openGL boilerplate'
template<typename ShaderProgram, GLenum shader_type>
GLuint create_shader()
{
    GLuint shader;
    const char * source;
    const char * name = ShaderProgram::name;
    if constexpr (shader_type == GL_VERTEX_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = ShaderProgram::vert;
    }
    else if constexpr (shader_type == GL_FRAGMENT_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = ShaderProgram::frag;
    }
    glShaderSource(shader, 1, (const GLchar**) &source, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (not compiled)
    {
        if constexpr (shader_type == GL_VERTEX_SHADER)
            std::cerr << "Vertex shader compilation failed: " << name << "\n";
        else std::cerr << "Fragment shader compilation failed: " << name << "\n";
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        GLchar * errLog = (GLchar*)malloc(logLength);
        if (errLog)
        {
            glGetShaderInfoLog(shader, logLength, &logLength, errLog);
            std::cerr << errLog << "\n";
            free(errLog);
        }
        else std::cerr << "Couldn't get shader log.\n";
    
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

template<typename ShaderProgram>
GLuint create_program()
{
    const char * prog_name = ShaderProgram::name;

    GLuint vertShader = create_shader<ShaderProgram, GL_VERTEX_SHADER>();
    if (vertShader == 0) return 0;
    GLuint fragShader = create_shader<ShaderProgram, GL_FRAGMENT_SHADER>();
    if (fragShader == 0)
    {
        glDeleteShader(vertShader);
        return 0;
    }

    GLuint program = glCreateProgram();
    if (not program)
    {
        std::cerr << "Couldn't create shader program: " << prog_name << "\n";
    }
    
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (not linked)
    {
        std::cerr << "Shader linking failed: " << prog_name << "\n";
        GLint logLength = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logLength);
        GLchar * errLog = (GLchar*)malloc(logLength);
        if (errLog)
        {
            glGetProgramInfoLog(program, logLength, &logLength, errLog);
            std::cerr << errLog << "\n";
            free(errLog);
        }
        else std::cerr << "Couldn't get program log.\n";
    
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return program;
}

template<typename Vertex>
void create_vertex_objects(const Vertex * vertices, GLuint numVertices, GLuint& vbo, GLuint& vao)
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

bool write_gl_texture(const Texture& mat, GLuint tex)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F
                , mat.cols(), mat.rows(), 0
                , GL_RGBA , GL_FLOAT, mat.data());
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        glDeleteBuffers(1, &tex);
        std::cerr << "Failed to allocate memory for texture.\n";
        return false;
    }
    return true;
}

GLuint create_gl_texture(const Texture& mat)
{
    GLuint tex;
    glGenTextures(1, &tex);
    if (not write_gl_texture(mat, tex)) return 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}
// @/
```
