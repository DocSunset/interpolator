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
#include "../interpolator/marier_spheres.h"
using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using RGBAVec = Eigen::Vector4f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;
using Texture = Eigen::Matrix<RGBAVec, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using Interpolator = Interpolators<Scalar, ID, Vec2, JzAzBzVec>;
#define INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__})
auto interpolators = std::make_tuple
        ( INTERPOLATOR(Interpolator::IntersectingNSpheres)
        , INTERPOLATOR(Interpolator::InverseDistance, 4, 0.001, 0.0)
        );
RGBVec XYZ_to_RGB(const CIEXYZVec& xyz)
{
    RGBVec rgb;
    Eigen::Matrix3f a;
    a <<  3.24096994, -1.53738318, -0.49861076,
         -0.96924364,  1.8759675,   0.04155506,
          0.05563008, -0.20397696,  1.05697151;

    rgb = a * xyz; // apply linear transformation

    for (unsigned int i = 0; i < 3; i++) // apply gamme correction
    {
        auto u = rgb[i];
        u = u > 0.0031308 ? (1.055 * pow(u, 1 / 2.4) - 0.055) : 12.92 * u;
        u = u < 0.0 ? 0.0 : u;
        u = u > 1.0 ? 1.0 : u;
        rgb[i] = u;
    }

    return rgb;
}

CIEXYZVec RGB_to_XYZ(const RGBVec& rgb)
{
    CIEXYZVec xyz = rgb;
    Eigen::Matrix3f a;
    a <<  0.41239080,  0.35758434,  0.18048079,
          0.21263901,  0.71516868,  0.07219232,
          0.01933082,  0.11919478,  0.95053215;

    for (unsigned int i = 0; i < 3; i++) // reverse gamme correction
    {
        auto u = xyz[i];
        u = u > 0.04045 ? pow((u + 0.055) / 1.055, 2.4) : u / 12.92;
        xyz[i] = u;
    }

    return a * xyz; // reverse linear transformation
}

JzAzBzVec XYZ_to_JzAzBz(const CIEXYZVec& xyz)
{
    static const Eigen::Matrix3f M1 = (Eigen::Matrix3f() <<
        0.41478972,  0.579999,  0.0146480,
        -0.2015100,  1.120649,  0.0531008,
        -0.0166008,  0.264800,  0.6684799).finished();
    static const Eigen::Matrix3f M2 = (Eigen::Matrix3f() <<  
        0.5,         0.5,       0.0,
        3.524000,   -4.066708,  0.542708,
        0.199076,    1.096799, -1.295875).finished();
    constexpr float two5 = 1 << 5;
    constexpr float two7 = 1 << 7;
    constexpr float two12 = 1 << 12;
    constexpr float two14 = 1 << 14;
    constexpr float b = 1.15;
    constexpr float g = 0.66;
    constexpr float c1 = 3424.0 / two12;
    constexpr float c2 = 2413.0 / two7;
    constexpr float c3 = 2392.0 / two7;
    constexpr float n = 2610.0 / two14;
    constexpr float p = 1.7 * 2523.0 / two5;
    constexpr float d = -0.56;
    constexpr float d_0 = 1.6295499532821566e-11;
    auto x = xyz.x();
    auto y = xyz.y();
    auto z = xyz.z();

    // pre-adjust to improve iso-hue linearity (Safdar et al. eqn. 8
    auto x_ = b * x - (b - 1.0) * z;
    auto y_ = g * y - (g - 1.0) * x;
    CIEXYZVec xyz_(x_, y_, z);

    // transform xyz to cone primaries (Safdar et al. eqn. 9)
    Eigen::Vector3f lms = M1 * xyz_;

    // perceptual quantizer (Safdar et al. eqn. 10
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i] / 10000.0, n);
        lms[i] = pow( (c1 + c2 * u) / (1.0 + c3 * u), p );
    }

    // transform to correlates of opponent colour space (Safdar et al. eqn. 11)
    Eigen::Vector3f jab = M2 * lms;

    // improve wide-range lightness prediction (Safdar et al. eqn. 12)
    auto i = jab[0];
    jab[0] = ((1.0 + d) * i) / (1.0 + d * i) - d_0;

    return jab;
}

CIEXYZVec JzAzBz_to_XYZ(const JzAzBzVec& jab)
{
    static const Eigen::Matrix3f M1 = (Eigen::Matrix3f() <<
        0.41478972,  0.579999,  0.0146480,
        -0.2015100,  1.120649,  0.0531008,
        -0.0166008,  0.264800,  0.6684799).finished();
    static const Eigen::Matrix3f M2 = (Eigen::Matrix3f() <<  
        0.5,         0.5,       0.0,
        3.524000,   -4.066708,  0.542708,
        0.199076,    1.096799, -1.295875).finished();
    constexpr float two5 = 1 << 5;
    constexpr float two7 = 1 << 7;
    constexpr float two12 = 1 << 12;
    constexpr float two14 = 1 << 14;
    constexpr float b = 1.15;
    constexpr float g = 0.66;
    constexpr float c1 = 3424.0 / two12;
    constexpr float c2 = 2413.0 / two7;
    constexpr float c3 = 2392.0 / two7;
    constexpr float n = 2610.0 / two14;
    constexpr float p = 1.7 * 2523.0 / two5;
    constexpr float d = -0.56;
    constexpr float d_0 = 1.6295499532821566e-11;
    static const Eigen::Matrix3f M1inv = M1.inverse();
    static const Eigen::Matrix3f M2inv = M2.inverse();

    // eqn. 17
    auto iab = jab;
    auto j = jab[0] + d_0;
    iab[0] = j / (1.0 + d - d * j);

    // eqn. 18
    Eigen::Vector3f lms = M2inv * iab;

    // eqn. 19
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i], 1.0/p);
        lms[i] = 10000.0 * pow( (c1 - u) / (c3 * u - c2), 1.0/n );
    }

    // eqn. 20
    auto xyz_ = M1inv * lms;

    // eqn. 21 - 23
    auto x_ = xyz_[0];
    auto y_ = xyz_[1];
    auto z_ = xyz_[2];
    auto x = (x_ + (b - 1.0) * z_) / b;
    auto y = (y_ + (g - 1.0) * x) / g;
    auto z = z_;

    CIEXYZVec xyz{x, y, z};
    return xyz;
}

JzAzBzVec RGB_to_JzAzBz(const RGBVec& rgb)
{
    return XYZ_to_JzAzBz(RGB_to_XYZ(rgb));
}

RGBVec JzAzBz_to_RGB(const JzAzBzVec& jab)
{
    return XYZ_to_RGB(JzAzBz_to_XYZ(jab));
}

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

struct Context
{
    unsigned int active_interpolator = 0;
    unsigned int C = 0;
    SDL_Window * window = nullptr;
    SDL_GLContext gl = nullptr;
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

template<typename T>
void draw(unsigned int& i, T& tup)
{
    if (i++ != context.active_interpolator) return;

    auto& interpolator = std::get<0>(tup);
    auto& meta = std::get<1>(tup);
    auto& para = std::get<2>(tup);

    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned int col = 0; col < context.texture.cols(); ++col)
    {
        for (unsigned int row = 0; row < context.texture.rows(); ++row)
        {
            RGBVec out = {0, 0, 0};
            auto q = Vec2{col/(Scalar)context.texture.cols(), row/(Scalar)context.texture.rows()};

            JzAzBzVec interpolated_jab{0, 0, 0};
            interpolator.query(q, context.demo, para, meta, interpolated_jab);
            std::cout << interpolated_jab.x() << " " << interpolated_jab.y() << " " << interpolated_jab.z() << std::endl;

            if (context.C) 
            {
                for (unsigned int n = 0; n < context.N; ++n)
                {
                    RGBVec rgb;
                    Scalar w;
                    if (context.grabbed) 
                    {
                        rgb = JzAzBz_to_RGB(context.grabbed->p);
                        w = meta[context.grabbed_idx].w;
                    }
                    else 
                    {
                        rgb = JzAzBz_to_RGB(context.demo[n].p); 
                        w = meta[n].w;
                    }
                    if (w >= 1.0 - std::numeric_limits<Scalar>::min() * 5)
                    {
                        // visualize maximum elevation with inverted colour dots
                        out = (col % 3) + (row % 3) == 0 ? RGBVec{1,1,1} - rgb : rgb;
                    }
                    else
                    {
                        Scalar brightness = std::pow(std::fmod(w * context.C, 1.0f), 8);
                        brightness = brightness * w;
                        out += rgb * brightness;
                    }
                    if (context.grabbed) break;
                }
            }
            else out = JzAzBz_to_RGB(interpolated_jab);

            context.texture(row, col) = RGBAVec{out.x(), out.y(), out.z(), 1};
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    std::cout << i-1 << ": Generated " << context.texture.cols() * context.texture.rows() << " interpolations in " << usec << " microseconds\n" 
            << "About " << 1000000 * context.texture.cols() * context.texture.rows() / usec << " interpolations per second" 
            << std::endl;
}
void cleanup ()
{
    glDeleteTextures(1, &context.texture_gl);
    glDeleteBuffers(1, &context.screen_quad_vbo);
    glDeleteProgram(context.prog);
    SDL_GL_DeleteContext(context.gl);
    SDL_DestroyWindow(context.window);
    SDL_Quit();
}
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

void loop ()
{
    static SDL_Event ev;
    while (SDL_PollEvent(&ev)) switch (ev.type)
    {
    case SDL_QUIT:
    case SDL_APP_TERMINATING:
    case SDL_APP_LOWMEMORY:
        context.quit = true;
        break;

        case SDL_WINDOWEVENT:
            // TODO
            break;

        case SDL_KEYDOWN:
            context.C = 10; 
            context.redraw = true;
            break;

        case SDL_KEYUP:
            context.C = 0; 
            context.redraw = true;
            break;

        case SDL_MOUSEMOTION:
            context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
            if (context.grabbed)
            {
                context.grabbed->s = context.mouse;
    #           ifndef __EMSCRIPTEN__
                context.redraw = true;
    #           endif
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
            {
                Scalar dist, min_dist;
                min_dist = std::numeric_limits<Scalar>::max();
                for (unsigned int n = 0; n < context.N; ++n)
                {
                    auto& d = context.demo[n];
                    dist = (context.mouse - d.s).norm();
                    if (dist < min_dist) 
                    {
                        context.grabbed = &d;
                        context.grabbed_idx = n;
                        min_dist = dist;
                    }
                }
                if (min_dist > context.grab_dist / (Scalar)context.w) context.grabbed = nullptr;
            }
            if (context.C) context.redraw = true;
            break;

        case SDL_MOUSEBUTTONUP:
            context.grabbed = nullptr;
            context.redraw = true;
            break;

        case SDL_MOUSEWHEEL:
            context.active_interpolator = (context.active_interpolator + 1) % context.num_interpolators;
            context.redraw = true;
            break;

    default:
        break;
    }

    if (context.redraw)
    {
        unsigned int i = 0;
        std::apply([&](auto& ... tuples) {((draw(i, tuples)), ...);}, interpolators);
        context.redraw = false;
//        for(int row=0; row<context.texture.rows(); row++)
//            for(int col=0; col<context.texture.cols(); col++)
//                context.texture(row,col) = RGBAVec(row/((float)context.texture.rows()), col/((float)context.texture.cols()), 0, 1);

        write_gl_texture(context.texture, context.texture_gl);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, context.screen_quad.size());
    SDL_GL_SwapWindow(context.window);
}

int main()
{
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
    context.window = SDL_CreateWindow
            ( "Interpolators"
            , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
            , context.w , context.h
            , SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN
            );
    if (context.window == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating window:\n    %s\n", 
                SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                "Couldn't create the main window :(", NULL);
        return EXIT_FAILURE;
    }
    else SDL_Log("Created window\n");

    context.gl = SDL_GL_CreateContext(context.window);
    if (context.gl == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating OpenGL context:\n    %s\n", 
                SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                "Couldn't create OpenGL context :(", NULL);
        return EXIT_FAILURE;
    }
    else SDL_Log("Created GL context\n");

    unsigned int n = context.N;
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<Scalar> random(0, 1);
    while(n-- > 0)
    {
        auto v = Vec2{random(generator), random(generator)};
        auto c = RGB_to_JzAzBz(RGBVec{random(generator), random(generator), random(generator)});
        context.demo.push_back({n, v, c});
    }

    auto resize_lists = [&](auto& tup)
    {
        auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        auto& default_para = std::get<3>(tup);
        meta.resize(context.demo.size());
        for (auto& m : meta) m = {};
        para.resize(context.demo.size());
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

    atexit(cleanup);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    while (not context.quit)
    {
        loop();
        SDL_Delay(33);
    }
#endif

    return EXIT_SUCCESS;
}
