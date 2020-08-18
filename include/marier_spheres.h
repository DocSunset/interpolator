#include<limits>
#include<cmath>

template<typename Scalar>
Scalar circle_circle_intersection_area(
        const Scalar& R, 
        const Scalar& r, 
        const Scalar& d)
{
    Scalar d2 = d * d;
    Scalar r2 = r * r;
    Scalar R2 = R * R;
    Scalar two_d = 2 * d;
    Scalar a = r2 * std::acos((d2 + r2 - R2)/(two_d * r));
    Scalar b = R2 * std::acos((d2 + R2 - r2)/(two_d * R));
    Scalar c = std::sqrt((-d+r+ R) * (d+r-R) * (d-r+R) * (d+r+R))/2.0;
    return a + b - c;
}

template<typename Scalar>
Scalar circle_area(const Scalar& r)
{
    constexpr Scalar pi = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899863;
    return pi * r * r;
}

template<typename Scalar>
Scalar weight(const Scalar& R, const Scalar& r, const Scalar& d)
{
    return circle_circle_intersection_area(R, r, d) / circle_area(r);
}

template<typename Scalar, typename ID, typename SVector, typename PVector>
class MarierSpheresInterpolator
{
public:
    struct Demo { ID id; SVector s; PVector p; };

    template<typename DemoList>
    PVector query(const SVector& q, const DemoList& demos)
    {
        Scalar q_radius, radius, distance;
        Scalar sum_of_weights = 0;
        PVector weighted_sum = {};
        if (demos.size() < 1) return weighted_sum;

        q_radius = std::numeric_limits<Scalar>::max();
        for (const auto& demo : demos)
        {
            Scalar d = norm(demo.s - q);
            if (d < q_radius) q_radius = d;
        }

        for (const auto& demo : demos)
        {
            distance = norm(demo.s - q);
            constexpr Scalar an_arbitrary_slop_factor = 
                std::numeric_limits<Scalar>::epsilon() * 5;
            if (distance <= an_arbitrary_slop_factor) return demo.p;
        
            radius = std::numeric_limits<Scalar>::max();
            for (const auto& demo2 : demos)
            {
                if (demo.id == demo2.id) continue;
                Scalar d = norm(demo.s - demo2.s);
                if (d < radius) radius = d;
            }
            if (distance < radius) radius = distance;
        
            if ((q_radius + radius) < distance) continue; // the circles are non-intersecting
            Scalar w = weight(q_radius, radius, distance);
            sum_of_weights += w;
            weighted_sum += w * demo.p;
        }
        weighted_sum = (1 / sum_of_weights) * weighted_sum;
        return weighted_sum;
    }
};