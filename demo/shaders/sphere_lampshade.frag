#define POWER 0
#define BRIGHTNESS 1
#define RADIUS 2

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

//void main()
//{
//    //vec2 q = vec2( 0.0,0.0 );
//    vec2 q = vec2(position.x * w/2.0, position.y * h/2.0);
//    int m = 0;
//    load_demonstration(m);
float calculate_weight(in vec2 q, in int m)
{
    vec2 s = vec2(d.s[0], d.s[1]);
    vec2 qms = q - s;
    float q2s2 = dot(qms, qms);
    float q2s  = sqrt(q2s2);
    float base = pow(q2s, r[POWER] * r[POWER]);
    float loss = r[BRIGHTNESS];
    for (int n = 0; n < N; ++n)
    {
        if (m == n) continue;
        load_demonstration(n);
        vec2 s_n = vec2(d.s[0], d.s[1]);
        vec2 nms = s_n - s;
        float u = dot(nms, qms) / q2s2;
        vec2 k = s + u * qms;

        float k2n2 = dot((s_n - k), (s_n - k));
        float r2 = r[RADIUS] * r[RADIUS];
        if (r2 < k2n2) continue;

        float q2n2 = dot( (s_n - q), (s_n - q) );
        float s2n2 = dot( (s_n - s), (s_n - s) );
        bool q_inside = q2n2 < r2;
        bool s_inside = s2n2 < r2;

        float secant;
        if (q_inside && s_inside) secant = q2s;
        else if (q_inside)
        {
            //if (u < 0.0) panic because that should never happen
            if (u < 1.0) secant = sqrt(r2 - k2n2) + sqrt(q2n2 - k2n2);
            else secant = sqrt(r2 - k2n2) - sqrt(q2n2 - k2n2);
        }
        else if (s_inside)
        {
            if (u < 0.0) secant = sqrt(r2 - k2n2) - sqrt(s2n2 - k2n2);
            else if (u < 1.0) secant = sqrt(r2 - k2n2) + sqrt(s2n2 - k2n2);
            //else panic because that should never happen
        }
        else if (0.0 < u && u < 1.0)
        {
            secant = 2.0 * sqrt(r2 - k2n2);
        }
        else continue;
        float l = 1.0 - (secant / (2.0 * r[RADIUS]));
        l = -0.5 * cos(pi * l) + 0.5;

        loss = loss * l;
    }
    return loss / base;
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
