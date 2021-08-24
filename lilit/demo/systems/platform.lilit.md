# Platform System

The platform system handles direct interactions with the platform library; in
this case, SDL. Its constructor is also in charge of setting up the SDL
subsystems (such as OpenGL) that other systems (such as graphical systems) may
rely on. For this reason, it's important that the platform system is the first
to be constructed by the main application.

The platform system uses the `pimpl` idiom to insulate the main application from
SDL, which is considered as an implementation detail. `platform.h` simply declares
the methods of the system:

```cpp
// @#'demo/interpolators/systems/platform.h'
#pragma once

#include <entt/entt.hpp>
#include "system.h"

namespace System
{

    class PlatformImplementation;
    
    class Platform : public System
    {
        PlatformImplementation* pimpl;
    public:
        Platform(entt::registry&);
        ~Platform();
        void run(entt::registry&) override;
    };

}
// @/
```

In the implementation file, the pimpl is constructed and deleted, and its
run method is called. These have to be defined outside the header, since the
definition of `PlatformImplementation` must be known to the compiler in order
for it to call the pimpl's methods, and the whole point of the pimpl idiom is
to keep this information out of the header file.

```cpp
// @#'demo/interpolators/systems/platform.cpp'
#include "platform.h"

#include <SDL.h>
#include "components/quit_flag.h"

namespace System
{

    @{PlatformImplementation definition}
    
    Platform::Platform(entt::registry& registry)
    {
        pimpl = new PlatformImplementation(registry);
    }
    
    Platform::~Platform()
    {
        delete pimpl;
    }

    void Platform::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }

}

// @/
```

The actual implementation follows:

```cpp
// @='PlatformImplementation definition'
class PlatformImplementation
{
    SDL_Window * window;
    SDL_GLContext gl;

public:
    // standard SDL setup boilerplate
    PlatformImplementation(entt::registry& registry)
    {
        // no audio yet...
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "Error initializing SDL:\n    %s\n", 
                    SDL_GetError());
            exit(EXIT_FAILURE);
        }
        else SDL_Log("Initialized successfully\n");
        
        // ask to use opengl es3 (for web/emscripten compatibility)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // make the window
        window = SDL_CreateWindow
                ( "Interpolators"
                , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
                , 500 , 500
                , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                );

        if (window == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "Error creating window:\n    %s\n", 
                    SDL_GetError());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                    "Couldn't create the main window :(", NULL);
            exit(EXIT_FAILURE);
        }
        else SDL_Log("Created window\n");
        
        // set up opengl context
        gl = SDL_GL_CreateContext(window);
        if (gl == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                    "Error creating OpenGL context:\n    %s\n", 
                    SDL_GetError());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                    "Couldn't create OpenGL context :(", NULL);
            exit(EXIT_FAILURE);
        }
        else SDL_Log("Created GL context\n");
    }

    // this should arguably delete the window and so on, but since the app is quitting...
    ~PlatformImplementation() { }

    void quit(entt::registry& registry)
    {
        registry.set<Component::QuitFlag>(true);
    }

    void run(entt::registry& registry)
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) switch (ev.type)
        {
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
        case SDL_APP_LOWMEMORY:
            quit(registry);
            break;
        case SDL_WINDOWEVENT:
            switch (ev.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                quit(registry);
                break;
            }
            break;
        }
    }
};
// @/
```
