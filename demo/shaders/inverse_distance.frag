#define POWER 0
#define D_MIN 1
#define R_MIN 2
#define RDIUS 3

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
uniform int grabbed_idx;
uniform int selectd_idx;
uniform int hovered_idx;
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
    vec3 p = vec3(d.p[0], d.p[1], d.p[2]);
    float dist = distance(q, s);
    float base = max(dist - r[R_MIN], r[D_MIN]);
    return r[RDIUS] / pow(base, r[POWER]);
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
    if (focus)
    {
        if      (grabbed_idx >= 0) loner = grabbed_idx;
        else if (selectd_idx >= 0) loner = selectd_idx;
        else if (hovered_idx >= 0) loner = hovered_idx;
    }

    for (int n = 0; n < N; ++n)
    {
        load_demonstration(n);
        weight = calculate_weight(q, n);
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
            weight = calculate_weight(q, n) / sum_of_weights;
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

    bool invert = false;
    for (int n = 0; n < N; ++n)
    {
        load_demonstration(n);
        vec2 s = vec2(d.s[0], d.s[1]);
        float dist = distance(s, q);
        if (dist <= 5.0) return;
        float brightness = 0.299 * colour.x + 0.587 * colour.y + 0.114 * colour.z;
        bool bright = 0.5 < brightness;
        if (8.0 < dist && dist < 10.0)
        {
            colour = vec4(vec3(0.0), 1.0);
        }
        else if (dist <= 8.0)
        {
            if (n == selectd_idx) colour = vec4(1.0, 0.0, 0.0, 1.0);
            else if (n == hovered_idx) colour = vec4(1.0, 1.0, 1.0, 1.0);
            else colour = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
}
