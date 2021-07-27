#define POWER 0
#define D_MIN 1
#define R_MIN 2
#define RADIUS 3

float sphere_of_influence = -1.0;

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

void setup(in vec2 q) {}

float calculate_weight(in vec2 q, in int n)
{
    load_demonstration(n);
    vec2 s = vec2(d.s[0], d.s[1]);
    float dist = distance(q, s);
    if (   sphere_of_influence < 0.0
        ||  dist < sphere_of_influence)
    {
        float powr = pow(dist - r[R_MIN], r[POWER] * r[POWER]);
        return r[RADIUS] / max(powr, 0.0000001);
    }
    else return 0.0;
}

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
