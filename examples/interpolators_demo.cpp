#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <vector>
#include <tuple>
#include <random>
#include <chrono>
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_error.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <Eigen/Core>
#include <Eigen/LU>
#include "../interpolator/marier_spheres.h"

using Scalar = float;
using ID = unsigned int;
using Vec2 = Eigen::Vector2f;
using RGBVec = Eigen::Vector3f;
using CIEXYZVec = Eigen::Vector3f;
using JzAzBzVec = Eigen::Vector3f;
using Interp = Interpolators<Scalar, ID, Vec2, JzAzBzVec>;

#define ANOTHER_INTERPOLATOR(type, ...) std::make_tuple(type{}, std::vector<type::Meta>{}, std::vector<type::Para>{}, type::Para{__VA_ARGS__})
auto interpolators = std::make_tuple
        ( ANOTHER_INTERPOLATOR(Interp::IntersectingNSpheres)
        , ANOTHER_INTERPOLATOR(Interp::InverseDistance, 4, 0.001, 0.0)
        );

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

struct Context
{
    std::vector<Interp::Demo> demo;
    std::size_t N = 3; // number of demonstrations
    std::size_t active_interpolator = 0;
    const std::size_t num_interpolators = std::tuple_size_v<decltype(interpolators)>;
    unsigned int C = 0;
    unsigned int w = 500;
    unsigned int h = 500;
    bool dynamic_demos = false;
    bool redraw = true;
    bool quit = false;
    Scalar grab_dist = 20;
    Interp::Demo * grabbed = nullptr;
    std::size_t grabbed_idx = 0;
    Vec2 mouse = {0, 0};
    SDL_Window * window;
    SDL_Surface * surface;
    SDL_Renderer * renderer;
} context;

template<typename T>
void draw(unsigned int& i, T& tup)
{
    if (i != context.active_interpolator) 
    {
        ++i; 
        return;
    }

    auto& interpolator = std::get<0>(tup);
    auto& meta = std::get<1>(tup);
    auto& para = std::get<2>(tup);

    bool ran = false;
    auto start = std::chrono::high_resolution_clock::now();
    for (unsigned int xpix = 0; xpix < context.w; ++xpix)
    {
        for (unsigned int ypix = 0; ypix < context.h; ++ypix)
        {
            auto q = Vec2{xpix/(Scalar)context.w, ypix/(Scalar)context.h};
            RGBVec out = {0, 0, 0};
            bool skip = false;
            for (auto& d : context.demo)
            {
                auto dist = (q - d.s).norm(); 
                if (dist < 6/(Scalar)context.w)
                {
                    if (dist < 3/(Scalar)context.w)
                        out = JzAzBz_to_RGB(d.p);
                    skip = true;
                }
            }
            if (not skip)
            {
                JzAzBzVec interpolated_jab{0, 0, 0};
                if constexpr (std::is_same_v<decltype(interpolator), Interp::IntersectingNSpheres>)
                {
                    if (not context.dynamic_demos && not ran)
                    {
                        interpolator.dynamic_demos = true;
                        interpolator.query(q, context.demo, para, meta, interpolated_jab);
                        interpolator.dynamic_demos = false;
                        ran = true;
                    }
                    else interpolator.query(q, context.demo, para, meta, interpolated_jab);
                }
                else interpolator.query(q, context.demo, para, meta, interpolated_jab);

                if (context.C) 
                {
                    for (unsigned int n = 0; n < context.N; ++n)
                    {
                        RGBVec rgb;
                        Scalar w;
                        if (context.grabbed) 
                        {
                            rgb = JzAzBz_to_RGB(context.grabbed->p);
                            w = meta[context.grabbed_idx].w;
                        }
                        else 
                        {
                            rgb = JzAzBz_to_RGB(context.demo[n].p); 
                            w = meta[n].w;
                        }
                        if (w >= 1.0 - std::numeric_limits<Scalar>::min() * 5)
                        {
                            // visualize maximum elevation with inverted colour dots
                            out = (xpix % 3) + (ypix % 3) == 0 ? RGBVec{1,1,1} - rgb : rgb;
                        }
                        else
                        {
                            Scalar brightness = std::pow(std::fmod(w * context.C, 1.0f), 8);
                            brightness = brightness * w;
                            out += rgb * brightness;
                        }
                        if (context.grabbed) break;
                    }
                    //contour_map.set_pixel(xpix, ypix,
                    //        (unsigned char)std::round(std::min(out.x(), 1.0f) * 255),
                    //        (unsigned char)std::round(std::min(out.y(), 1.0f) * 255),
                    //        (unsigned char)std::round(std::min(out.z(), 1.0f) * 255));
                }
                else out = JzAzBz_to_RGB(interpolated_jab);
            }
            SDL_SetRenderDrawColor
                    ( context.renderer
                    , (unsigned char)std::round(std::min(out.x(), 1.0f) * 255)
                    , (unsigned char)std::round(std::min(out.y(), 1.0f) * 255)
                    , (unsigned char)std::round(std::min(out.z(), 1.0f) * 255)
                    , 255
                    );
            SDL_RenderDrawPoint(context.renderer, xpix, ypix);
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    std::cout << i-1 << ": Generated " << context.w * context.h << " interpolations in " << usec << " microseconds\n" 
            << "About " << 1000000 * context.w * context.h / usec << " interpolations per second" 
            << std::endl;
    ++i;
}

void loop ()
{
    static SDL_Event ev;
    while (SDL_PollEvent(&ev)) switch (ev.type)
    {
    case SDL_QUIT:
    case SDL_APP_TERMINATING:
    case SDL_APP_LOWMEMORY:
        context.quit = true;
        break;

    case SDL_WINDOWEVENT:
        // TODO
        break;

    case SDL_KEYDOWN:
        context.C = 10; 
        context.redraw = true;
        break;

    case SDL_KEYUP:
        context.C = 0; 
        context.redraw = true;
        break;

    case SDL_MOUSEMOTION:
        context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
        if (context.grabbed)
        {
            context.grabbed->s = context.mouse;
#           ifndef __EMSCRIPTEN__
            context.redraw = true;
#           endif
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        context.mouse = {ev.button.x / (Scalar)context.w, ev.button.y / (Scalar)context.h};
        {
            Scalar dist, min_dist;
            min_dist = std::numeric_limits<Scalar>::max();
            for (unsigned int n = 0; n < context.N; ++n)
            {
                auto& d = context.demo[n];
                dist = (context.mouse - d.s).norm();
                if (dist < min_dist) 
                {
                    context.grabbed = &d;
                    context.grabbed_idx = n;
                    min_dist = dist;
                }
            }
            if (min_dist > context.grab_dist / (Scalar)context.w) context.grabbed = nullptr;
        }
        if (context.C) context.redraw = true;
        break;

    case SDL_MOUSEBUTTONUP:
        context.grabbed = nullptr;
        context.redraw = true;
        break;

    case SDL_MOUSEWHEEL:
        context.active_interpolator = (context.active_interpolator + 1) % context.num_interpolators;
        context.redraw = true;
        break;

    default:
        break;
    }

    if (context.redraw)
    {
        SDL_RenderClear(context.renderer);
        unsigned int i = 0;
        std::apply([&](auto& ... tuples) {((draw(i, tuples)), ...);}, interpolators);
        context.redraw = false;
        SDL_RenderPresent(context.renderer);
    }

    //SDL_UpdateWindowSurface(context.window);
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error initializing SDL:\n    %s\n", 
                SDL_GetError());
        return 1;
    }
    else SDL_Log("Initialized successfully\n");

    context.window = SDL_CreateWindow
            ( "Interpolators"
            , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
            , context.w , context.h
            , SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
            );
    if (context.window == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating window:\n    %s\n", 
                SDL_GetError());
        return 1;
    }
    else SDL_Log("Created window\n");

    context.renderer = SDL_CreateRenderer(context.window, -1, SDL_RENDERER_ACCELERATED);
    if (context.renderer == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "Error creating renderer:\n    %s\n", 
                SDL_GetError());
        return 1;
    }
    else SDL_Log("Created renderer\n");

    context.surface = SDL_GetWindowSurface(context.window);

    unsigned int n = context.N;
    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    std::uniform_real_distribution<Scalar> random(0, 1);
    while(n-- > 0)
    {
        auto v = Vec2{random(generator), random(generator)};
        auto c = RGB_to_JzAzBz(RGBVec{random(generator), random(generator), random(generator)});
        context.demo.push_back({n, v, c});
    }

    auto resize_lists = [&](auto& tup)
    {
        auto& meta = std::get<1>(tup);
        auto& para = std::get<2>(tup);
        auto& default_para = std::get<3>(tup);
        meta.resize(context.demo.size());
        for (auto& m : meta) m = {};
        para.resize(context.demo.size());
        for (auto& p : para) p = default_para;
    };
    std::apply([&](auto& ... tuples) {((resize_lists(tuples)), ...);}, interpolators);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    while (not context.quit)
    {
        loop();
        SDL_Delay(33);
    }
#endif

    SDL_FreeSurface(context.surface);
    SDL_DestroyRenderer(context.renderer);
    SDL_DestroyWindow(context.window);
    SDL_Quit();
    return 0;
}
