#include<unordered_map>
#include<numeric_limits>
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

    void add_demo(const Demo& d) = void;
    void update_demo(const Demo& d) = void;
    void remove_demo(const Demo& d) = void;

    template<typename DemoList>
    void add_demo(const DemoList& demos) = void;

    template<typename DemoList>
    void update_demo(const DemoList& demos) = void;

    template<typename DemoList>
    void remove_demo(const DemoList& demos) = void;

    PVector query(const SVector& s) = void;
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
    Scalar two_dr = 2 * d * r;
    Scalar a = r2 * std::acos((d2 + r2 - R2)/two_dr);
    Scalar b = R2 * std::acos((d2 + R2 - r2)/two_dr);
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
    static constexpr std::string cursor_name = "/cursor";

    MarierSpheresInterpolator() {add_demo({cursor_name,{},{}});}
    const Demo& get_cursor() {return *(set.find({cursor_name,{},{}}));}
    void set_cursor(const SVector& q) {return update_demo({cursor_name,q,{}});}

    void add_demo(const Demo& d)
    {
        set.insert(decltype(set)::value_type(d, {}));
    }

    void update_demo(const Demo& d)
    {
        auto prior_it = set.find(d);
        if (prior_it != set.end()) prior_it->first = d;
    }

    void remove_demo(const Demo& d)
    {
        if (d.name == cursor_name) return; // silently ignore if the caller tries to remove the cursor
        set.erase(d);
    }

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

    PVector query(const SVector& q)
    {
        if (set.size() < 2) return PVector();
        set_cursor(q);

        for (auto& pair1 : set)
        {
            const auto& d1 = pair1.first;
            auto& circle = pair1.second;
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
                const auto& d2 = pair2.first;
                if (d1.name == d2.name) continue;
                Scalar distance = norm(d1.s - d2.s);
                if (distance < radius) radius = distance;
            }
            circle.r = radius;
        }

        Scalar sum_of_weights;
        PVector weighted_sum();
        Scalar q_radius = get_cursor().r;
        for (const auto& pair : set)
        {
            const auto& demo   = pair.first;
            if (demo.name == cursor_name) continue;
            const auto& circle = pair.second;
            if ((q_radius + circle.r) < circle.d) continue; // the circles are non-intersecting
        
            Scalar w = weight(q_radius, circle.r, circle.d);
            sum_of_weights += w;
            weighted_sum += w * demo.p;
        }
        weighted_sum = (1 / sum_of_weights) * weighted_sum;
        return weighted_sum;
    }

private:
    std::unordered_map<Demo, Circle, DemonstrationHash<Demo>> set;
}