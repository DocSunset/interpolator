#define POWER 0
#define D_MIN 1
#define R_MIN 2
#define RDIUS 3

struct Demo
{
    float s[S];
    float p[P];
} d;

float r[R];

int N;
int rows;

uniform sampler2D tex_sampler;
uniform int contour_lines;
uniform int grabbed_idx;
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
    for (i = 0; i < R; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
          r[i] = texelFetch(tex_sampler, ivec2(row, n), 0)[subrow];
    }
    for (i = 0; i < P; ++i, ++idx)
    {
        row = idx / 4;
        subrow = idx % 4;
        d.p[i] = texelFetch(tex_sampler, ivec2(row, n), 0)[subrow];
    }
}
void set_dimensions()
{
    ivec2 sz = textureSize(tex_sampler, 0);
    N = sz[0];
    rows = sz[1];
}

float calculate_weight()
{
    vec2 q = position;
    vec2 s = vec2(d.s[0], d.s[1]);
    vec3 p = vec3(d.p[0], d.p[1], d.p[2]);
    float dist = distance(q, s);
    float base = max(dist - r[R_MIN], r[D_MIN]);
    return r[RDIUS] / pow(base, r[POWER]);
}

void main() // line 65
{
    set_dimensions();

    vec3 weighted_sum = vec3(0.0, 0.0, 0.0);
    float weight = 0.0;
    float sum_of_weights = 0.0;

    if (N < 1)
    {
        colour = vec4(weighted_sum, 1.0);
        return;
    }

    for (int n = 0; n < N; ++n)
    {
        load_demonstration(n);
        weight = calculate_weight();
        sum_of_weights += weight;
        if (contour_lines <= 0)
        {
            weighted_sum += vec3(d.p[0], d.p[1], d.p[2]) * weight;
        }
    }
    if (contour_lines > 0)
    {
        for (int n = 0; n < N; ++n)
        {
            if (grabbed_idx >= 0) n = grabbed_idx;
            load_demonstration(n);
            weight = calculate_weight() / sum_of_weights;
            if (weight >= 1.0)
            {
                weighted_sum = vec3(1.0, 1.0, 1.0);
                break;
            }
            else
            {
                float brightness = pow(mod(weight * float(contour_lines), 1.0), 8.0);
                weighted_sum += vec3(d.p[0], d.p[1], d.p[2]) * brightness * weight;
            }
            if (grabbed_idx >= 0) break;
        }
    }
    if (contour_lines <= 0) colour = vec4(weighted_sum / sum_of_weights, 1.0);
    else colour = vec4(weighted_sum, 1.0);
}
