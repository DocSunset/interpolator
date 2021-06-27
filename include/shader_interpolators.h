#ifndef SHADER_INTERPOLATORS_H
#define SHADER_INTERPOLATORS_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <Eigen/Core>
#include <GLES3/gl3.h>
#include "gl_boilerplate.h"
#include "gl_primitives.h"

namespace ShaderInterpolators
{
    struct ShaderInterpolatorState
    {
        bool focus = false;
        bool enable_contours = 0;
        float contours = 10;
        int grabbed_idx = -1;
        int selectd_idx = -1;
        int hovered_idx = -1;
        float w = 720;
        float h = 720;
    };

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
                   std::string(Interpolator::name)
                 + std::string(" (") 
                 + std::to_string(S)
                 + std::string(", ")
                 + std::to_string(R)
                 + std::string(", ")
                 + std::to_string(P)
                 + std::string(")");

            std::string vertex_source = load_file("demo/shaders/position_passthrough.vert");
            const char * vsrc = vertex_source.c_str();

            std::string main_source = load_file(Interpolator::frag);
            std::string preamble = std::string("#version 300 es\n")
                                 + std::string("#ifdef GL_ES\n")
                                 + std::string("precision highp float;\n")
                                 + std::string("#endif\n")
                                 + std::string("#define S ") 
                                 + std::to_string(S) + std::string("\n") 
                                 + std::string("#define P ") 
                                 + std::to_string(P) + std::string("\n")
                                 ;
            if (R > 0) preamble += std::string("#define R ") 
                                 + std::to_string(R) + std::string("\n") 
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
            glViewport(0,0,state.w,state.h);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texname);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glUseProgram(program);

            glUniform1i(glGetUniformLocation(program, "tex_sampler"), 0);
            glUniform1i(glGetUniformLocation(program, "N"), N);
            glUniform1i(glGetUniformLocation(program, "rows"), rows);

            if (state.enable_contours)
                glUniform1f(glGetUniformLocation(program, "contours"), state.contours);
            else
                glUniform1f(glGetUniformLocation(program, "contours"), 0);
            glUniform1i(glGetUniformLocation(program, "grabbed_idx"), state.grabbed_idx);
            glUniform1i(glGetUniformLocation(program, "selectd_idx"), state.selectd_idx);
            glUniform1i(glGetUniformLocation(program, "hovered_idx"), state.hovered_idx);
            glUniform1f(glGetUniformLocation(program, "w"), state.w);
            glUniform1f(glGetUniformLocation(program, "h"), state.h);
            glUniform1i(glGetUniformLocation(program, "focus"), state.focus);

            Fullscreen::draw();
        }

        ShaderInterpolatorState state;
    private:
        GLuint program = 0;
        GLuint texname = 0;
        Texture texture;
        std::size_t N = 0; // number of demonstrations ( == number of columns)
        std::size_t S, R, P, rows;
    };
    
}
#endif
