struct Demo
{
    float s[S];
    float p[P];
} d;

float r[R];

uint N;
float fN;
uint rows;
float frows;

uniform sampler2D tex_sampler;
in vec2 position;
out vec4 colour;
void load_demonstration(uint n)
{
    float x = (float(n) + 0.5) / fN;
    float y;
    uint i, idx, row, subrow;
    idx = 0u;
    for (i = 0u; i < S; ++i, ++idx)
    {
        row = idx / 4u;
        subrow = idx % 4u;
        y = (float(row) + 0.5)/frows;
        d.s[i] = texture(tex_sampler, vec2(x, y))[subrow];
    }
    for (i = 0u; i < R; ++i, ++idx)
    {
        row = idx / 4u;
        subrow = idx % 4u;
        y = (float(row) + 0.5)/frows;
          r[i] = texture(tex_sampler, vec2(x, y))[subrow];
    }
    for (i = 0u; i < P; ++i, ++idx)
    {
        row = idx / 4u;
        subrow = idx % 4u;
        y = (float(row) + 0.5)/frows;
        d.p[i] = texture(tex_sampler, vec2(x, y))[subrow];
    }
}
void set_dimensions()
{
    ivec2 sz = textureSize(tex_sampler, 0);
    N = uint(sz[0]);
    fN = float(N);
    rows = uint(sz[1]);
    frows = float(rows);
}
#define POWER 0
#define D_MIN 1
#define R_MIN 2
#define RDIUS 3

void main() // line 65
{
    set_dimensions();

    vec3 weighted_sum = vec3(0.0, 0.0, 0.0);
    float sum_of_weights = 0.0;

    if (N < 1u)
    {
        colour = vec4(weighted_sum, 1.0);
        return;
    }

    vec2 q = position;
    vec2 s = vec2(d.s[0], d.s[1]);
    for (uint n = 0u; n < N; ++n)
    {
        load_demonstration(n);
        float dist = distance(q, s);
        float base = max(dist - r[R_MIN], r[D_MIN]);
        float weight = r[RDIUS] / pow(base, r[POWER]);
        sum_of_weights += weight;
        weighted_sum += vec3(d.p[0], d.p[1], d.p[2]) * weight;
    }
    //colour = vec4(weighted_sum / sum_of_weights, 1.0);
    colour = vec4( vec3(4.0 / 4.0), 1.0);
}
