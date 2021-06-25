#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <vector>
#include <tuple>
#include <random>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL_opengles2.h>
#include <GLES3/gl3.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include "../include/interpolators.h"
#include "types.h"
#include "ui.h"
//#include "graphics.h"

struct TextureQuad
{
    static constexpr const char * name = "texture quad";
    static constexpr const char * vert_name = "texture quad vertex shader";
    static constexpr const char * vert =
    "\
        #version 300 es\n\
        in vec2 pos;\n\
        out vec2 tex_coord;\n\
        const vec4 white = vec4(1.0);\n\
        \n\
        void main()\n\
        {\n\
            gl_Position = vec4(pos, 0.0, 1.0);\n\
            tex_coord   = vec2(pos[0] * 0.5 + 0.5, pos[1] * 0.5 + 0.5);\n\
        }\n\
    ";
    
    static constexpr const char * frag_name = "texture quad fragment shader";
    static constexpr const char * frag =
    "\
        #version 300 es\n\
        #ifdef GL_ES\n\
        precision highp float;\n\
        #endif\n\
        in vec2 tex_coord;\n\
        out vec4 fragColour;\n\
        uniform sampler2D tex_sampler;\n\
        \n\
        void main()\n\
        {\n\
            fragColour = texture(tex_sampler, tex_coord);\n\
        }\n\
    ";

    void texture_parameters(GLuint tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
};
DemoList demo;
UserInterface ui;

struct
{
    SDL_Window * window = nullptr;
    SDL_GLContext gl = nullptr;
} sdl;
struct Context
{
    GLuint prog = 0;
    const std::vector<Vec2> screen_quad = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
    GLuint screen_quad_vbo = 0;
    Texture texture;
    GLuint texture_gl = 0;
        std::vector<Interpolator::Demo> demo;
        std::size_t N = 3; // number of demonstrations
        const std::size_t num_interpolators = std::tuple_size_v<decltype(interpolators)>;
        unsigned int w = 500;
        unsigned int h = 500;
        bool redraw = true;
        bool quit = false;
        Scalar grab_dist = 20;
        Interpolator::Demo * grabbed = nullptr;
        std::size_t grabbed_idx = 0;
        Vec2 mouse = {0, 0};
} context;

template<typename ShaderProgram, GLenum shader_type>
GLuint create_shader()
{
    GLuint shader;
    const char * source;
    const char * name;
    if constexpr (shader_type == GL_VERTEX_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = ShaderProgram::vert;
        name = ShaderProgram::vert_name;
    }
    else if constexpr (shader_type == GL_FRAGMENT_SHADER)
    {
        shader = glCreateShader(shader_type);
        source = ShaderProgram::frag;
        name = ShaderProgram::frag_name;
    }
    glShaderSource(shader, 1, (const GLchar**) &source, NULL);
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
GLuint create_vbo(const Vertex * vertices, GLuint numVertices)
{
    GLuint vbo;
    int nBuffers = 1;
    glGenBuffers(nBuffers, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        glDeleteBuffers(nBuffers, &vbo);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "VBO creation failed with code `%u`.\n", err);
        vbo = 0;
    }

    return vbo;
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

void loop()
{
    ui.poll_event_queue(demo);

    if (ui.needs_to_redraw())
    {
        unsigned int i = 0;
        auto draw = [](unsigned int i, auto& tuple) 
                {if (i++ == ui.active_interpolator()) ui.draw(i, tuple, demo);};
        std::apply([&](auto& ... tuples) {((draw(i, tuples)), ...);}, interpolators);
        //for(int row=0; row<ui.texture().rows(); row++)
        //    for(int col=0; col<ui.texture().cols(); col++)
        //        ui._texture(row,col) = RGBAVec(row/((float)ui.texture().rows()), col/((float)ui.texture().cols()), 0, 1);

        write_gl_texture(ui.texture(), context.texture_gl);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, context.screen_quad.size());
    SDL_GL_SwapWindow(sdl.window);
}

int main()
{
    ui = UserInterface{};
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error initializing SDL:\n    %s\n", 
                SDL_GetError());
        return EXIT_FAILURE;
    }
    else SDL_Log("Initialized successfully\n");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    sdl.window = SDL_CreateWindow
            ( "Interpolators"
            , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
            , ui.texture().cols() , ui.texture().rows()
            , SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN
            );
    if (sdl.window == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating window:\n    %s\n", 
                SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                "Couldn't create the main window :(", NULL);
        return EXIT_FAILURE;
    }
    else SDL_Log("Created window\n");

    sdl.gl = SDL_GL_CreateContext(sdl.window);
    if (sdl.gl == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating OpenGL context:\n    %s\n", 
                SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                "Couldn't create OpenGL context :(", NULL);
        return EXIT_FAILURE;
    }
    else SDL_Log("Created GL context\n");

    unsigned int n = 3;
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<Scalar> random(0, 1);
    while(n-- > 0)
    {
        auto v = Vec2{random(generator), random(generator)};
        auto c = RGB_to_JzAzBz(RGBVec{random(generator), random(generator), random(generator)});
        demo.push_back({n, v, c});
    }

    auto resize_lists = [&](auto& tup)
    {
        auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        auto& default_para = std::get<3>(tup);
        meta.resize(demo.size());
        for (auto& m : meta) m = {};
        para.resize(demo.size());
        for (auto& p : para) p = default_para;
    };
    std::apply([&](auto& ... tuples) {((resize_lists(tuples)), ...);}, interpolators);

    context.prog = create_program<TextureQuad>();
    if (not context.prog) return EXIT_FAILURE;
    glUseProgram(context.prog);

    context.screen_quad_vbo = create_vbo(context.screen_quad.data(), context.screen_quad.size());
    if (not context.screen_quad_vbo) return EXIT_FAILURE;

    GLuint positionIdx = 0;
    glBindBuffer(GL_ARRAY_BUFFER, context.screen_quad_vbo);
    glVertexAttribPointer(positionIdx, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (const GLvoid*)0);
    glEnableVertexAttribArray(positionIdx);

    context.texture = Texture(context.h, context.w);
    context.texture_gl = create_gl_texture(context.texture);
    if (not context.texture_gl) return EXIT_FAILURE;

    glUseProgram(context.prog);
    GLint tex_sampler_uniform_location = glGetUniformLocation(context.prog, "tex_sampler");
    if (tex_sampler_uniform_location < 0) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get 'tex_sampler' uniform location.\n");
        return EXIT_FAILURE;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context.texture_gl);
    glUniform1i(tex_sampler_uniform_location, 0);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    while (not ui.ready_to_quit())
    {
        loop();
        SDL_Delay(33);
    }
#endif

    return EXIT_SUCCESS;
}
