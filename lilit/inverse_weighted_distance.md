# Inverse Distance Interpolator

Another simple interpolation algorithm is presented here. This algorithm is one
of the earliest proposed in the literature, with roots in the early 1980s with
the SYTER system developed at GRM. The implementation below follows the presentation
by Todoroff and colleagues in their 2009 paper at the International Computer
Music Conference, "1-d, 2-d and 3-d interpolation tools for max/msp/jitter."
Despite the name of their paper, the algorithm naturally extends to arbitrary
source and destination spatial dimensions.

```cpp
// @+'interpolators'
struct InverseDistance /* after e.g. Todoroff 2009 ICMC */
{
    struct Meta { Scalar d = 0, w = 0; };
    struct Para 
    { 
        Scalar power = 8
        ,      d_min = std::numeric_limits<Scalar>::min()
        ,      r_min = 0
        ,      r = 1
        ;
    };

    template<typename DemoList, typename MetaList, typename ParaList>
    PVector query(const SVector& q, const DemoList& demo, const ParaList& para,
            MetaList& meta, PVector& weighted_sum)
    {
        Scalar sum_of_weights = 0;
        std::size_t i, N = demo.size();
        if (N < 1) return weighted_sum;
        if (N != meta.size()) return weighted_sum;

        for (i=0; i<N; ++i)  { meta[i].d = (demo[i].s - q).norm(); }
        for (i=0; i<N; ++i)  
        { 
            auto base = std::max(meta[i].d - para[i].r_min, para[i].d_min);
            meta[i].w = para[i].r / pow( base, para[i].power); 
        }
        for (i=0; i<N; ++i)  { weighted_sum = weighted_sum + meta[i].w * demo[i].p; }
        for (Meta& m : meta) { sum_of_weights = sum_of_weights + m.w; }
        for (Meta& m : meta) { m.w = m.w / sum_of_weights; }

        return weighted_sum = (1 / sum_of_weights) * weighted_sum;
    }
};
// @/
```
