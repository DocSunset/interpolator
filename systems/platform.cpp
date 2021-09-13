#include "platform.h"

#include <GLES3/gl3.h>
#include <SDL.h>
#include <SDL_video.h>
#include "components/quit_flag.h"
#include "components/window.h"
#include "gl/ll.h"

namespace System
{

    class Platform::PlatformImplementation
    {
        SDL_Window * window;
        SDL_GLContext gl;
        entt::registry::entity_type window_entity;

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
                    , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN// | SDL_WINDOW_RESIZABLE
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

            SDL_GL_SetSwapInterval(1); // should check for errors
            
            // create platform entities
            window_entity = registry.create();
            registry.emplace<Component::Window>(window_entity, 500, 500);
        }

        // this should arguably delete the window and so on, but since the app is quitting...
        ~PlatformImplementation() { }

        void quit(entt::registry& registry)
        {
            registry.set<Component::QuitFlag>(true);
        }

        void poll_events(entt::registry& registry)
        {
            auto win = registry.get<Component::Window>(window_entity);
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

        void run(entt::registry& registry)
        {
            poll_events(registry);

            SDL_GL_SwapWindow(window);
            glViewport(0, 0, 500, 500);// this should only be called when window size changes
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void * get_window() const
        {
            return window;
        }
    };
    
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

    void * Platform::window() const
    {
        return pimpl->get_window();
    }

}

