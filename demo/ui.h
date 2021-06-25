#ifndef UI_H
#define UI_H

#include <cstddef>
#include "types.h"

RGBVec XYZ_to_RGB(const CIEXYZVec& xyz)
{
    RGBVec rgb;
    Eigen::Matrix3f a;
    a <<  3.24096994, -1.53738318, -0.49861076,
         -0.96924364,  1.8759675,   0.04155506,
          0.05563008, -0.20397696,  1.05697151;

    rgb = a * xyz; // apply linear transformation

    for (unsigned int i = 0; i < 3; i++) // apply gamme correction
    {
        auto u = rgb[i];
        u = u > 0.0031308 ? (1.055 * pow(u, 1 / 2.4) - 0.055) : 12.92 * u;
        u = u < 0.0 ? 0.0 : u;
        u = u > 1.0 ? 1.0 : u;
        rgb[i] = u;
    }

    return rgb;
}

CIEXYZVec RGB_to_XYZ(const RGBVec& rgb)
{
    CIEXYZVec xyz = rgb;
    Eigen::Matrix3f a;
    a <<  0.41239080,  0.35758434,  0.18048079,
          0.21263901,  0.71516868,  0.07219232,
          0.01933082,  0.11919478,  0.95053215;

    for (unsigned int i = 0; i < 3; i++) // reverse gamme correction
    {
        auto u = xyz[i];
        u = u > 0.04045 ? pow((u + 0.055) / 1.055, 2.4) : u / 12.92;
        xyz[i] = u;
    }

    return a * xyz; // reverse linear transformation
}

JzAzBzVec XYZ_to_JzAzBz(const CIEXYZVec& xyz)
{
    static const Eigen::Matrix3f M1 = (Eigen::Matrix3f() <<
        0.41478972,  0.579999,  0.0146480,
        -0.2015100,  1.120649,  0.0531008,
        -0.0166008,  0.264800,  0.6684799).finished();
    static const Eigen::Matrix3f M2 = (Eigen::Matrix3f() <<  
        0.5,         0.5,       0.0,
        3.524000,   -4.066708,  0.542708,
        0.199076,    1.096799, -1.295875).finished();
    constexpr float two5 = 1 << 5;
    constexpr float two7 = 1 << 7;
    constexpr float two12 = 1 << 12;
    constexpr float two14 = 1 << 14;
    constexpr float b = 1.15;
    constexpr float g = 0.66;
    constexpr float c1 = 3424.0 / two12;
    constexpr float c2 = 2413.0 / two7;
    constexpr float c3 = 2392.0 / two7;
    constexpr float n = 2610.0 / two14;
    constexpr float p = 1.7 * 2523.0 / two5;
    constexpr float d = -0.56;
    constexpr float d_0 = 1.6295499532821566e-11;
    auto x = xyz.x();
    auto y = xyz.y();
    auto z = xyz.z();

    // pre-adjust to improve iso-hue linearity (Safdar et al. eqn. 8
    auto x_ = b * x - (b - 1.0) * z;
    auto y_ = g * y - (g - 1.0) * x;
    CIEXYZVec xyz_(x_, y_, z);

    // transform xyz to cone primaries (Safdar et al. eqn. 9)
    Eigen::Vector3f lms = M1 * xyz_;

    // perceptual quantizer (Safdar et al. eqn. 10
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i] / 10000.0, n);
        lms[i] = pow( (c1 + c2 * u) / (1.0 + c3 * u), p );
    }

    // transform to correlates of opponent colour space (Safdar et al. eqn. 11)
    Eigen::Vector3f jab = M2 * lms;

    // improve wide-range lightness prediction (Safdar et al. eqn. 12)
    auto i = jab[0];
    jab[0] = ((1.0 + d) * i) / (1.0 + d * i) - d_0;

    return jab;
}

CIEXYZVec JzAzBz_to_XYZ(const JzAzBzVec& jab)
{
    static const Eigen::Matrix3f M1 = (Eigen::Matrix3f() <<
        0.41478972,  0.579999,  0.0146480,
        -0.2015100,  1.120649,  0.0531008,
        -0.0166008,  0.264800,  0.6684799).finished();
    static const Eigen::Matrix3f M2 = (Eigen::Matrix3f() <<  
        0.5,         0.5,       0.0,
        3.524000,   -4.066708,  0.542708,
        0.199076,    1.096799, -1.295875).finished();
    constexpr float two5 = 1 << 5;
    constexpr float two7 = 1 << 7;
    constexpr float two12 = 1 << 12;
    constexpr float two14 = 1 << 14;
    constexpr float b = 1.15;
    constexpr float g = 0.66;
    constexpr float c1 = 3424.0 / two12;
    constexpr float c2 = 2413.0 / two7;
    constexpr float c3 = 2392.0 / two7;
    constexpr float n = 2610.0 / two14;
    constexpr float p = 1.7 * 2523.0 / two5;
    constexpr float d = -0.56;
    constexpr float d_0 = 1.6295499532821566e-11;
    static const Eigen::Matrix3f M1inv = M1.inverse();
    static const Eigen::Matrix3f M2inv = M2.inverse();

    // eqn. 17
    auto iab = jab;
    auto j = jab[0] + d_0;
    iab[0] = j / (1.0 + d - d * j);

    // eqn. 18
    Eigen::Vector3f lms = M2inv * iab;

    // eqn. 19
    for (unsigned int i = 0; i < 3; ++i)
    {
        auto u = pow(lms[i], 1.0/p);
        lms[i] = 10000.0 * pow( (c1 - u) / (c3 * u - c2), 1.0/n );
    }

    // eqn. 20
    auto xyz_ = M1inv * lms;

    // eqn. 21 - 23
    auto x_ = xyz_[0];
    auto y_ = xyz_[1];
    auto z_ = xyz_[2];
    auto x = (x_ + (b - 1.0) * z_) / b;
    auto y = (y_ + (g - 1.0) * x) / g;
    auto z = z_;

    CIEXYZVec xyz{x, y, z};
    return xyz;
}

JzAzBzVec RGB_to_JzAzBz(const RGBVec& rgb)
{
    return XYZ_to_JzAzBz(RGB_to_XYZ(rgb));
}

RGBVec JzAzBz_to_RGB(const JzAzBzVec& jab)
{
    return XYZ_to_RGB(JzAzBz_to_XYZ(jab));
}

class UserInterface
{
public:
    bool ready_to_quit() const {return quit;}
    bool needs_to_redraw() const {return redraw;}
    std::size_t active_interpolator() const {return _active_interpolator;}
    const Texture& texture() const {return _texture;}

    template<typename Tuple> void draw(const std::size_t i, Tuple& tup, const DemoList& demo) const
    {
        auto& interpolator = std::get<0>(tup);
        auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        
        auto start = std::chrono::high_resolution_clock::now();

        for (unsigned int col = 0; col < _texture.cols(); ++col)
        {
            for (unsigned int row = 0; row < _texture.rows(); ++row)
            {
                RGBVec out = {0, 0, 0};
                auto q = Vec2{col/(Scalar)_texture.cols(), row/(Scalar)_texture.rows()};
                JzAzBzVec interpolated_jab{0, 0, 0};

                interpolator.query(q, demo, para, meta, interpolated_jab);

                if (contour_lines) 
                {
                    for (unsigned int n = 0; n < demo.size(); ++n)
                    {
                        RGBVec rgb;
                        Scalar w;
                        if (grabbed) 
                        {
                            rgb = JzAzBz_to_RGB(grabbed->p);
                            w = meta[grabbed_idx].w;
                        }
                        else 
                        {
                            rgb = JzAzBz_to_RGB(demo[n].p); 
                            w = meta[n].w;
                        }
                        if (w >= 1.0 - std::numeric_limits<Scalar>::min() * 5)
                        {
                            // visualize maximum elevation with inverted colour dots
                            out = (col % 3) + (row % 3) == 0 ? RGBVec{1,1,1} - rgb : rgb;
                        }
                        else
                        {
                            Scalar brightness = std::pow(std::fmod(w * contour_lines, 1.0f), 8);
                            brightness = brightness * w;
                            out += rgb * brightness;
                        }
                        if (grabbed) break;
                    }
                }
                else out = JzAzBz_to_RGB(interpolated_jab);

                _texture(row, col) = RGBAVec{out.x(), out.y(), out.z(), 1};
            }
        }

        auto stop = std::chrono::high_resolution_clock::now();
        auto usec = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
        std::cout << i << ": Generated " << _texture.cols() * _texture.rows() << " interpolations in " << usec << " microseconds\n" 
                << "About " << 1000000 * _texture.cols() * _texture.rows() / usec << " interpolations per second" 
                << std::endl;

        redraw = false;
    }

    void poll_event_queue(DemoList& demo)
    {
        static SDL_Event ev;
        while (SDL_PollEvent(&ev)) switch (ev.type)
        {
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
        case SDL_APP_LOWMEMORY:
            quit = true;
            break;

            case SDL_WINDOWEVENT:
                // TODO
                break;

            case SDL_KEYDOWN:
                contour_lines = 10; 
                redraw = true;
                break;

            case SDL_KEYUP:
                contour_lines = 0; 
                redraw = true;
                break;

            case SDL_MOUSEMOTION:
                mouse = {ev.button.x / (Scalar)w, ev.button.y / (Scalar)h};
                if (grabbed)
                {
                    grabbed->s = mouse;
        #           ifndef __EMSCRIPTEN__
                    redraw = true;
        #           endif
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                mouse = {ev.button.x / (Scalar)w, ev.button.y / (Scalar)h};
                {
                    Scalar dist, min_dist;
                    min_dist = std::numeric_limits<Scalar>::max();
                    for (unsigned int n = 0; n < demo.size(); ++n)
                    {
                        auto& d = demo[n];
                        dist = (mouse - d.s).norm();
                        if (dist < min_dist) 
                        {
                            grabbed = &d;
                            grabbed_idx = n;
                            min_dist = dist;
                        }
                    }
                    if (min_dist > grab_dist / (Scalar)w) grabbed = nullptr;
                }
                if (contour_lines) redraw = true;
                break;

            case SDL_MOUSEBUTTONUP:
                grabbed = nullptr;
                redraw = true;
                break;

            case SDL_MOUSEWHEEL:
                _active_interpolator = (_active_interpolator + 1) % num_interpolators;
                redraw = true;
                break;

        default:
            break;
        }
    }

    mutable Texture _texture = Texture(500, 500);
private:
    mutable bool redraw = true;

    bool quit = false;
    unsigned int contour_lines = 0;
    Vec2 mouse = {0, 0};
    unsigned int w = 500;
    unsigned int h = 500;
    Interpolator::Demo * grabbed = nullptr;
    std::size_t grabbed_idx = 0;
    Scalar grab_dist = 20.0/500.0;
    std::size_t _active_interpolator = 0;
};
#endif
