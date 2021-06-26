#define POWER 1
#define D_MIN 2
#define R_MIN 3
#define RDIUS 4

void main()
{
    set_dimensions();

    if (N < 1)
    {
        color = weighted_sum;
        return;
    }

    vec3 weighted_sum = {0, 0, 0};
    float sum_of_weights = 0;
    vec2 q = position;
    vec2 s = {d.s[0], d.s[1]};
    for (uint n = 0; n < N; ++n)
    {
        load_demonstration(n);
        float dist = distance(q, s);
        float base = max(dist - r[R_MIN], r[D_MIN]);
        float weight = r[RDIUS] / pow(base, r[POWER]);
        sum_of_weights += weight;
        weighted_sum += vec3(p[0], p[1], p[2]) * weight;
    }
    color = vec4(weighted_sum / sum_of_weights, 1);
}
