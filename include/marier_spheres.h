#include<unordered_map>
#include<limits>
#include<string>
#include<cstddef>
#include<cmath>

template<typename Scalar, typename S, typename P>
struct Demonstration
{
    using SVector = S;
    using PVector = P;
    std::string name;
    SVector s;
    PVector p;
};

template<typename Demo>
struct DemonstrationHash
{
    std::size_t operator()(const Demo& d) const noexcept
    {
        return std::hash<std::string>{}(d.name);
    }
};

template<typename Scalar, typename S, typename P>
class PresetInterpolator
{
public:
    using SVector = S;
    using PVector = P;
    using Demo = Demonstration<Scalar, SVector, PVector>;

    virtual void add_demo(const Demo& d) = 0;
    virtual void update_demo(const Demo& d) = 0;
    virtual void remove_demo(const Demo& d) = 0;
    virtual PVector query(const SVector& s) = 0;

    template<typename DemoList>
    void add_demo(const DemoList& demos) 
    {
        for (const Demo& d : demos) add_demo(d);
    }

    template<typename DemoList>
    void update_demo(const DemoList& demos)
    {
        for (const Demo& d : demos) update_demo(d);
    }

    template<typename DemoList>
    void remove_demo(const DemoList& demos)
    {
        for (const Demo& d : demos) remove_demo(d);
    }
};

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


template<typename Scalar, typename S, typename P>
class MarierSpheresInterpolator : public PresetInterpolator<Scalar, S, P>
{
public:
    struct Circle { Scalar r; Scalar d; };
    using Parent = PresetInterpolator<Scalar, S, P>;
    using Demo = typename Parent::Demo;
    using SVector = typename Parent::SVector;
    using PVector = typename Parent::PVector;
    const std::string cursor_name = "/cursor";

    MarierSpheresInterpolator() {add_demo({cursor_name,{},{}});}
    const Circle& get_cursor() 
    {
        auto it = set.find(cursor_name);
        return it->second.second;
    }

    void set_cursor(const SVector& q) 
    {
        Demo cursor = {cursor_name,q,{}};
        update_demo(cursor);
    }

    void add_demo(const Demo& d) override
    {
        set.insert({d.name, std::pair<Demo, Circle>({d, {}})});
    }

    void update_demo(const Demo& d) override
    {
        auto prior_it = set.find(d.name);
        if (prior_it != set.end()) 
        {
            Demo& p = prior_it->second.first;
            p = d;
        }
    }

    void remove_demo(const Demo& d) override
    {
        if (d.name == cursor_name) return; // silently ignore if the caller tries to remove the cursor
        set.erase(d.name);
    }

    PVector query(const SVector& q) override
    {
        if (set.size() < 2) return PVector();
        set_cursor(q);

        for (auto& pair1 : set)
        {
            auto& data = pair1.second;
            const auto& d1 = data.first;
            auto& circle = data.second;
            if (d1.name != cursor_name)
            {
                Scalar distance = norm(d1.s - q);
                constexpr Scalar an_arbitrary_slop_factor = 
                        std::numeric_limits<Scalar>::epsilon() * 5;
                if (distance <= an_arbitrary_slop_factor) return d1.p;
        
                circle.d = distance;
            }
        
            Scalar radius = std::numeric_limits<Scalar>::max();
            for (const auto& pair2 : set)
            {
                const auto& d2 = pair2.second.first;
                if (d1.name == d2.name) continue;
                Scalar distance = norm(d1.s - d2.s);
                if (distance < radius) radius = distance;
            }
            circle.r = radius;
        }

        Scalar sum_of_weights = 0;
        PVector weighted_sum{};
        Scalar q_radius = get_cursor().r;
        for (const auto& pair : set)
        {
            const auto& data = pair.second;
            const auto& demo = data.first;
            if (demo.name == cursor_name) continue;
            const auto& circle = data.second;
            if ((q_radius + circle.r) < circle.d) continue; // the circles are non-intersecting
        
            Scalar w = weight(q_radius, circle.r, circle.d);
            sum_of_weights += w;
            weighted_sum += w * demo.p;
        }
        weighted_sum = (1 / sum_of_weights) * weighted_sum;
        return weighted_sum;
    }

private:
    std::unordered_map<std::string, std::pair<Demo, Circle>> set;
};