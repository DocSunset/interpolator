#pragma once

namespace Shader::Interpolator
{
constexpr const char * vertex = R"GLSL(
#version 300 es

in vec2 pos;
out vec2 position;

void main()
{
    gl_Position = vec4(pos, 0.0, 1.0);
    position = vec2(pos[0], pos[1]);
}
)GLSL";

constexpr const char * preamble = R"GLSL(
#version 300 es
#ifdef GL_ES
precision highp float;
#endif

#define pi 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899863
)GLSL";

constexpr const char * define_s = "#define S ";
constexpr const char * define_p = "#define P ";
constexpr const char * define_r = "#define R ";
constexpr const char * define_u = "#define U ";

constexpr const char * variables = R"GLSL(
struct Demo
{
    float s[S]; // source vector
    float p[P]; // destination vector
} d;

#ifdef R
float r[R]; // interpolator varying parameters
#endif

#ifdef U
uniform u[U]; // interpolator uniform parameters
#endif

uniform int N; // number of demonstrations
uniform int rows;

uniform sampler2D tex_sampler;
uniform bool focus;
uniform float contours;
uniform int focus_idx;
uniform float w;
uniform float h;

in vec2 position;
out vec4 colour;
)GLSL";

constexpr const char * loader = R"GLSL(
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
)GLSL";

constexpr const char * main = R"GLSL(
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
)GLSL";

constexpr int num_sources = 11;

char * assemble_shader(const char * interp_func, int s, int p, int r = -1, int u = -1);

}
