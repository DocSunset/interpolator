For maximum portability and generality, the straightforward CPU-based
implementation of the interpolators is ideal. However, for certain applications
higher performance and throughput is required. This is especially true of the
interactive demo application provided in this repository, where the whole
interpolated output topology of an algorithm is systematically queried in real
time and ideally needs to render at least as quickly as the display framerate.

This document gives an overview of the GPU based implementation of the 
algorithms used for the display in the interactive demos. 

```cpp
// @#'include/shader_interpolators.h'
#ifndef SHADER_INTERPOLATORS_H
#define SHADER_INTERPOLATORS_H
#include <string>
#include <Eigen/Core>
#include <GLES3/gl3.h>
#include "gl_boilerplate.h"
#include "gl_primitives.h"

namespace ShaderInterpolators
{
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
struct ShaderInterpolatorState
{
    bool focus = false;
    bool enable_contours = 0;
    float contours = 10;
    int focus_idx = -1;
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
        @{prepare shader program}
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
        @{copy demonstrations to texture buffer}
        write_gl_texture(texture, texname);
    }

    void run() const
    {
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
        glUniform1i(glGetUniformLocation(program, "focus_idx"), state.focus_idx);
        glUniform1f(glGetUniformLocation(program, "w"), window.w);
        glUniform1f(glGetUniformLocation(program, "h"), window.h);
        glUniform1i(glGetUniformLocation(program, "focus"), state.focus);

        Fullscreen::draw();
    }

    ShaderInterpolatorState state;
    WindowSize window;
private:
    GLuint program = 0;
    GLuint texname = 0;
    Texture texture;
    std::size_t N = 0; // number of demonstrations ( == number of columns)
    std::size_t S, R, P, rows;
};
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

```cpp
// @='copy demonstrations to texture buffer'
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
// @/
```

Similarly, in the shader the current parameters can be loaded as follows based
on the source, parameter, and destination vector sizes' availability in from
the symbols `S`, `R`, and `P` respectively, as well as the number of
demonstrations and rows in `N` and `rows` respectively.

```cpp
// @+'shader functions'
void load_demonstration(int n)
{
    int i, idx, row, subrow;
    idx = 0;
    for (i = 0; i < S; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
        d.s[i] = texelFetch(tex_sampler, ivec2(row, n), 0)[subrow];
    }
#ifdef R
    for (i = 0; i < R; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
          r[i] = texelFetch(tex_sampler, ivec2(row, n), 0)[subrow];
    }
#endif
    for (i = 0; i < P; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
        d.p[i] = texelFetch(tex_sampler, ivec2(row, n), 0)[subrow];
    }
}
// @/
```

The above implies the presence of a few common variables, which are provided
here. 

```cpp
// @='common shader interpolator variables'
#define pi 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899863

struct Demo
{
    float s[S];
    float p[P];
} d;

#ifdef R
float r[R];
#endif

uniform int N;
uniform int rows;

uniform sampler2D tex_sampler;
uniform bool focus;
uniform float contours;
uniform int focus_idx;
uniform float w;
uniform float h;
in vec2 position;
out vec4 colour;
// @/

```

So for most interpolators, all that remains is to calculate the weights and
accumulate a weighted sum for all of the demonstrations. The shader thus
takes this form:

```cpp
// @='shader main'
void main()
{
    vec2 q = vec2(position.x * w/2.0, position.y * h/2.0);
    setup(q);

    vec3 weighted_sum = vec3(0.0, 0.0, 0.0);
    float weight = 0.0;
    float sum_of_weights = 0.0;

    if (N < 1)
    {
        colour = vec4(weighted_sum, 1.0);
        return;
    }

    int loner = -1;
    if (focus && focus_idx >= 0) loner = focus_idx;
    for (int n = 0; n < N; ++n)
    {
        load_demonstration(n);
        weight = calculate_weight(q, n);
        load_demonstration(n);
        sum_of_weights += weight;
        if (contours <= 0.0)
        {
            if (loner < 0 || n == loner)
                weighted_sum += vec3(d.p[0], d.p[1], d.p[2]) * weight;
        }
    }
    if (contours > 0.0)
    {
        for (int n = 0; n < N; ++n)
        {
            if (loner >= 0) n = loner;
            load_demonstration(n);
            weight = calculate_weight(q, n) / sum_of_weights;
            load_demonstration(n);
            if (weight >= 1.0)
            {
                weighted_sum = vec3(1.0, 1.0, 1.0);
                break;
            }
            else
            {
                float brightness = pow(mod(weight * contours, 1.0), 8.0);
                weighted_sum += vec3(d.p[0], d.p[1], d.p[2]) * brightness * weight;
            }
            if (loner >= 0) break;
        }
    }
    if (contours <= 0.0) colour = vec4(weighted_sum / sum_of_weights, 1.0);
    else colour = vec4(weighted_sum, 1.0);
}
// @/
```

Each unique interpolation algorithm simply needs to define its own functions:
`void setup()` that does any pre-weight calculating set up,
and `float calculate_weights()` that examines the current contents of the `d`
structure and calculates the weight for that demonstration.

Obviously the shader currently assumes the output is a 3D normalized floating
point vector that can be directly rendered as a colour. A fully general shader-
based implementation remains as future work; the most useful incarnation of
this would be for each instance of the shader to calculate a single weight or
weighted parameter vector that could then be collected and added together on
the CPU. If there were a large number of demonstrations this would probably
provide a performance improvement over using the CPU. It would probably be best
to do this using a vertex shader and transform feedback techniques to read the
output. 

Another useful approach using fragment shaders would be for the shader to
perform some means of visualizing the P-dimensional output of the interpolator
that is agnostic about the actual number of dimensions.  Development of such
algorithms is the subject of high-dimensional data visualization, and is
currently beyond the scope of this project. Maybe one day though.

With those resources in place, it shouldn't be difficult to implement various
interpolators as fragment shaders. The shaders themselves are listed alongside
the CPU-based implementation of the interpolators.

@[lilit/opengl_primitives.md]

@[lilit/opengl_rendering_boilerplate.md]