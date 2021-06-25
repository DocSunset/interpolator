# Setup

```cpp
// @='setup'
@{SDL setup}

@{initialize random demonstrations}

@{resize interpolators extra lists}

@{create shader programs}

@{create vector buffer objects}

@{create main texture}

atexit(cleanup);
// @/

// @+'globally visible state'
SDL_Window * window = nullptr;
SDL_GLContext gl = nullptr;
// @/
```

# Interpolators Setup

```cpp
// @='initialize random demonstrations'
unsigned int n = 3;
unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator (seed);
std::uniform_real_distribution<Scalar> random(0, 1);
while(n-- > 0)
{
    auto v = Vec2{random(generator), random(generator)};
    auto c = RGB_to_JzAzBz(RGBVec{random(generator), random(generator), random(generator)});
    demo.push_back({n, v, c});
}
// @/

// @='resize interpolators extra lists'
auto resize_lists = [&](auto& tup)
{
    auto& meta = std::get<1>(tup);
    auto& para = std::get<2>(tup);
    auto& default_para = std::get<3>(tup);
    meta.resize(demo.size());
    for (auto& m : meta) m = {};
    para.resize(demo.size());
    for (auto& p : para) p = default_para;
};
std::apply([&](auto& ... tuples) {((resize_lists(tuples)), ...);}, interpolators);
// @/
```


# SDL

```cpp
// @='SDL setup'
if (SDL_Init(SDL_INIT_VIDEO) != 0)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Error initializing SDL:\n    %s\n", 
            SDL_GetError());
    return EXIT_FAILURE;
}
else SDL_Log("Initialized successfully\n");

SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
context.window = SDL_CreateWindow
        ( "Interpolators"
        , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
        , context.w , context.h
        , SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN
        );
if (context.window == nullptr)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Error creating window:\n    %s\n", 
            SDL_GetError());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            "Couldn't create the main window :(", NULL);
    return EXIT_FAILURE;
}
else SDL_Log("Created window\n");

context.gl = SDL_GL_CreateContext(context.window);
if (context.gl == nullptr)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "Error creating OpenGL context:\n    %s\n", 
            SDL_GetError());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            "Couldn't create OpenGL context :(", NULL);
    return EXIT_FAILURE;
}
else SDL_Log("Created GL context\n");
// @/

// @+'helper functions'
void cleanup ()
{
    glDeleteTextures(1, &context.texture_gl);
    glDeleteBuffers(1, &context.screen_quad_vbo);
    glDeleteProgram(context.prog);
    SDL_GL_DeleteContext(context.gl);
    SDL_DestroyWindow(context.window);
    SDL_Quit();
}
// @/
```

