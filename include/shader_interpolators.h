#ifndef SHADER_INTERPOLATORS_H
#define SHADER_INTERPOLATORS_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <Eigen/Core>
#include <GLES3/gl3.h>

namespace ShaderInterpolators
{
    using Vec2 = Eigen::Vector2f;
    using RGBAVec = Eigen::Vector4f;
    using Texture = Eigen::Matrix<RGBAVec, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

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

    struct Fullscreen
    {
        static const std::vector<Vec2> quad;
        static GLuint vbo;
        static GLuint vao;
        static GLuint idx;
        static bool initialized;
    
        static void init()
        {
            if (initialized) return;
            create_vertex_objects(quad.data(), quad.size(), vbo, vao);
            glBindBuffer(GL_ARRAY_BUFFER, Fullscreen::vbo);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (const GLvoid*)0);
            glEnableVertexAttribArray(0);
            assert(vbo != 0);
            assert(vao != 0);
            initialized = true;
        }
    };
    const std::vector<Vec2> Fullscreen::quad = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
    GLuint Fullscreen::vbo = 0;
    GLuint Fullscreen::vao = 0;
    GLuint Fullscreen::idx = 0;
    bool Fullscreen::initialized = false;
    
    std::size_t ceil(std::size_t x, std::size_t y) {return x/y + (x % y != 0);}
    template<typename Interpolator>
    class AcceleratedInterpolator
    {
    public:
        USING_INTERPOLATOR_TYPES;

        AcceleratedInterpolator()
        {
            S = SVector{}.size();   // source dimensionality
            R = Para{}.size();            // interpolator parameter dimensionality
            P = PVector{}.size();   // destination dimensionality
            rows = ceil(S + R + P, 4);
        }

        template<typename DemoList, typename ParaList>
        void init(const DemoList& demo, const ParaList& para)
        {
            Fullscreen::init();
            std::string name = 
                   std::string("Inverse Distance Fragment Interpolator (") 
                 + std::to_string(S)
                 + std::string(", ")
                 + std::to_string(R)
                 + std::string(", ")
                 + std::to_string(P)
                 + std::string(")");

            std::string vertex_source = load_file("demo/shaders/position_passthrough.vert");
            const char * vsrc = vertex_source.c_str();

            std::string main_source = load_file("demo/shaders/inverse_distance.frag");
            std::string preamble = std::string("#version 300 es\n")
                                 + std::string("#ifdef GL_ES\n")
                                 + std::string("precision highp float;\n")
                                 + std::string("#endif\n")
                                 + std::string("#define S ") 
                                 + std::to_string(S) + std::string("u\n") 
                                 + std::string("#define P ") 
                                 + std::to_string(P) + std::string("u\n") 
                                 + std::string("#define R ") 
                                 + std::to_string(R) + std::string("u\n")
                                 ;

            constexpr std::size_t sources = 2;
            const char * source[sources];
            source[0] = preamble.c_str();
            source[1] = main_source.c_str();

            GLuint vertex_shader   = create_shader(name.c_str(), GL_VERTEX_SHADER, &vsrc, 1);
            GLuint fragment_shader = create_shader(name.c_str(), GL_FRAGMENT_SHADER, source, sources);
            program = create_program(name.c_str(), vertex_shader, fragment_shader);
            resize(demo, para, false);
            texname = create_gl_texture(texture);
            reload(demo, para);
            assert(texname != 0);
        }

        // this is to be called when demonstrations are added or removed
        template<typename DemoList, typename ParaList>
        void resize(const DemoList& demo, const ParaList& para, bool need_to_reload = true)
        {
            N = demo.size();
            texture.resize(N, rows);
            if (need_to_reload) reload(demo, para);
        }

        // this is to be called when demonstrations are changed
        template<typename DemoList, typename ParaList>
        void reload(const DemoList& demo, const ParaList& para)
        {
            std::size_t n, i, idx, row, subrow;
            for (n = 0; n < N; ++n)
            {
                const auto& d = demo[n];
                const auto& r = para[n];
                idx = 0;
                for (i = 0; i < S; ++i, ++idx)
                {
                    row = idx / 4; // floor(i / 4) implied
                    subrow = idx % 4;
                    texture(n, row)(subrow) = d.s[i];
                }
                for (i = 0; i < R; ++i, ++idx)
                {
                    row = idx / 4;
                    subrow = idx % 4;
                    texture(n, row)(subrow) =   r[i];
                }
                for (i = 0; i < P; ++i, ++idx)
                {
                    row = idx / 4;
                    subrow = idx % 4;
                    texture(n, row)(subrow) = d.p[i];
                }
            }
            write_gl_texture(texture, texname);
        }

        void run() const
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texname);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glUseProgram(program);
            glUniform1i(glGetUniformLocation(program, "tex_sampler"), 0);
            glBindVertexArray(Fullscreen::vao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, Fullscreen::quad.size());
        }

    private:
        GLuint program = 0;
        GLuint texname = 0;
        Texture texture;
        std::size_t N = 0; // number of demonstrations ( == number of columns)
        std::size_t S, R, P, rows;
    };
    
}
#endif
