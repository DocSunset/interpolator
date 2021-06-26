#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct TextureQuad
{
    static constexpr const char * name = "texture quad";
    static constexpr const char * vert_name = "texture quad vertex shader";
    static constexpr const char * vert = "demo/shaders/position_passthrough.vert";
    
    static constexpr const char * frag_name = "texture quad fragment shader";
    static constexpr const char * frag = "demo/shaders/texture_quad.frag";

    void texture_parameters(GLuint tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
};
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

template<typename ShaderProgram, GLenum shader_type>
GLuint create_shader()
{
    GLuint shader;
    std::string source;
    const char * src;
    const char * name = ShaderProgram::name;
    if constexpr (shader_type == GL_VERTEX_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = load_file(ShaderProgram::vert);
        src = source.c_str();
    }
    else if constexpr (shader_type == GL_FRAGMENT_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = load_file(ShaderProgram::frag);
        src = source.c_str();
    }
    glShaderSource(shader, 1, (const GLchar**) &src, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (not compiled)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader compilation failed: %s.\n", name);
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        GLchar * errLog = (GLchar*)malloc(logLength);
        if (errLog)
        {
            glGetShaderInfoLog(shader, logLength, &logLength, errLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", errLog);
            free(errLog);
        }
        else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get shader log.\n");
    
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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create shader program: %s.\n", prog_name);
    }
    
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (not linked)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader linking failed: %s.\n", prog_name);
        GLint logLength = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logLength);
        GLchar * errLog = (GLchar*)malloc(logLength);
        if (errLog)
        {
            glGetProgramInfoLog(program, logLength, &logLength, errLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", errLog);
            free(errLog);
        }
        else SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get program log.\n");
    
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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "VBO creation failed with code `%u`.\n", err);
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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate memory for texture.\n");
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
struct Fullscreen
{
    static const std::vector<Vec2> quad;
    static GLuint vbo;
    static GLuint vao;
    static GLuint idx;
};
const std::vector<Vec2> Fullscreen::quad = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
GLuint Fullscreen::vbo = 0;
GLuint Fullscreen::vao = 0;
GLuint Fullscreen::idx = 0;

#endif
