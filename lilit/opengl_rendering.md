For maximum portability and generality, the straightforward CPU-based
implementation of the interpolators is ideal. However, for certain applications
higher performance and throughput is required. This is especially true of the
interactive demo application provided in this repository, where the whole
interpolated output of an algorithm is systematically queried in real time and
ideally needs to render at least as quickly as the display framerate.

This document gives an overview of the GPU based implementation of the 
algorithms used for the display in the interactive demos. 

```cpp
// @='include/shader_interpolators.h'
#ifndef SHADER_INTERPOLATORS_H
#define SHADER_INTERPOLATORS_H
#include <iostream>
#include <string>
#include <Eigen/Core>
#include <GLES3/gl3.h>

namespace ShaderInterpolators
{
    using Texture = Eigen::Matrix<RGBAVec, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    @{openGL boilerplate}

    @{fullscreen quad} // this is defined in interactive_demo/opengl.md 
    
    @{shader interpolators base class}
    
    @{shader interpolators}
}
#endif
// @/
```

The `AcceleratedInterpolator` class provides the basic plumbing needed to take
a shader-based interpolator and set up an OpenGL program, texture, and vertex
object in order to run the interpolator and display the output. Rather than
inherit the class, the actual shader implementations are passed as a template
parameter; `AcceleratedInterpolator` defines the expected parameters that
the implementations should have in order to be turned into OpenGL programs.

```cpp
// @='shader interpolators base class'
std::size_t ceil(std::size_t x, std::size_t y) {return x/y + (x % y != 0);}
template<typename Interpolator>
class AcceleratedInterpolator
{
public:
    using Demo = ShaderInterpolator::Demo;
    using Para = ShaderInterpolator::Para;
    using ShaderNames = ShaderInterpolator::ShaderNames;

    AcceleratedInterpolator()
    : S{Demo::SVector{}.size()}   // source dimensionality
    , R{Para{}.size()}            // interpolator parameter dimensionality
    , P{Demo::PVector{}.size()}   // destination dimensionality
    , rows{ceil(S + T + P, 4)}
    {
    }

    template<typename DemoList, typename ParaList>
    void init(const DemoList& demo, const ParaList& para)
    {
        @{prepare shader program}
        texname = create_gl_texture(texture);
        assert(texname != 0);
        resize(demo, para);
    }

    // this is to be called when demonstrations are added or removed
    template<typename DemoList, typename ParaList>
    void resize(const DemoList& demo, const ParaList& para)
    {
        N = demo.size();
        texture.resize(N, texture.rows());
        reload(demo, para);
    }

    // this is to be called when demonstrations are changed
    template<typename DemoList, typename ParaList>
    void reload(const DemoList& demo, const ParaList& para)
    {
        @{copy demonstrations to texture buffer}
        write_gl_texture(texture, texname);
    }

    void run() const
    {
        glUseProgram(gl.prog);
        glBindVertexArray(Fullscreen::vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, Fullscreen::quad.size());
    }

private:
    GLuint prog = 0;
    GLuint texname = 0;
    Texture texture;
    std::size_t N = 0; // number of demonstrations ( == number of columns)
    const std::size_t S, R, P, rows;
};

constexpr const char * const FragmentShaderInterpolatorVert =
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

template <typename anything>
const char * AcceleratedInterpolator<anything>::vert = FragmentShaderInterpolatorVert;
// @/
```

The shader program is assembled from the generic `position_passthrough.vert`
vertex shader and the fragment shader source named in the main interpolator
class. The dimensions of the source, parameter, and destination vectors are
dynamically prepended to the fragment shader source, along with the glsl
version and float precision preprocessor directives, by passing an array of two
strings to the program creation function instead of the typical single string.

```cpp
// @='prepare shader program'
std::string name = 
       std::string("Inverse Distance Fragment Interpolator (") 
     + std::string(S)
     + std::string(", ")
     + std::string(R)
     + std::string(", ")
     + std::string(P)
     + std::string(")");

std::string vertex_source = load_file("demo/shaders/position_passthrough.vert");
const char * vsrc = vertex_source.c_str();

std::string main_source = load_file("demo/shaders/inverse_distance.frag");
std::string preamble = { "#version 300 es\n"
                       , "#ifdef GL_ES\n"
                       , "precision highp float;\n"
                       , "#endif\n"
                       };

preamble += std::string("#define S ") + std::string(S) + std::string("\n")
          , std::string("#define P ") + std::string(P) + std::string("\n")
          , std::string("#define R ") + std::string(R) + std::string("\n")
          ;

constexpr std::size_t source = 2;
const char * source[sources];
source[0] = preamble.c_str();
source[1] = main_source.c_str();

GLuint vertex_shader   = create_shader(name.c_str(), GL_VERTEX_SHADER, &vsrc, 1);
GLuint fragment_shader = create_shader(name.c_str(), GL_FRAGMENT_SHADER, source, sources);
prog = create_program(name, vertex_shader, fragment_shader);
// @/
```

We are using OpenGLES3, since it has good compatibility with the web and
bindings available for most platforms. Unfortunately, it doesn't have GPGPU
compute functionality, so it's necessary for us to hack the graphics shaders a
little bit in order to get the interpolators' data into the program.  The
chosen route is to use a texture's RGBA data fields to hold the arbitrary
demonstration and parameter data. We give up the ability to to query the
metadata produced by the interpolation algorithms, since we are using the
output texture to draw to the screen, but the metadata isn't really meaningful
in a global way anyways; it's more relevant to each individual query.

First let's consider how to coerce our demonstration and parameter data into
a texture that can be uploaded to the GPU. Reading the data will be done by the
shader, and will simply be the inverse operation. For simplicity, let's start
with the scenario where the texture is already the right size and we just
need to cram in the data.

The data is laid out such that each demonstration takes up one column. The
number of pixels per column (i.e. the number of rows in the texture) is
calculated in the constructor so that there is guaranteed to be enough space
for each demonstration. If the sum of the dimensionality of source, destination
and parameter vectors is not exactly a multiple of four, there will be some
space left over, but this is not considered harmful. So the procedure for
loading the texture is really quite simple. Just scan over the source,
parameter, and destination vectors for a demonstration and load them
sequentially into the sub-pixels of the same column.

In the implementation below, the variable `i` is used to point into the
vectors.  It is reset to zero when switching from one vector to the next (i.e.
from source to parameter, and from parameter to destination). The variable
`idx` is not reset; it points into the subpixels of the column that is
currently being filled. It's a bit tricky, and not particularly nice to look
at, but it's really very simple.

Do note that the JzAzBz colour space is not used in the shaders at this time.

```cpp
// @='copy demonstrations to texture buffer'
for (std::size_t n = 0; n < demo.size(); ++n)
{
    const auto& d = demo;
    const auto& r = para;
    std::size_t n, i, idx, row, subrow;
    for (n = 0; n < N; ++n)
    {
        idx = 0;
        for (i = 0; i < S; ++i, ++idx)
        {
            row = idx / 4; // floor(i / 4) implied
            subrow = idx % 4;
            texture(row, n)(subrow) = d.s[i];
        }
        for (i = 0; i < R; ++i, ++idx)
        {
            row = idx / 4;
            subrow = idx % 4;
            texture(row, n)(subrow) =   r[i];
        }
        for (i = 0; i < P; ++i, ++idx)
        {
            row = idx / 4;
            subrow = idx % 4;
            texture(row, n)(subrow) = JzAzBz_to_RGB(d.p)[i];
        }
    }
}
// @/
```

Similarly, in the shader the current parameters can be loaded as follows based
on the source, parameter, and destination vector sizes' availability in from
the symbols `S`, `R`, and `P` respectively, as well as the number of
demonstrations and rows in `fN` and `frows` respectively. The latter two, as
floating point numbers, are needed to convert discrete texel coordinates to the
normalized 0-1 coordinates used by the texture lookup function.

```cpp
// @+'shader functions'
void load_demonstration(uint n)
{
    float x = ((float)n + 0.5) / fN;
    float y;
    uint i, idx, row, subrow;
    idx = 0;
    for (i = 0; i < S; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
        y = ((float)row + 0.5)/frows;
        d.s[i] = texture(tex_sampler, vec2(x, y))[subrow];
    }
    for (i = 0; i < R; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
        y = ((float)row + 0.5)/frows;
          r[i] = texture(tex_sampler, vec2(x, y))[subrow];
    }
    for (i = 0; i < P; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
        y = ((float)row + 0.5)/frows;
        d.p[i] = texture(tex_sampler, vec2((x, y)/rows, n/fN))[subrow];
    }
}
// @/
```

The above implies the presence of a few common variables, which are provided
here. 

```cpp
// @='common shader interpolator variables'
struct Demo
{
    float s[S];
    float p[P];
}
float r[R];

uint N;
float fN;
uint rows;
float frows;

uniform sampler2D tex_sampler;
in vec2 position;
out vec4 colour;
// @/

```

The dimensions of the demonstrations texture can be queried at runtime using
this routine:

```cpp
// @+'shader functions'
void set_dimensions()
{
    ivec2 sz = textureSize(tex_sampler, 0);
    N = sz[0];
    fN = N;
    rows = sz[1];
    frows = rows;
}
// @/
```

With those resources in place, it shouldn't be difficult to implement various
interpolators as fragment shaders. The shaders themselves are listed alongside
the CPU-based implementation of the interpolators.

@[lilit/opengl_rendering_boilerplate.md]
