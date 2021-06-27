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

float circle_circle_intersection_area(
        in float R,
        in float r,
        in float d)
{
    float d2 = d * d;
    float r2 = r * r;
    float R2 = R * R;
    float two_d = 2.0 * d;
    float arg1 = (d2 + r2 - R2)/(two_d * r);
    float arg2 = (d2 + R2 - r2)/(two_d * R);
    float arg3 = (-d+r+ R) * (d+r-R) * (d-r+R) * (d+r+R);
    float a, b, c;
    if (arg1 > 1.0) a = 0.0;
    else a = r2 * acos(arg1);
    if (arg2 > 1.0) b = 0.0;
    else b = R2 * acos(arg2);
    if (arg3 < 0.0) c = 0.0;
    else c = sqrt(arg3) / 2.0;
    return a + b - c;
}

#define pi 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899863

float circle_area(in float r)
{
    return pi * r * r;
}

float intersecting_spheres_weight(in float R, in float r, in float d)
{
    return circle_circle_intersection_area(R, r, d) / circle_area(r);
}

float r_q;

void setup(in vec2 q)
{
    float fMaxFloat = intBitsToFloat(2139095039);
    r_q = fMaxFloat;
    for (int n = 0; n < N; ++n)
    {
        load_demonstration(n);
        vec2 s = vec2(d.s[0], d.s[1]);
        vec3 p = vec3(d.p[0], d.p[1], d.p[2]);
        float d_n = distance(q, s);
        r_q = min(d_n, r_q);
    }
}

float calculate_weight(in vec2 q, in int n)
{
    float fMaxFloat = intBitsToFloat(2139095039);
    load_demonstration(n);
    vec2 s = vec2(d.s[0], d.s[1]);
    vec3 p = vec3(d.p[0], d.p[1], d.p[2]);
    float d_n = distance(q, s);
    float r_n = fMaxFloat;
    for (int m = 0; m < N; ++m)
    {
        if (n == m) continue;
        load_demonstration(m);
        vec2 s_m = vec2(d.s[0], d.s[1]);
        float r = distance(s, s_m);
        r_n = min(r, r_n);
    }
    load_demonstration(n);
    if ((r_q + r_n) < d_n) return 0.0;
    return intersecting_spheres_weight(r_q, min(r_n, d_n), d_n);
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
