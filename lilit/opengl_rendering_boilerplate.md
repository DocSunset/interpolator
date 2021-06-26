This document contains the helper functions that allow the copious amount of
boilerplate code involved in using OpenGL to be abstracted away slightly.

```cpp
// @+'openGL boilerplate'
std::string load_file(const char * filename)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(filename);
    std::stringstream shader_stream;
    shader_stream << file.rdbuf();
    file.close();
    return shader_stream.str();
}

GLuint create_shader(const char * name, GLenum shader_type, const GLchar ** source, std::size_t sources)
{
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, sources, source, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (not compiled)
    {
        if (shader_type == GL_VERTEX_SHADER)
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

GLuint create_program(const char * name, GLuint vert, GLuint frag)
{
    assert(vert != 0);
    assert(frag != 0);

    GLuint program = glCreateProgram();
    if (not program)
    {
        std::cerr << "Couldn't create shader program: " << name << "\n";
    }
    
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (not linked)
    {
        std::cerr << "Shader linking failed: " << name << "\n";
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

    glDeleteShader(vert);
    glDeleteShader(frag);
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
